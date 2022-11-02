#!/bin/sh
# Copyright (C) 2009 OpenWrt.org

EXEC=0
setup_switch_dev() {
	local name
	config_get name "$1" name
	name="${name:-$1}"
	[ -d "/sys/class/net/$name" ] && ip link set dev "$name" up
	swconfig dev "$name" reset_and_load network

	config_get fiber_priority "$1" fiber_priority

	[ -n "$fiber_priority" ] && {
		swconfig dev "$name" set preference "$fiber_priority"
	}

	EXEC=1
}

setup_switch() {
	config_load network
	config_foreach setup_switch_dev switch

	[ "$EXEC" = "0" ] && swconfig list &>/dev/null && {
		swconfig dev "switch0" reset_and_load network;
	}
}
