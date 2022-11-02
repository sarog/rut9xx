#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

MIN_COUNT=
IP_ENTRIES=
DROPBEAR_PORT=

add_ip_block_entry(){
	local ip="$1"
	local port="$2"
	uci_add ip_blockd entry
	[ "$?" -eq 0 ] && {
		uci_set ip_blockd "$CONFIG_SECTION" ip "$ip"
		uci_set ip_blockd "$CONFIG_SECTION" port "$port"
		uci_set ip_blockd "$CONFIG_SECTION" counter "$MIN_COUNT"
	}
}

fix_block(){
	local sec="$1"
	local name="$2"

	config_get ip "$sec" ip

	[ "$name" = "dropbear" ] && {
		add_ip_block_entry "$ip" "$DROPBEAR_PORT"
	}

	[ "$name" = "uhttpd" ] && {
		add_ip_block_entry "$ip" 80
		add_ip_block_entry "$ip" 443
	}
}

get_min_count() {
	local sec="$1"
	local count
	config_get count "$sec" maxfail 10
	[ ! "${MIN_COUNT}" ] || [ "$count" -lt "$MIN_COUNT" ] && {
		MIN_COUNT="$count"
	}
}

change_max_count() {
	local sec="$1"
	uci_set ip_blockd "$sec" max_attempt_count "$MIN_COUNT"
}

get_dropber_port() {
	local sec="$1"
	config_get DROPBEAR_PORT "$sec" Port 22
}

config_load logtrigger
config_foreach get_min_count rule

config_load ip_blockd
config_foreach change_max_count globals

config_load dropbear
config_foreach get_dropber_port dropbear

config_load blocklist
config_foreach fix_block dropbear "dropbear"
config_foreach fix_block uhttpd "uhttpd"

uci_commit ip_blockd
rm /etc/config/blocklist

