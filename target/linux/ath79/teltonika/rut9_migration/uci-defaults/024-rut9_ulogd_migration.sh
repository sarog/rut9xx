#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

move_option() {
	local option="$1"
	local new_option="$2"

	config_get value "$_SECTION_OLD" "$option"
	[ -n "$value" ] || return 0

	uci_set ulogd "$_SECTION_NEW" "${new_option:-$option}" "$value"
}

init_sections() {
	_SECTION_OLD="$1"
	_SECTION_NEW="${2:-$_SECTION_OLD}"
}

move_ftp_intervals() {
	local section="$1"

	init_sections "$section" ftp
	move_option fixed
	move_option fixed_hour hours
	move_option fixed_minute minutes
	move_option weekdays
	move_option interval_time interval
}

config_load tcplogger

init_sections ftp
move_option host
move_option user username
move_option psw password
move_option port
move_option extra_name_info

init_sections ftp global
config_get enabled general enabled 0
[ "$enabled" -eq 1 ] && move_option enabled

config_foreach move_ftp_intervals interval
uci_commit ulogd
rm /etc/config/tcplogger
