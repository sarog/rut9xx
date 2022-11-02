#!/bin/sh

. /lib/functions.sh
. /lib/netifd/netifd-proto.sh

DEVICENAME="$1"

find_wwan_iface() {
	local cfg="$1"
	local proto modem enabled
	config_get proto "$cfg" proto
	config_get modem "$cfg" modem
	config_get enabled "$cfg" auto

	[ "$proto" = wwan ] && [ "$modem" = "$DEVICENAME" ]  || return 0
	proto_set_available "$cfg" 1
	[ "$enabled" != "0" ] || return 0
	ifup "$cfg"
}

config_load network
config_foreach find_wwan_iface interface
