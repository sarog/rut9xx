#!/bin/sh

. /lib/functions.sh
. /lib/teltonika-functions.sh

k=0

add_rule(){
	local ttl_increase
	local disabled
	config_get ttl_increase "$1" "ttl_increase" "0"
	config_get disabled "$1" "disabled" "0"

	if [ "$disabled" != "1" ]; then
		if [ "$ttl_increase" == "1" ]; then
			case "$k" in
				0)
					iptables -t mangle -A PREROUTING  -j TTL -i wlan0 --ttl-inc 10
					;;
				1)
					iptables -t mangle -A PREROUTING  -j TTL -i wlan0-1 --ttl-inc 10
					;;
				2)
					iptables -t mangle -A PREROUTING  -j TTL -i wlan0-2 --ttl-inc 10
					;;
				3)
					iptables -t mangle -A PREROUTING  -j TTL -i wlan0-3 --ttl-inc 10
					;;
			esac
		fi
	fi
	k=$((k+1))

}
config_load wireless
config_foreach add_rule "wifi-iface"