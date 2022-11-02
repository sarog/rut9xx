#!/bin/sh

. /lib/functions/migrate.sh

[ -f "/etc/config/teltonika" ] || return 0

interval_cb() {
	local interval
	local option=$1

	json_get_vars default
	config_get interval sim_switch interval "$default"
	_OPTION_VALUE="$interval"

	return 1
}

enabled_cb() {
	local enabled
	local option="$1"

	json_get_vars default
	config_get enabled sim_switch enabled "$default"
	_OPTION_VALUE="$enabled"

	return 1
}

get_modem() {
	local position modem
	local section="$1"

	config_get position "$section" position ""
	[ "$position" = "$2" ] || return
	config_get modem "$section" modem ""
	_OPTION_VALUE="$modem"
}

modem_cb() {
	json_get_vars sim
	config_load simcard
	config_foreach get_modem sim "$sim"

	return 1
}

set_simcard() {
	local position
	local section="$1"
	local enabled="$3"
	local period="$4"
	local sms="$5"

	config_get position "$section" position
	[ "$position" = "$2" ] || return

	[ -z "$enabled" ] || uci_set simcard "$section" enable_sms_limit "$enabled"
	[ -z "$period" ] || uci_set simcard "$section" sms_limit "$period"
	[ -z "$sms" ] || uci_set simcard "$section" sms_limit_num "$sms"
	uci_commit simcard
}

migrate_sms_limit() {
	local enabled1 enabled2 period1 period2 sms1 sms2

	config_get enabled1 rules sms_limit_enable_sim1 ""
	config_get enabled2 rules sms_limit_enable_sim2 ""
	config_get period1 rules period_sms_sim1 ""
	config_get period2 rules period_sms_sim2 ""
	config_get sms1 rules sms_sim1 ""
	config_get sms2 rules sms_sim2 ""

	config_load simcard
	config_foreach set_simcard "sim" "1" "$enabled1" "$period1" "$sms1"
	config_foreach set_simcard "sim" "2" "$enabled2" "$period2" "$sms2"
}

config_load sim_switch

uci_get sim_switch rules
[ "$?" -ne 0 ] && return 0

migrate_sms_limit
migrate /etc/migrate.conf/sim_switch.json
uci_remove sim_switch services
uci_remove sim_switch sim_switch
uci_commit sim_switch

