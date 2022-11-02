#!/bin/sh

. /lib/functions.sh
. /usr/share/libubox/jshn.sh

SECTION=$1
FILE="/tmp/wget_check_file"
PINGCMD="/bin/ping"
PINGCMDV6="/bin/ping6"

log() {
	/usr/bin/logger -t ping_reboot.sh "$@"
}

get_config() {
	config_get ENABLE "$SECTION" "enable" 0
	config_get TIMEOUT "$SECTION" "time_out" 0
	config_get TIME "$SECTION" "time" 0
	config_get RETRIES "$SECTION" "retry" 0
	config_get HOST "$SECTION" "host" ""
	config_get ACTION "$SECTION" "action" 0
	config_get CURRENT_TRY "$SECTION" "current_try" 0
	config_get PACKET_SIZE "$SECTION" "packet_size" 0
	config_get MODEM "$SECTION" "modem" ""
	config_get TYPE "$SECTION" "type" ""
	config_get STOP_ACTION "$SECTION" "stop_action" 0
	# Determines if ping is done using any available interface (1) or any mobile interface (2)
	config_get IF_TYPE "$SECTION" "interface" 1
	config_get PHONE_LIST "$SECTION" "number" ""
	config_get MESSAGE "$SECTION" "message" ""
	config_get MODEM_ID_SMS "$SECTION" "modem_id_sms" ""
	config_get IP_TYPE "$SECTION" ip_type "ipv4"
}

set_uci_fail_counter() {
	# Check if non-negative integer
	if echo "$1" | grep -qE '^[0-9]+$'; then
		uci_set ping_reboot "$SECTION" current_try "$1"
		uci_commit ping_reboot
	fi
}

restart_modem() {
	/bin/ubus call gsmd reinit_modems "{\"id\":\"$1\"}"
}

restart_mobile_interface() {
	ifdown "$1"
	sleep 1
	ifup "$1"
}

get_l3_device() {
	local interface=$1
	local status=$(ubus call network.interface status "{ \"interface\" : \"$interface\" }" 2>/dev/null)
	[ -z "$status" ] && return

	json_init
	json_load "$status"
	json_get_var up "up"

	[ "$up" -ne 1 ] && return

	ACTIVE_INTERFACE="$interface"

	json_get_var l3_device "l3_device"
	IF_OPTION="-I ${l3_device}"
}

get_active_mobile_interface() {
	local section_name="$1"

	config_get modem "$section_name" "modem"

	[ "$modem" != "$MODEM" ] && return

	# Check IPv6, IPv4 and legacy interface names for an l3_device
	[ -z "$IF_OPTION" ] && get_l3_device "${section_name}_6"
	[ -z "$IF_OPTION" ] && get_l3_device "${section_name}_4"
	[ -z "$IF_OPTION" ] && get_l3_device "${section_name}"
}

get_modem() {
	local modem modems id builtin primary
	local primary_modem=""
	local builtin_modem=""
	json_init
	json_load_file "/etc/board.json"
	json_get_keys modems modems
	json_select modems

	for modem in $modems; do
		json_select "$modem"
		json_get_vars builtin primary id

		[ -z "$id" ] && {
			json_select ..
			continue
		}

		[ "$builtin" ] && builtin_modem=$id
		[ "$primary" ] && {
			primary_modem=$id
			break
		}

		json_select ..
	done

	[ -n "$primary_modem" ] && {
		MODEM=$primary_modem
		return
	}

	if [ -n "$builtin_modem" ]; then
		primary_modem=$builtin_modem
	else
		json_load "$(/bin/ubus call gsmd get_modems)"
		json_get_keys modems modems
		json_select modems

		for modem in $modems; do
			json_select "$modem"
			json_get_vars id
			primary_modem=$id
			break
		done
	fi

	MODEM=$primary_modem
}

send_sms() {
	for phone in $PHONE_LIST; do
		local res=$(gsmctl --usb "${MODEM_ID_SMS:-$MODEM}" --sms --send "${phone} ${MESSAGE}")

		if [ "$res" != "OK" ]; then
			set_uci_fail_counter "$CURRENT_TRY"
		fi
	done
}

exec_action() {
	case "$ACTION" in
	"1")
		log "Rebooting router after ${CURRENT_TRY} unsuccessful tries"
		reboot "$1"
		;;
	"2")
		log "Restarting modem after ${CURRENT_TRY} unsuccessful tries"
		ubus call log write_ext "{
			\"event\": \"Restarting modem after ${CURRENT_TRY} unsuccessful tries\",
			\"sender\": \"Ping Reboot\",
			\"table\": 0,
			\"write_db\": 1,
		}"
		restart_modem "$MODEM"
		;;
	"4")
		log "Reregistering after ${CURRENT_TRY} unsuccessful tries"
		ubus call log write_ext "{
			\"event\": \"Reregistering after ${CURRENT_TRY} unsuccessful tries\",
			\"sender\": \"Ping Reboot\",
			\"table\": 0,
			\"write_db\": 1,
		}"
		restart_modem "$MODEM"
		;;
	"5")
		log "Restarting mobile data connection after ${CURRENT_TRY} unsuccessful retries"
		ubus call log write_ext "{
			\"event\": \"Restarting mobile data connection after ${CURRENT_TRY} unsuccessful retries\",
			\"sender\": \"Ping Reboot\",
			\"table\": 0,
			\"write_db\": 1,
		}"
		restart_mobile_interface "$ACTIVE_INTERFACE"
		;;
	"6")
		log "Sending message after ${CURRENT_TRY} unsuccessful retries"
		ubus call log write_ext "{
			\"event\": \"Sending message after ${CURRENT_TRY} unsuccessful retries\",
			\"sender\": \"Ping Reboot\",
			\"table\": 0,
			\"write_db\": 1,
		}"
		send_sms
		;;
	"3" | *)
		log "${CURRENT_TRY} unsuccessful ${TYPE} tries"
		;;
	esac
}

is_over_limit() {
	local over_limit

	if [ "$STOP_ACTION" -eq 1 ]; then
		json_init
		json_load "$(ubus call quota_limit get_quota_status "{\"iface\":\"${ACTIVE_INTERFACE}\"}")"
		json_get_var over_limit "status"
	else
		over_limit=0
	fi

	[ "$over_limit" -eq 1 ]
}

check_tries() {
	log "$2"

	if [ "$CURRENT_TRY" -ge "$RETRIES" ]; then
		if is_over_limit; then
			log "Action stopped. Data limit reached."
		else
			set_uci_fail_counter 0
			exec_action "$1"
		fi
	else
		log "$3"
	fi
}

perform_ping() {
	local ping_cmd="$PINGCMD"

	[ "$IP_TYPE" = "ipv6" ] && ping_cmd="$PINGCMDV6"
	if $ping_cmd $IF_OPTION -W "$TIMEOUT" -s "$PACKET_SIZE" -q -c 1 "$HOST" >/dev/null 2>&1; then
		set_uci_fail_counter 0
		log "Ping successful."
	else
		check_tries "-p" "Host ${HOST} unreachable" "${TIME} min. until next ping retry"
	fi
}

perform_wget() {
	wget -q -T "$TIMEOUT" "$HOST" -O $FILE >/dev/null 2>&1

	if [ ! -s $FILE ]; then
		check_tries "-g" "Can't wget URL." "Will be retrying wget"
	else
		set_uci_fail_counter 0
		log "Wget URL successful."
	fi

	rm $FILE >/dev/null 2>&1
}

config_load ping_reboot
get_config

[ "$ENABLE" -ne 1 ] && return

CURRENT_TRY=$((CURRENT_TRY + 1))
set_uci_fail_counter $CURRENT_TRY

[ -z "$MODEM" ] && {
	get_modem
	uci_set ping_reboot "$SECTION" modem "$MODEM"
	uci_commit ping_reboot
}

[ -z "$ACTIVE_INTERFACE" ] && {
	config_load network
	config_foreach get_active_mobile_interface "interface"

	config_load ping_reboot
}

if [ "$IF_TYPE" = "2" ] || [ -z "$HOST" ]; then
	[ -z "$IF_OPTION" ] && {
		check_tries "-p" "No mobile data connection active" "${TIME} min. until next ping retry"
		exit
	}

	if echo "$ACTIVE_INTERFACE" | grep "2"; then
		config_get HOST "$SECTION" "host2"
		config_get IP_TYPE "$SECTION" ip_type2 "ipv4"
	else
		config_get HOST "$SECTION" "host1"
		config_get IP_TYPE "$SECTION" ip_type1 "ipv4"
	fi
else
	IF_OPTION=""
fi

case "$TYPE" in
"ping")
	perform_ping
	;;
"wget")
	perform_wget
	;;
esac
