#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

fix_stunnel() {
	config_get cert "$1" cert 0
	config_get key "$1" key 0
	
	[ "$cert" -eq 0 ] || [ "$key" -eq 0 ] && continue

	config_get client "$1" client 0
	
	if [ "$client" -eq 0 ]; then
		local base="/etc/vuci-uploads/cbid.stunnel.${1}."
		mv "$cert" "${base}certcert.crt" && uci set stunnel."$1".cert="${base}certcert.crt"
		mv "$key" "${base}keykey.key" && uci set stunnel."$1".key="${base}keykey.key"
	else
		uci delete stunnel."$1".cert
        	uci delete stunnel."$1".key
	fi
}

config_load stunnel
config_foreach fix_stunnel service

uci_commit stunnel
