#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

ENABLED_INSTANCES=0

check_multi_iface() {
	local enabled

	config_get enabled "$1" enabled ""
	[ "$enabled" = "1" ] && ENABLED_INSTANCES=1
}

check_wifi_iface() {
	local section="$1"
	local mode

	config_get mode "$section" mode ""
	[ "$mode" = "sta" ] || return
	uci_set wireless "$section" multiple "1"
	uci_commit wireless
	exit 0
}

config_load multi_wifi
config_get enabled general enabled ""
[ "$enabled" = "1" ] || exit 0
config_foreach check_multi_iface wifi-iface

[ "$ENABLED_INSTANCES" -eq 1 ] && {
	config_load wireless
	config_foreach check_wifi_iface
}

exit 0
