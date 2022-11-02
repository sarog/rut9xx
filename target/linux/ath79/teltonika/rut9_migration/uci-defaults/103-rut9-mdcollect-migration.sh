#!/bin/sh

. /lib/functions.sh

config_load mdcollectd
config_get ignore config ignore
[ -n "$ignore" ] || {
	uci_add_list mdcollectd config ignore "lo"
	uci_add_list mdcollectd config ignore "wwan0"
}

uci_commit mdcollectd
