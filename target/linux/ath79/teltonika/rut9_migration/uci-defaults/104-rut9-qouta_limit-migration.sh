#!/bin/sh
#NOTE: this script must be executed after network migration
. /lib/functions/migrate.sh

[ -f "/etc/config/teltonika" ] || return 0

modem_cb() {
	local option=$1
	local old_sec=$2
	local new_sec=$3

	_OPTION_VALUE=$(uci_get network "$new_sec" modem)

	return 1
}

config_load data_limit
uci_get data_limit limit
[ "$?" -ne 0 ] && return 0

[ -f "/etc/config/quota_limit" ] || touch /etc/config/quota_limit

migrate /etc/migrate.conf/quota_limit.json
