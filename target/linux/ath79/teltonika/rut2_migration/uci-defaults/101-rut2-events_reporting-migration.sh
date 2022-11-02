#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

fix_config_eventmark() {
	local var=$1
	local val=${2// /_}
	local value

	case $val in
	open_vpn)
		value="openvpn"
		;;
	multiwan)
		value="mwan"
		;;
	mobile)
		value="simcard"
		;;
	data_limit)
		value="quota_limit"
		;;
	site_blocking)
		value="hostblock"
		;;
	l2tpd)
		value="xl2tpd"
		;;
	pptp)
		value="pptpd"
		;;
	hotspot)
		value="chilli"
		;;
	content_blocker)
		value="privoxy"
		;;
	language)
		value="luci"
		;;
	access_control)
		value="uhttpd"
		;;
	rs232/rs485)
		value="rs"
		;;
	input/output)
		value="ioman"
		;;
	ssh)
		value="dropbear"
		;;
	mobile_traffic)
		value="ulogd"
		;;
	*)
		value="$val"
		;;
	esac

	eval export "$var=$value"
}

fix_reboot_eventmark() {
	local var=$1
	local val=${2// /_}
	local value

	case $val in
	From_Web_UI)
		value="web ui"
		;;
	From_input/output)
		value="input/output"
		;;
	From_ping_reboot)
		value="ping reboot"
		;;
	From_periodic_reboot)
		value="reboot scheduler"
		;;
	From_button)
		value="from button"
		;;
	From_SMS)
		value="SMS reboot from"
		;;
	From_Call)
		value="Reboot by call from"
		;;
	After_FW_upgrade)
		value="Request after FW upgrade"
		;;
	*)
		value="all"
		;;

	esac

	eval export "$var=$value"
}

fix_simswitch_eventmark() {
	local var=$1
	local val=${2// /_}
	local value

	case $val in
	SIM_2_to_SIM_1)
		value="Changing to SIM1"
		;;
	SIM_1_to_SIM_2)
		value="Changing to SIM2"
		;;

	esac

	eval export "$var=$value"
}

fix_simswitch_eventmark() {
	local var=$1
	local val=${2// /_}
	local value

	case $val in
	unplugged)
		value="changed to DOWN"
		;;
	plugged_in)
		value="changed to UP"
		;;

	esac

	eval export "$var=\"$value\""
}

fix_signal_strength_eventmark() {
	local var=$1
	local val=$2
	local value

	value=$(echo "$val" | sed 's/droped/dropped/')

	eval export "$var=\"$value\""
}

fix_wifi_eventmark() {
	local var=$1
	local val=${2// /_}
	local value

	case $val in
	*disconnected)
		value="client disconnected"
		;;
	*connected) # 'connected' and ' connected'
		value="client connected"
		;;
	esac

	eval export "$var=\"$value\""
}

create_email_group() {
	local sec="$1"
	local smtpIP smtpPort userName password senderEmail secureConnection

	config_get smtpIP "$sec" smtpIP
	config_get smtpPort "$sec" smtpPort
	config_get userName "$sec" userName
	config_get password "$sec" password
	config_get senderEmail "$sec" senderEmail
	config_get secureConnection "$sec" secureConnection ""

	uci_add user_groups email
	uci_set user_groups "$CONFIG_SECTION" name "event_reporting_$sec"
	uci_set user_groups "$CONFIG_SECTION" smtp_ip "$smtpIP"
	uci_set user_groups "$CONFIG_SECTION" smtp_port "$smtpPort"
	uci_set user_groups "$CONFIG_SECTION" username "$userName"
	uci_set user_groups "$CONFIG_SECTION" password "$password"
	uci_set user_groups "$CONFIG_SECTION" senderemail "$senderEmail"
	uci_set user_groups "$CONFIG_SECTION" secure_conn "$secureConnection"
	[ -z "$userName" ] && [ -z "$password" ] ||
		uci_set user_groups "$CONFIG_SECTION" credentials 1
}

fix_rule() {
	local sec="$1"
	local event mark action new_mark

	config_get event "$sec" event
	config_get mark "$sec" eventMark
	config_get action "$sec" action

	tmp_event=${event// /_}
	case $tmp_event in
	Config)
		fix_config_eventmark new_mark "$mark"
		;;
	Backup)
		event="Failover"
		[ "$mark" = "backup" ] &&
			new_mark="to backup" || new_mark="$mark"
		;;
	Port)
		event="Switch Events"
		fix_simswitch_eventmark new_mark "$mark"
		;;
	Signal_strength)
		fix_signal_strength_eventmark new_mark "$mark"
		;;
	SIM_switch)
		fix_simswitch_eventmark new_mark "$mark"
		;;
	Reboot)
		fix_reboot_eventmark new_mark "$mark"
		;;
	WiFi)
		fix_wifi_eventmark new_mark "$mark"
		;;
	Restore_Point)
		uci_remove events_reporting "$sec"
		return
		;;
	esac

	[ "$action" = "sendEmail" ] && {
		create_email_group "$sec"
		uci_set events_reporting "$sec" emailgroup "event_reporting_$sec"
	}

	uci_set "events_reporting" "$sec" "event" "$event"
	uci_set "events_reporting" "$sec" "eventMark" "${new_mark:-$mark}"
}

config_load events_reporting
config_foreach fix_rule rule
uci_commit events_reporting
