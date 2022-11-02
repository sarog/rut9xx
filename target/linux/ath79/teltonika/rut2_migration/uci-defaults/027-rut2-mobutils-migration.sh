#!/bin/sh

. /lib/functions/migrate.sh

[ -f "/etc/config/teltonika" ] || return 0

sms_to_email_cb() {
	local smtpip smtpport secureconne username password senderemail email_name
	local group_sec
	local val=$1

	config_get smtpip forwarding_to_smtp smtpip
	config_get smtpport forwarding_to_smtp smtpport
	config_get secureconnection forwarding_to_smtp secureconnection
	config_get username forwarding_to_smtp username
	config_get password forwarding_to_smtp password
	config_get senderemail forwarding_to_smtp senderemail

	[ -z "$smtpip" -o -z "$smtpport" ] && return 0

	uci_add user_groups email
	uci_set user_groups "$CONFIG_SECTION" name "sms_to_email_group"
	uci_set user_groups "$CONFIG_SECTION" smtp_ip "$smtpip"
	uci_set user_groups "$CONFIG_SECTION" smtp_port "$smtpport"
	uci_set user_groups "$CONFIG_SECTION" secure_conn "$secureconnection"
	uci_set user_groups "$CONFIG_SECTION" username "$username"
	uci_set user_groups "$CONFIG_SECTION" password "$password"
	uci_set user_groups "$CONFIG_SECTION" senderemail "$senderemail"
	[ -n "$username" ] || [ -n "$password" ] &&
		uci_set user_groups "$CONFIG_SECTION" credentials "1"

	uci_commit user_groups

	_OPTION_VALUE="sms_to_email_group"

	return 1
}

get_modem() {
	local primary modem
	local section="$1"

	config_get primary "$section" primary ""
	[ "$primary" = "1" ] || return
	config_get modem "$section" modem ""
	_OPTION_VALUE="$modem"
}

modem_cb() {
	config_load simcard
	config_foreach get_modem sim

	return 1
}

limit_cb() {
	config_get limit "$2" limit ""
	_OPTION_VALUE=$(((limit + 135) / 136))

	return 1
}

migrate_io_set() {
	local section=$1 io_pin

	config_get io_pin "$section" "outputnb"

	uci_set sms_utils "$section" io "$(echo "$io_pin" | awk '{print tolower($0)}')"
	uci_set sms_utils "$section" action "io_set"
}

fix_rules() {
	local action
	local section=$1

	config_get action "$section" action ""

	[ "$action" = "dout" ] && migrate_io_set "$section"
	[ "$action" = "monitoring" ] && uci_set sms_utils "$section" action "rms_action"
	[ "$action" = "web_access" ] && uci_rename sms_utils "$section" web_access_enabled_https webs_access_enabled
	uci_rename sms_utils "$section" authorisation authorization

	[ "$action" = "get_configure" ] && uci_remove sms_utils "$section"
	[ "$action" = "set_configure" ] && uci_remove sms_utils "$section"
}

fix_groups() {
	local name tel
	local section=$1

	config_get name "$section" name ""
	config_get tel "$section" tel ""

	uci_add user_groups phone
	uci_set user_groups "$CONFIG_SECTION" name "$name"
	for t in $tel; do
		uci_add_list user_groups "$CONFIG_SECTION" tel "$t"
	done
	uci_commit user_groups
	uci_remove sms_utils "$section"
}

add_sender_num_cb() {
	local old_section="$2"
	local new_section="$3"

	local number_name
	local sender_num=0

	config_get number_name "$old_section" "number_name" ""

	[ -n "$number_name" ] && sender_num=1

	uci_set sms_gateway "$new_section" "sender_num" "$sender_num"
}

fix_call_io_pin() {
	local section=$1 action outputnb pin

	config_get action "$section" "action"
	[ "$action" = "dout" ] || return

	config_get outputnb "$section" "outputnb"
	pin=$(migrate_io_pin "$outputnb")

	uci_set call_utils "$section" pin "$pin"
	uci_commit call_utils
}

config_load sms_utils
config_foreach fix_rules rule

[ -f "/etc/config/user_groups" ] || touch /etc/config/user_groups
config_foreach fix_groups group

uci_commit sms_utils
uci_commit user_groups

config_load call_utils
config_foreach fix_call_io_pin rule
uci_commit call_utils

migrate /etc/migrate.conf/email_to_sms.json
migrate /etc/migrate.conf/sms_auto_reply.json
migrate /etc/migrate.conf/sms_gateway.json
migrate /etc/migrate.conf/sms_storage.json
