#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

make_list() {
	local sec=$1

	config_get code "$sec" code
	uci_add_list operctl "$CONFIG_SECTION" mcc_mnc "$code"
	uci_remove operctl "$sec"
}

set_list() {
	local sec=$1
	local mode=$2

	uci_set simcard "$sec" operlist 1
	uci_set simcard "$sec" opermode "$mode"
	uci_set simcard "$sec" operlist_name "operators"
}

uci_add operctl operlist
[ "$?" -ne 0 ] && return 1

uci_set operctl "$CONFIG_SECTION" name "operators"

config_load operctl
config_foreach make_list list
config_get operlist general operlist
[ "$operlist" -ne 1 ] && return 0

config_get mode general mode
uci_remove operctl general
uci_commit operctl

config_load simcard
config_foreach set_list sim "$mode"
uci_commit simcard
