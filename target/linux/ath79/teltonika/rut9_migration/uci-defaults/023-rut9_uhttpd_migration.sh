#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

fix_hotspot_instance() {
	uci_set uhttpd hotspot home "/hotspotlogin"
	uci_set uhttpd hotspot cgi_prefix "/cgi-bin"
}

fix_main_instance() {
	local enablehttps

	uci_rename uhttpd main enablehttp enable_http
	config_get enablehttps main enablehttps
	if [ -z "$enablehttps" ]; then
		uci_set uhttpd main enable_https 1
	else
		uci_rename uhttpd main enablehttps enable_https
	fi
}

remove_rms_section() {
	uci_remove uhttpd "$1"
}

config_load uhttpd
config_foreach remove_rms_section rms_uhttpd

fix_hotspot_instance
fix_main_instance
uci_commit uhttpd
