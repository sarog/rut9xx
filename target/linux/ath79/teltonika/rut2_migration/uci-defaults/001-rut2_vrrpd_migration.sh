#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

if_name=

fix_vrrpd(){
	[ "$1" = "ping" ] && {
		config_get enabled "$1" enabled
		config_get ping_attempts "$1" ping_attempts
		config_get fail_counter "$1" fail_counter
		config_get host "$1" host
		config_get interval "$1" interval
		config_get time_out "$1" time_out
		config_get packet_size "$1" packet_size
		config_get retry "$1" retry

		local sec="$if_name"_ping

		uci_get vrrpd "$sec"
		[ "$?" -eq 0 ] && return 0

		uci_add vrrpd ping "$sec"
		[ "$?" -ne 0 ] && return 0

		uci_set vrrpd "$sec" enabled $enabled
		uci_set vrrpd "$sec" ping_attempts $ping_attempts
		uci_set vrrpd "$sec" fail_counter $fail_counter
		uci_set vrrpd "$sec" host $host
		uci_set vrrpd "$sec" interval $interval
		uci_set vrrpd "$sec" time_out $time_out
		uci_set vrrpd "$sec" packet_size $packet_size
		uci_set vrrpd "$sec" retry $retry

		uci -q delete vrrpd."$1"

		return
	}

	if_name="$1"
}

config_load vrrpd
config_foreach fix_vrrpd vrrpd
uci_commit vrrpd
