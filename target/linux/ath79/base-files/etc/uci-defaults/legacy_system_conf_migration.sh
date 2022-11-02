#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0
[ -f "/etc/config/buttons" ] || touch "/etc/config/buttons"

EXIST=0
LED_ON=1
MAX=

move_option() {
	local option="$1"
	local new_option="$2"

	config_get value "$_SECTION_OLD" "$option"
	[ -n "$value" ] || return 0

	[ "$value" = "firstboot && reboot" ] && value="firstboot"

	uci_set buttons "$_SECTION_NEW" "${new_option:-$option}" "$value"
}

init_sections() {
	_SECTION_OLD="$1"
	_SECTION_NEW="${2:-$_SECTION_OLD}"
}

move_button() {
	local section="$1"

	uci_add buttons button
	[ "$?" -ne 0 ] && return 0

	config_get min "$section" min 0
	[ -n "$MAX" ] && [ "$min" -eq "$MAX" ] && min=$((min+1))

	config_get MAX "$section" max 0
	init_sections "$section" "$CONFIG_SECTION"
	uci_set buttons "$CONFIG_SECTION" min "$min"
	move_option max
	move_option handler
	move_option action
	move_option enabled
	uci_remove system "$section"
}

remove_leds(){
	local section="$1"

	[ "$(uci_get system "$1" enable 1)" -eq 0 ] && LED_ON=0

	uci_remove system "$section"
}

add_user_group() {
	grep "user:x:2" /etc/group || \
		echo "user:x:2:" >> /etc/group
}

mk_leds_conf() {
        . /bin/config_generate

        json_init
        json_load "$(cat /etc/board.json)"
        json_get_keys keys led

        for key in $keys; do generate_led $key; done

		uci_add system ledman ledman
		uci_set system ledman enabled "$LED_ON"
}

rule_exists() {
	local sec="$1"
	local val="$2"
	local action

	config_get action "$sec" action
	[ "$action" = "$val" ] && EXIST=1
}

#empty config before migration
echo > /etc/config/buttons
config_load system
config_foreach move_button button
config_foreach remove_leds leds

#remove old sections
uci_remove system device_info
uci_remove system ipv6
uci_remove system module
uci_remove system usb_led

#remove unused options
uci_remove system system enable_pppd_debug
uci_remove system system enable_chat_log
uci_remove system system enable_gsmd_log
uci_remove system system enable_hotplug_log
uci_remove system system enable_luci_reload_log
uci_remove system system enable_sim_switch_log
uci_remove system system sms_utils_debug_level

uci_add system timeserver ntp
uci_set system ntp enabled 0
uci_set system ntp enable_server 0
uci_add_list system ntp server "0.pool.ntp.org"
uci_add_list system ntp server "1.pool.ntp.org"
uci_add_list system ntp server "2.pool.ntp.org"
uci_add_list system ntp server "3.pool.ntp.org"

uci_add system debug debug
uci_add_list system debug sms_utils_debug_level "4"

mk_leds_conf
add_user_group

config_load buttons
config_foreach rule_exists button released
[ "$EXIST" -ne 1 ] && {
	uci_add buttons button
	uci_set buttons "$CONFIG_SECTION" min 6
	uci_set buttons "$CONFIG_SECTION" max 11
	uci_set buttons "$CONFIG_SECTION" handler default
	uci_set buttons "$CONFIG_SECTION" action released
}

uci_commit
