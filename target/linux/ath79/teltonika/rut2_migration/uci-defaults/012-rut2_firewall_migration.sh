#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

add_forwarding() {
	local name
	config_get name "$1" name
	[ "$name" = "lan" ] && return
	[ "$name" = "wan" ] && {
		uci_set firewall "$1" helper "pptp";
	}

	uci_add firewall forwarding
	uci_set firewall "$CONFIG_SECTION" src lan
	uci_set firewall "$CONFIG_SECTION" dest "$name"
}

add_missing_zone_options() {
	local network name
	config_get network "$1" network
	config_get name "$1" name

	list_contains network ppp && {
		network="${network/ppp/mob1s1a1}"
		uci_set firewall "$1" network "$network"
	}

	list_contains network wan2 && {
		network="${network/wan2/wwan}"
		uci_set firewall "$1" network "$network"
	}

	list_contains name wan && {
		list_contains network wan6 && return 0
		if [ -n "$(uci_get network wan6)" ]; then
			network="$network wan6"
			uci_set firewall "$1" network "$network"
		fi
	}

	if [ -z $(uci_get network tun) ]; then
		network="${network/tun/}"
		network="${network/  / }"
		uci_set firewall "$1" network "$network"
	fi
}

config_load firewall
config_foreach add_forwarding zone
config_foreach add_missing_zone_options zone
uci_commit firewall
