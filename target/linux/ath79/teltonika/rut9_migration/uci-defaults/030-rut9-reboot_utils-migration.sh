#!/bin/sh

[ -f "/etc/config/teltonika" ] || return 0

. /lib/functions.sh

migrate_periodic_reboot() {
	local section=$1
	local enable day_list hours minutes

	config_get enable "$section" enable 0
	config_get day_list "$section" day ""
	config_get hours "$section" hours ""
	config_get minutes "$section" minutes ""

	days=$(echo "$day_list" | sed 's| |,|g')

	section_scheduler=$(uci -q add periodic_reboot reboot_instance)

	uci_set periodic_reboot "$section_scheduler" action 1
	uci_set periodic_reboot "$section_scheduler" enable "$enable"
	uci_set periodic_reboot "$section_scheduler" days "$days"
	uci_set periodic_reboot "$section_scheduler" hour "$hours"
	uci_set periodic_reboot "$section_scheduler" minute "$minutes"

	uci_remove periodic_reboot "$section"

	uci_commit periodic_reboot
}

migrate_ping() {
	local section="$1"
	local action
	local old_action
	local fail_counter

	config_get old_action "$section" 'action' 0
	config_get fail_counter "$section" 'fail_counter' -1

	uci_set ping_reboot "$section" type 'ping'

	case $old_action in
	3)
		action=5
		;;
	5)
		action=3
		;;
	*)
		action=$old_action
		;;
	esac

	uci_set ping_reboot "$section" action "$action"

	if [ "$fail_counter" -ne -1 ]; then
		uci_set ping_reboot "$section" current_try "$fail_counter"
		uci_remove ping_reboot "$section" fail_counter
	fi

	uci_commit ping_reboot
}

migrate_wget() {
	local section="$1"
	local enable action time timeout retry host phone_list message

	config_get enable "$section" 'enable' 0
	config_get action "$section" 'action' 0
	config_get time "$section" 'time' 0
	config_get timeout "$section" 'timeout' 0
	config_get retry "$section" 'retry' 0
	config_get host "$section" 'host' ""
	config_get phone_list "$section" 'number' ""
	config_get message "$section" 'message' ""

	uci_add ping_reboot ping_reboot
	section_ping=$CONFIG_SECTION

	uci_set ping_reboot "$section_ping" enable "$enable"
	uci_set ping_reboot "$section_ping" action "$action"
	uci_set ping_reboot "$section_ping" host "$host"
	uci_set ping_reboot "$section_ping" retry "$retry"
	uci_set ping_reboot "$section_ping" time "$time"
	uci_set ping_reboot "$section_ping" time_out "$timeout"
	uci_set ping_reboot "$section_ping" time_out "$timeout"

	uci_set ping_reboot "$section_ping" type 'wget'
	uci_set ping_reboot "$section_ping" stop_action '0'
	uci_set ping_reboot "$section_ping" interface '1'

	uci_set ping_reboot "$section_ping" message "$message"

	for i in $phone_list; do
		uci_add_list ping_reboot "$section_ping" number "$i"
	done

	uci_remove wget_reboot "$section"

	uci_commit ping_reboot
}

config_load periodic_reboot
config_foreach migrate_periodic_reboot "periodic_reboot"

#--------------------------------------------------

config_load ping_reboot
config_foreach migrate_ping "ping_reboot"

#--------------------------------------------------

config_load wget_reboot
config_foreach migrate_wget "wget_reboot"
