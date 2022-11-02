#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

fix_server() {
	local section="$1"
	config_get name "$section" _name
	uci_set xl2tpd "$section" type 'server'
}

config_load xl2tpd
config_foreach fix_server service
uci commit xl2tpd
