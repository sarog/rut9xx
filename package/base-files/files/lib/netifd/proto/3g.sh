#!/bin/sh
INCLUDE_ONLY=1

. /lib/teltonika-functions.sh
. ../netifd-proto.sh
. ./ppp.sh
init_proto "$@"

proto_3g_init_config() {
	no_device=1
	available=1
	ppp_generic_init_config
	proto_config_add_string "device"
	proto_config_add_string "apn"
	proto_config_add_string "service"
	proto_config_add_string "enabled"
	proto_config_add_string "dialnumber"
	proto_config_add_string "roaming"
	proto_config_add_string "backup"
	proto_config_add_string "pdptype"
}

proto_3g_setup() {
	local interface="$1"
	local chat_path="/tmp/chatscripts"
	local chat="$chat_path/3g.chat"
	local evidpid=$(get_ext_vidpid_tlt)
	local disableipv6
	local context

	if [ ! -d "$chat_path" ]; then
		mkdir "$chat_path"
	fi

	json_get_var device device
	json_get_var apn apn
	json_get_var service service
	json_get_var enabled enabled
	json_get_var dialnumber dialnumber
	json_get_var roaming roaming
	json_get_var pdptype pdptype

	dialnumber=${dialnumber:-"*99#"}

	if [ -n "$pdptype" ] && [ "$pdptype" = "1" ]; then
		disableipv6="1"
	else
		disableipv6="0"
	fi

	# if roaming - exit
	if [ "$roaming" = "1" ]; then
		local variable=`gsmctl -A "AT+CREG?"`
		local stat=${variable#*,}
		if [ "$stat" = "5" ]; then
			logger -t $0 "roaming detected"
			return 1
		fi
	fi

	[ -e "$device" ] || {
		proto_set_available "$interface" 0
		return 1
	}

	# if ppp disabled - exit
	if [ "$enabled" != "1" ] && [ ! -f "/tmp/mobileon" ]; then
		ifdown ppp
		return 0
	fi

	# Check GSMD
	if [ "$(pidof gsmd)" ]
	then
		echo "$config(3g): gsmd is running"
	else
		/etc/init.d/gsmd start
		sleep 3
	fi
	local netstate=`/usr/sbin/gsmctl -g`
	if [ -z "$netstate" ]; then
		Retry if no answer was received
		sleep 1
		netstate=`/usr/sbin/gsmctl -g`
	fi

	count="0"
	retry="5"

	# daznai buna like Acgatt=0 del to nesudaro pas kliendus duomenu konekcijos.
	attach=`gsmctl -A 'AT+CGATT?' | awk -F ' ' '{print $2}'`
	if [ "$attach" != "1" ]; then
		gsmctl -A 'AT+CGATT=1'
		sleep 2
	fi

	# Check if module is registered to network before trying to establish data connection
	while [ "$netstate" != "registered (home)" ] && [ "$netstate" != "registered (roaming)" ];
	do
		# When switching from "3G only" mode to "Automatic" with LE910_V2 module it sometimes stays in searching for operator mode.
		# This may be because of AT#AUTOATT=0 in ppp mode.
		# We force it to try to register again.
		if [ "$evidpid" = "1BC7:0036" ]; then
			local opernum=$(uci -q get network.ppp.numeric)
			if [ -n "$opernum" ]; then
			    /usr/sbin/gsmctl -A "AT+COPS=1,2,\"$numeric\""
			    sleep 15
			else
				/usr/sbin/gsmctl -A AT+COPS=0
			    sleep 15
			fi
		fi
		sleep 2

		netstate=`/usr/sbin/gsmctl -g`
		if [ "$netstate" == "denied" ] && [ "$count" == "$retry" ]; then
			/usr/sbin/gsmctl -A AT+CGDCONT=1
			/usr/sbin/gsmctl -A AT+CGDCONT=3
			/usr/sbin/gsmctl -A AT+CGDCONT=4
			debug "Netstate denied"
			/etc/init.d/modem restart
			count="0"
		fi
		count=$((count+1))
	done
	case "$service" in
		cdma |\
		evdo)
			chat="$chat_path/evdo.chat"
			;;

		*)
	esac

	#select context and set its parameters
	if [ "$evidpid" = "1BC7:0036" ] || [ "$evidpid" = "05C6:9215" ]; then
		if [ "$apn" ]; then
			context=3
			if [ "$disableipv6" = "1" ]; then
				gsmctl -A "AT+CGDCONT=1,\"IP\""
				gsmctl -A "AT+CGDCONT=$context,\"IP\",\"$apn\""
			else
				gsmctl -A "AT+CGDCONT=1,\"IPV4V6\""
				gsmctl -A "AT+CGDCONT=$context,\"IPV4V6\",\"$apn\""
			fi
		else
			apn=""
			context=1
		fi

	else
		#to clear defaul context if modem is not Telit LE910_V2 for "use only IPV4"
		if [ "$disableipv6" = "1" ]; then
			gsmctl -A "AT+CGDCONT=1,\"IP\""
		else
			gsmctl -A "AT+CGDCONT=1,\"IPV4V6\""
		fi
	fi

	# making chat script
	case "$service" in
		*)
			# default to UMTS
			rm -f $chat > /dev/null 2>&1
			printf "ABORT   BUSY\n" >> $chat
			printf "ABORT   'NO CARRIER'\n" >> $chat
			printf "ABORT   ERROR\n" >> $chat
			printf "REPORT  CONNECT\n" >> $chat
			printf "TIMEOUT 10\n" >> $chat
			printf "\"\"      ATZ\n" >> $chat
			printf "\"\"      \"AT&F\"\n" >> $chat

			# Telit LE910_V2 single and double qoutemarks in AT commands
			if [ "$evidpid" = "1BC7:0036" ]; then
				printf "OK      \"AT#SETHEXSTR=2\"\n" >> $chat
			fi

			# Huawei LTE modem has echo enabled by default, so we disable it because it messes up output of GSMD.
			if [ "$evidpid" = "12D1:1573" ]; then
				printf "OK      \"ATVE0\"\n" >> $chat
			else
				printf "OK      \"ATE1\"\n" >> $chat
			fi
			if [ "$evidpid" = "1BC7:1201" ]; then
				printf "OK      'AT+CREG=2'\n" >> $chat
			fi

			if [ "$evidpid" != "1BC7:0036" ]; then
				if [ "$disableipv6" = "1" ]; then
					printf "OK      'AT+CGDCONT=1,\"IP\",\"\$USE_APN\"'\n" >> $chat
				else
					printf "OK      'AT+CGDCONT=1,\"IPV4V6\",\"\$USE_APN\"'\n" >> $chat
				fi
			fi
			printf "SAY     \"Calling UMTS/GPRS\"\n" >> $chat
			printf "TIMEOUT 30\n" >> $chat

			#Fix for Quectel modems: disconnects previous data connection if connection failed
			if [ "$evidpid" = "2C7C:0125" ]; then
                printf "OK      'AT+QPPPDROP'\n" >> $chat
			fi

			#********TELIT LE910_V2 ONLY********
			#AT+CGDATA command causes the MT to perform whatever
			#actions are necessary to establish communication between
			#the TE and the network using one or more Packet Domain PDP types
			#This may include performing a PS attach and one
			#or more PDP context activations
			if [ "$evidpid" == "1BC7:0036" ]; then
				band=`gsmctl -nA at#scfg? | grep 1,1 | awk -F '1,1,' '{print $2}'`

				printf "OK      \"AT#SCFG=6,$context,$band\"\n" >> $chat

				if [ "$apn" == "" ]; then
					printf "OK      \"ATD$dialnumber\"\n" >> $chat
				else
					printf "OK      \"ATD*99***$context#\"\n" >> $chat
				fi
			#********TELIT LE910_V2 ONLY********
			else
				printf "OK      \"ATD$dialnumber\"\n" >> $chat
			fi
			printf "CONNECT ' '\n" >> $chat
			sync
			;;
	esac

	local chat_args=""
	local pppd_aux_args=""
	local enable_chat_log="`uci get system.system.enable_chat_log`"
	local enable_pppd_debug="`uci get system.system.enable_pppd_debug`"


	if [ "$enable_chat_log" = "0" ]
	then
		logger "3g.sh: \"chat\" logging disabled by uci"
		chat_args="-t5 -S -E -f $chat"
	else
		chat_args="-t5 -v -E -f $chat"
	fi

	if [ "$enable_pppd_debug" = "1" ]
	then
		logger "3g.sh: setting \"pppd\" to debug mode"
		pppd_aux_args="debug"
	fi

	#Clear uncolicited messages from device
	microcom -t 100 "$device" >/dev/null 2>&1

	connect="${apn:+USE_APN=$apn }/usr/sbin/chat  $chat_args"
	ppp_generic_setup "$interface" \
		$pppd_aux_args \
		noaccomp \
		nopcomp \
		novj \
		nobsdcomp \
		noauth \
		lock \
		crtscts \
		115200 "$device"
	return 0
}

proto_3g_teardown() {
	proto_kill_command "$interface"

}

add_protocol 3g
