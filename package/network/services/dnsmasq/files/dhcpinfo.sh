#!/bin/sh

action="$1"
lease_mac="$2"
ip="$3"
dev_name="$4"
conn_inter="LAN"

[ -z "$dev_name" ] && exit

[ -n "$lease_mac" ] && hash iw 2>/dev/null && {
	for ifname in $(wifi status | grep ifname | awk '{print $2}' | tr -d '",'); do
		iw dev "$ifname" station dump | grep -q "$lease_mac" && {
			conn_inter="WiFi"
			break
		}
	done
}

# todo: maybe we need different messages for every action
case "$1" in
	add)
		ubus call log write_ext "{
			\"event\": \"Leased $ip IP address for client $lease_mac - $dev_name in $conn_inter\",
			\"sender\": \"DHCP\",
			\"table\": 2,
			\"write_db\": 1,
		}"
	;;
esac
