#!/bin/sh
# Copyright (C) 2015 Teltonika
# Ipsec firewall rules

. /lib/functions.sh

set_rule() {
	local ip netmask
	config_get ip $1 "remote_lan"
	config_get netmask "$1" "remote_mask"

	if [ $ip ] && [ $netmask ]; then
		iptables -t nat -I zone_wan_postrouting -d $ip/$netmask -j ACCEPT
	fi
}

config_load "racoon"
config_foreach set_rule "sainfo"
