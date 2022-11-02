#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

fix_enabled() {
	config_get proto "$1" proto 0
	[ "$proto" = "wireguard" ] || continue

	config_get enabled "$1" enabled 0
	config_get disabled "$1" disabled 1

	if [ "$disabled" -eq 0 ]; then
		return 0
	elif [ "$enabled" -eq 1 ]; then
		uci_set network "$1" disabled 0
	fi
	uci_remove network "$1" enabled
}

config_load network
config_foreach fix_enabled interface
uci_commit
