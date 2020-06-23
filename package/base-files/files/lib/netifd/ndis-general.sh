#!/bin/sh

# NDIS connection functions

# Returns WAN ifname that uses given interface
# 1: interface name
ndis_get_wan_ifname() {
	WAN=`uci -q get network.wan.ifname`
	WAN2=`uci -q get network.wan2.ifname`
	if [ "$WAN2" = "$1" ]; then
		WAN_X="wan2"
	elif [ "$WAN" = "$1" ]; then
		WAN_X="wan"
	else
		logger -t "$0" "Error. Can't match NDIS interface to any WAN"
	fi
	echo "$WAN_X"
}
