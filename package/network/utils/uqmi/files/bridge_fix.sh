#!/bin/sh
# 
# Copyright (C), 2021 Teltonika
#
. /lib/functions.sh


find_bridge() {
	local sect="$1"
	config_get method "$sect" method

	[ "$method" = "bridge" ] && {
		bridge_name="$sect"
	}
}

fix_bridge() {
	local sect="$1"
	local ifnames new_ifnames
	[ "lan_${bridge_name}" = "$sect" ] &&
        {
		config_get ipaddr "$sect" ipaddr
		config_get netmask "$sect" netmask
		config_get proto "$sect" proto

		ifnames="$(uci_get network lan ifname ifname)"

		for i in $ifnames; do
			[ "wwan" = "${i:0:4}" ] || {
				new_ifnames="$new_ifnames $i"
			}
		done

		uci_set network lan ifname "${new_ifnames}"
		uci_set network lan ipaddr "$ipaddr"
		uci_set network lan netmask "$netmask"
		uci_set network lan proto "$proto"

		uci_remove network "lan_$bridge_name"
		uci_commit network
	}
}

config_load network
config_foreach find_bridge interface
[ -z "$bridge_name" ] && exit 0

config_foreach fix_bridge interface
exit 0
