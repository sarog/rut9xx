#!/bin/sh

. /lib/functions.sh

PORT=

move_port_section() {
	config_get enable_mirror_rx "$1" enable_mirror_rx 0
	config_get enable_mirror_tx "$1" enable_mirror_tx 0
	config_get port "$1" port

	[ "$enable_mirror_tx" = "1" ] || [ "$enable_mirror_rx" = "1" ] || continue

	[ -n "$PORT" ] && PORT= || PORT="$port"
			
	[ "$enable_mirror_tx" = "1" ] && {
		uci_set network @switch[0] enable_mirror_tx 1
		uci_remove network "$1" enable_mirror_tx
	}
	[ "$enable_mirror_rx" = "1" ] && {
		uci_set network @switch[0] enable_mirror_rx 1
		uci_remove network "$1" enable_mirror_rx
	}
}


config_load network

config_foreach move_port_section switch_port

[ -n "$PORT" ] && uci_set network @switch[0] mirror_source_port "$PORT"

uci_commit network
