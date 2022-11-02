#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

fix_relay() {
	config_get enabled dhcp_relay enabled 0
	[ "$enabled" = "1" ] && [ "$1" = "lan" ] && uci_remove dhcp "$1" ignore
}

configure_dhcp_relay() {
	local enabled server lan_ip

	config_get enabled dhcp_relay enabled 0
	[ "$enabled" = 0 ] && return 0

	config_get server dhcp_relay server
	lan_ip="$(uci_get network lan ipaddr)"
	uci_set dhcp lan server_relay "$server"
	uci_add dhcp relay
	uci_set dhcp "$CONFIG_SECTION" local_addr "$lan_ip"
	uci_set dhcp "$CONFIG_SECTION" server_addr "$server"
}

config_load dhcp
config_foreach fix_relay dhcp
configure_dhcp_relay

uci_remove dhcp dhcp_relay
uci_remove dhcp passthrough
uci_commit dhcp
