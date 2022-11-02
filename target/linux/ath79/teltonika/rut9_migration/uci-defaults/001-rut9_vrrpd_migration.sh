#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

fix_vrrpd(){
	local sec="$1"_ping

	uci_get vrrpd "$sec"
	[ "$?" -eq 0 ] && return 0

	uci_add vrrpd ping "$sec"
	[ "$?" -ne 0 ] && return 0
}

config_load vrrpd
config_foreach fix_vrrpd vrrpd
uci_commit vrrpd