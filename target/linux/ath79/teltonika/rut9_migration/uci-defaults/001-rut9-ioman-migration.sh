#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

mv /etc/config/ioman /etc/config/ioman_legacy
cp /rom/etc/config/ioman /etc/config/

config_load ioman_legacy

################################## inversions ##################################
# ins
config_get active_high ioman active_DIN1_status 1
uci_set ioman din2 invert_input $((!active_high))

config_get active_high ioman active_DIN2_status 1
uci_set ioman iio invert_input $((!active_high))

config_get active_high ioman active_DIN3_status 1
uci_set ioman din1 invert_input $((active_high))

# outs
config_get dout2_active_high ioman active_DOUT1_status 1
config_get relay0_active_high ioman active_DOUT2_status 1
config_get dout1_active_high ioman active_DOUT3_status 1

########################## restore IO states on bootup #########################
config_get dout2_new ioman default_DOUT1_status 1
[ $dout2_active_high -eq 0 ] && dout2_new=$((!dout2_new))
uci_set ioman dout2 value "$dout2_new"

config_get relay0_new ioman default_DOUT2_status 1
[ $relay0_active_high -eq 0 ] && relay0_new=$((!relay0_new))
[ "$relay0_new" = "1" ] && relay0_new="closed" || relay0_new="open"
uci_set ioman relay0 state "$relay0_new"

config_get dout1_new ioman default_DOUT3_status 1
[ $dout1_active_high -eq 0 ] && dout1_new=$((!dout1_new))
uci_set ioman dout1 value "$dout1_new"

################### migrate custom analog converter/formula ####################

analogs=$(uci show ioman | grep -oE "a(dc|cl)[0-9]" | uniq)

for an in $analogs; do                                                                    
	config_get opt iolabels analoginput && uci set ioman.${an}.custom_name="$opt"     
	config_get opt iolabels anformeasunit && uci set ioman.${an}.custom_unit="$opt"   
	config_get opt iolabels anforadd "0" && uci set ioman.${an}.custom_add="$opt"     
	config_get opt iolabels anformultiply "1" && uci set ioman.${an}.custom_mul="$opt"
	config_get opt iolabels anfordivide "1" && uci set ioman.${an}.custom_div="$opt"  
	config_get opt iolabels anforoffset "0" && uci set ioman.${an}.custom_off="$opt"  
done 

uci_commit ioman

######################################## IOJ ###################################
enable_ioj=0
action_id=1

ioj_group_name_counter=1
new_ioj_group_name() {
	ioj_group_name="IOJuggler_$ioj_group_name_counter"
	ioj_group_name_counter=$((ioj_group_name_counter+1))
}
new_ioj_group_name

rule_to_ioj_cb() {
	revert_changes() {
		uci revert ioman
		uci revert iojuggler
		uci revert user_groups
	}

	local enabled need_2_input_action_pairs=0
	config_get enabled $section enabled 0

	replace_params() {
		text="${text//%di/%g2}"
		text="${text//%ii/%g5}"
		text="${text//%ai/%g0}"
		text="${text//%pi/%g1}"
		text="${text//%po/%g3}"
		text="${text//%et/Input}"
		text="${text//%nl/$'\n'}"

		local an ax
		config_get an $section minc -
		config_get ax $section maxc -
		[ "$an" = "-" ] && {
			config_get an $section min -
			config_get ax $section max -
			[ "$an" = "-" ] && {
				an="Unknown"
				ax="Unknown"
			}
		}

		text="${text//%an/$an}"
		text="${text//%ax/$ax}"
	}

	######## migrate legacy action #########
	local action ui_name
	config_get action $section action
	case $action in
		sendSMS)
			ui_name="Send SMS #$action_id"

			touch /etc/config/user_groups
			uci batch <<-EOF
				add iojuggler action
				set iojuggler.@action[-1].type=sms
				set iojuggler.@action[-1].id=$action_id
				set iojuggler.@action[-1].ui_name="$ui_name"
				set iojuggler.@action[-1].ui_recipient_format=group
				add user_groups phone
			EOF

			copy_phones() {
				uci_add_list user_groups @phone[-1] tel "$1"
			}

			local recipient_format
			config_get recipient_format $section recipient_format
			case $recipient_format in
				single)
					uci_set iojuggler @action[-1] phone_group "$ioj_group_name"
					uci_set user_groups @phone[-1] name "$ioj_group_name"
					new_ioj_group_name
					config_list_foreach $section telnum copy_phones
				;;
				group)
					local group_name
					config_get group_name $section group
					uci_set iojuggler @action[-1] phone_group "$group_name"
					uci_set user_groups @phone[-1] name "$group_name"
					(
						config_load sms_utils

						nzn() {
							local name
							config_get name $section name
							[ "$name" != "$group_name" ] && return

							config_list_foreach $section tel copy_phones
						}
						config_foreach nzn group
					)
				;;
				*)
					revert_changes
					echo "unknown recipient format"
					return 1
				;;
			esac

			uci_commit user_groups

			local text
			config_get text $section smstxt
			replace_params
			text="$({ base64 <<-EOF
				$text
			EOF
			} | tr -d '\n')"

			uci_set iojuggler @action[-1] text "$text"

		;;
		changeSimCard)
			ui_name="Switch SIM #$action_id"
			uci batch <<-EOF
				add iojuggler action
				set iojuggler.@action[-1].type=sim_switch
				set iojuggler.@action[-1].id=$action_id
				set iojuggler.@action[-1].ui_name="$ui_name"
				set iojuggler.@action[-1].flip=1
			EOF
		;;
		sendEmail)
			ui_name="Send email #$action_id"

			local smtp_addr smtp_port smtp_secure smtp_user smtp_addr sender credentials
			config_get smtp_addr $section smtpIP
			config_get smtp_port $section smtpPort
			config_get smtp_secure $section secureConnection
			config_get smtp_user $section userName
			config_get smtp_pass $section password
			config_get sender $section senderEmail

			[ -z "$smtp_user" -a -z "$smtp_pass" ]; credentials=$?

			touch /etc/config/user_groups
			uci batch <<-EOF
				add user_groups email
				set user_groups.@email[-1].name="$ioj_group_name"
				set user_groups.@email[-1].smtp_ip="$smtp_addr"
				set user_groups.@email[-1].smtp_port=$smtp_port
				set user_groups.@email[-1].secure_conn=$smtp_secure
				set user_groups.@email[-1].credentials=$credentials
				set user_groups.@email[-1].username="$smtp_user"
				set user_groups.@email[-1].password="$smtp_pass"
				set user_groups.@email[-1].senderemail="$sender"
				commit user_groups
			EOF

			local subject text
			config_get subject $section subject
			config_get text $section message
			replace_params
			text="$({ base64 <<-EOF
				$text
			EOF
			} | tr -d '\n')"

			uci batch <<-EOF
				add iojuggler action
				set iojuggler.@action[-1].type=email
				set iojuggler.@action[-1].id=$action_id
				set iojuggler.@action[-1].ui_name="$ui_name"
				set iojuggler.@action[-1].email_group="$ioj_group_name"
				set iojuggler.@action[-1].subject="$subject"
				set iojuggler.@action[-1].text=$text
			EOF
			new_ioj_group_name

			copy_recipients() {
				uci_add_list iojuggler @action[-1] recipients "$1"
			}
			config_list_foreach $section recipEmail copy_recipients

		;;
		changeProfile)
			ui_name="Change Profile #$action_id"
			local profiles
			config_get profiles $section profiles
			profiles=$(echo "$profiles" | sed 's/_[^_]*$//g')
			uci batch <<-EOF
				add iojuggler action
				set iojuggler.@action[-1].type=profile
				set iojuggler.@action[-1].id=$action_id
				set iojuggler.@action[-1].ui_name="$ui_name"
				set iojuggler.@action[-1].profile="$profiles"
			EOF
		;;
		wifion|wifioff)
			ui_name="Toggle WiFi #$action_id"
			[ "${action:4}" = "off" ]; local on=$?

			uci batch <<-EOF
				add iojuggler action
				set iojuggler.@action[-1].type=wifi
				set iojuggler.@action[-1].id=$action_id
				set iojuggler.@action[-1].ui_name="$ui_name"
				set iojuggler.@action[-1].wifi_on=$on
			EOF
		;;
		rmson|rmsoff)
			ui_name="Toggle RMS connection #$action_id"
			[ "${action:3}" = "off" ]; local on=$?

			uci batch <<-EOF
				add iojuggler action
				set iojuggler.@action[-1].type=rms
				set iojuggler.@action[-1].id=$action_id
				set iojuggler.@action[-1].ui_name="$ui_name"
				set iojuggler.@action[-1].rms_on=$on
			EOF
		;;
		reboot)
			ui_name="Reboot #$action_id"
			local action_delay
			config_get action_delay $section reboottime 0
			uci batch <<-EOF
				add iojuggler action
				set iojuggler.@action[-1].type=reboot
				set iojuggler.@action[-1].id=$action_id
				set iojuggler.@action[-1].ui_name="$ui_name"
				set iojuggler.@action[-1].delay=$action_delay
			EOF
		;;
		output)
			ui_name="Output #"
			local continuous outputtime outputnb dest state
			config_get continuous $section continuous 0
			config_get outputnb $section outputnb

			case $outputnb in
				1)
					dest="dout2"
					state=$dout2_active_high
				;;
				2)
					dest="relay0"
					state=$relay0_active_high
				;;
				3)
					dest="dout1"
					state=$dout1_active_high
				;;
				*)
					revert_changes
					echo "unknown output"
					return 2
				;;
			esac

			dout_action() {
				uci batch <<-EOF
					add iojuggler action
					set iojuggler.@action[-1].type=dout
					set iojuggler.@action[-1].id=$1
					set iojuggler.@action[-1].ui_name="$ui_name$1"
					set iojuggler.@action[-1].dest=$dest
					set iojuggler.@action[-1].state=$2
					set iojuggler.@action[-1].maintain=0
				EOF
			}
			dout_action $action_id $state

			if [ $continuous -eq 0 ]; then
				config_get outputtime $section outputtime 0
				uci_set iojuggler @action[-1] revert $outputtime
			else # 1, 2
				dout_action $((action_id+1)) $((!state))
				need_2_input_action_pairs=1
			fi

			if [ $continuous -eq 2 ]; then
				local startdelay stopdelay
				config_get startdelay $section startdelay 0
				config_get stopdelay $section stopdelay 0

				uci_set iojuggler @action[-2] delay $startdelay
				uci_set iojuggler @action[-1] delay $stopdelay
			fi
		;;
		postGet)
			ui_name="HTTP Post/Get #$action_id"
			local httplink no_verify use_post use_https list_httpdata

			config_get httplink $section httplink
			[ -z "$httplink" ] && {
				revert_changes
				echo "no URL"
				return 3
			}
			# trim any leading whitespace
			httplink="$(sed -e 's/^[[:space:]]*//' <<-EOF
				$httplink
			EOF
			)"

			config_get use_https $section use_https 0
			[ $use_https -eq 1 ] && {
				case "$httplink" in
					https://*)
						# ok
					;;
					http://*)
						httplink="${httplink/http:\/\//https:\/\/}"
					;;
					*)
						httplink="https://$httplink"
					;;
				esac
			}

			config_get no_verify $section verify_cert verify
			[ "$no_verify" = "verify" ]; no_verify=$?

			config_get use_post $section httptype 2
			if [ "$use_post" = "2" ]; then
				use_post=0
				list_httpdata="httpdata1"
			else
				use_post=1
				list_httpdata="httpdata"
			fi

			local post_text
			copy_http_params() {
				local text="$1"
				replace_params
				if [ -z "$post_text" ]; then
					post_text="$text"
				else
					post_text="$post_text&$text"
				fi
			}
			config_list_foreach $section $list_httpdata copy_http_params
			post_text="$({ base64 <<-EOF
				$post_text
			EOF
			} | tr -d '\n')"

			uci batch <<-EOF
				add iojuggler action
				set iojuggler.@action[-1].type=http
				set iojuggler.@action[-1].id=$action_id
				set iojuggler.@action[-1].ui_name="$ui_name"
				set iojuggler.@action[-1].post=$use_post
				set iojuggler.@action[-1].no_verify=$no_verify
				set iojuggler.@action[-1].url="$httplink"
				set iojuggler.@action[-1].text="$post_text"
				set iojuggler.@action[-1].ui_params=1
			EOF

			copy_http_headers() {
				uci_add_list iojuggler @action[-1] headers "$1"
			}
			config_list_foreach $section header copy_http_headers

		;;
		*)
			revert_changes
			echo "unknown action type"
			return 4
		;;
	esac

	######### migrate a legacy input ##########
	local input_name type analog_type analog_min analog_max analog_trigger
	config_get type $section type
	case $type in
		digital1)
			input_name="din2"
		;;
		digital2)
			input_name="iio"
		;;
		digital3)
			input_name="din1"
		;;
		analog)
			config_get analog_type $section analogtype
			if [ "$analog_type" = "currenttype" ]; then
				input_name="acl0"
				uci_set ioman acl0 state active
				config_get analog_min $section minc 0
				config_get analog_max $section maxc 0
			else
				input_name="adc0"
				config_get analog_min $section min 0
				config_get analog_max $section max 0
			fi

			config_get analog_trigger $section triger
			[ "$analog_trigger" = "out" ]; analog_trigger=$?
		;;
		*)
			revert_changes
			echo "unknown type"
			return 5
		;;
	esac

	input_part_1() {
		uci batch <<-EOF
			add iojuggler input
			set iojuggler.@input[-1].name=$input_name
			set iojuggler.@input[-1].enabled=$enabled
			set iojuggler.@input[-1].wait=1
			add_list iojuggler.@input[-1].actions=$1
		EOF
	}
	input_part_1 $action_id
	[ $need_2_input_action_pairs -eq 1 ] && input_part_1 $((action_id+1))

	if [ $type = "analog" ]; then
		local wait_interval
		config_get wait_interval ioman interval 0

		input_part_2() {
			uci batch <<-EOF
				set iojuggler.@input[$1].inside=$2
				set iojuggler.@input[$1].min=$analog_min
				set iojuggler.@input[$1].max=$analog_max
				set iojuggler.@input[$1].wait=$wait_interval
			EOF

			[ "$input_name" = "acl0" ] && uci_set iojuggler @input[$1] acl current
		}

		if [ $need_2_input_action_pairs -eq 1 ]; then
			input_part_2 -2 $analog_trigger
			input_part_2 -1 $((!analog_trigger))
		else
			input_part_2 -1 $analog_trigger
		fi
	else
		local trigger trigger2
		config_get trigger $section triger
		case $trigger in
			nc)
				trigger="rising"
				trigger2="falling"
			;;
			no)
				trigger="falling"
				trigger2="rising"
			;;
			both)
				if [ $need_2_input_action_pairs -eq 1 ]; then
					revert_changes
					echo "edge=both and need_2_input_action_pairs=1: unable to migrate"
					return 6
				else
					trigger="both"
				fi
			;;
		esac

		if [ $need_2_input_action_pairs -eq 1 ]; then
			uci_set iojuggler @input[-2] trigger $trigger
			uci_set iojuggler @input[-1] trigger $trigger2
		else
			uci_set iojuggler @input[-1] trigger $trigger
		fi
	fi
	###########################################

	action_id=$((action_id + 1 + need_2_input_action_pairs))

	[ $enabled -eq 1 ] && enable_ioj=1
	uci_commit iojuggler
	uci_commit ioman
}

config_foreach rule_to_ioj_cb rule

uci_set iojuggler @general[0] enabled $enable_ioj
uci_commit iojuggler
################################################################################

rm /etc/config/ioman_legacy
