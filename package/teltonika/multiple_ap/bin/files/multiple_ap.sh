#!/bin/sh

. /lib/functions.sh
. /usr/share/libubox/jshn.sh

[ "$#" -ne 1 ] && {
	echo "Usage: multiple_ap.sh <timeout>"
	exit 1
}

TIMEOUT="$1"
PRIORITY=0
SSID=""
ENCRYPTION=""
BSSID=""

get_wifi_section() {
	local multiple
	local section="$1"

	config_get multiple "$section" multiple ""
	[ "$multiple" = "1" ] || return
	SECTION="$1"
	config_get DEVICE "$section" device ""
}

check_connection() {
	json_load "$(/bin/ubus call iwinfo info "{\"device\":\"$DEVICE\"}")"
	json_get_vars ssid
	[ -z "$ssid" ] && PRIORITY=0
}

find_available() {
	local enabled ssid_name priority key
	local section="$1"

	config_get enabled "$section" "enabled" 0
	config_get ssid_name "$section" "ssid" ""
	config_get priority "$section" "priority" 0
	[ "$enabled" -ne 1 ] || [ "$priority" -eq 0 ] || [ -z "$ssid_name" ] && return
	config_get key "$section" key ""

	for ap in $LIST; do
		json_select "$ap"
		json_get_vars ssid bssid
		owe=0 sae=0 psk=0 wpa1=0 wpa2=0 ccmp=0 tkip=0 aes=0

		[ "$PRIORITY" -ne 0 ] && [ "$priority" -gt "$PRIORITY" ] || [ "$ssid" != "$ssid_name" ] && {
			json_select ..
			continue
		}

		PRIORITY="$priority"
		SSID="$ssid"
		BSSID="$bssid"
		KEY="$key"
		ENCRYPTION=""

		json_select encryption
		json_get_vars enabled
		[ "$enabled" ] && [ "$enabled" -ne 0 ] || {
			ENCRYPTION="none"
			json_select ..
			json_select ..
			continue
		}

		json_select authentication
		idx=1
		while json_is_a ${idx} string; do
			json_get_var auth $idx
			[ "$auth" = "owe" ] && owe=1 && break
			[ "$auth" = "sae" ] && sae=1
			[ "$auth" = "psk" ] && psk=1
			idx=$((idx + 1))
		done
		json_select ..

		json_select wpa
		idx=1
		while json_is_a ${idx} int; do
			json_get_var wpa_psk $idx
			[ "$wpa_psk" -eq 1 ] && wpa1=1
			[ "$wpa_psk" -eq 2 ] && wpa2=1
			idx=$((idx + 1))
		done
		json_select ..

		json_select ciphers
		idx=1
		while json_is_a ${idx} string; do
			json_get_var cip $idx
			[ "$cip" = "ccmp" ] && ccmp=1
			[ "$cip" = "tkip" ] && tkip=1
			[ "$cip" = "aes" ] && aes=1
			idx=$((idx + 1))
		done

		json_select ..
		json_select ..
		json_select ..

		[ "$owe" -eq 1 ] && ENCRYPTION="owe" && continue
		[ "$sae" -eq 1 ] && {
			ENCRYPTION="sae"
			[ "$psk" -eq 1 ] && ENCRYPTION="$ENCRYPTION-mixed"
			continue
		}
		[ "$psk" -eq 1 ] && {
			ENCRYPTION="psk"
			[ "$wpa1" -eq 1 ] && [ "$wpa2" -eq 1 ] && ENCRYPTION="$ENCRYPTION-mixed"
			[ "$wpa1" -eq 0 ] && [ "$wpa2" -eq 1 ] && ENCRYPTION="${ENCRYPTION}2"
			[ "$tkip" -eq 1 ] && ENCRYPTION="$ENCRYPTION+tkip"
			[ "$ccmp" -eq 1 ] && ENCRYPTION="$ENCRYPTION+ccmp" && continue
			[ "$aes" -eq 1 ] && ENCRYPTION="$ENCRYPTION+aes" && continue
		}
	done
}

config_load "wireless"
config_foreach get_wifi_section "wifi-iface"
[ -z "$SECTION" ] && {
	echo "No multiple AP wifi section created in wireless configuration"
	exit 1
}
[ -z "$DEVICE" ] && {
	echo "No device found in wireless configuration"
	exit 1
}

config_load "multi_wifi"
json_init
while :; do
	tmp_ssid="$SSID"
	tmp_bssid="$BSSID"
	tmp_enc="$ENCRYPTION"

	[ -n "$SSID" ] && [ "$PRIORITY" -eq 1 ] && check_connection
	[ "$PRIORITY" -eq 1 ] && sleep "$TIMEOUT" && continue

	json_load "$(/bin/ubus call iwinfo scan "{\"device\":\"$DEVICE\"}")"
	json_get_keys LIST results
	json_select results
	PRIORITY=0
	config_foreach find_available "wifi-iface"
	[ "$PRIORITY" -eq 0 ] || [ -z "$SSID" ] || [ -z "$BSSID" ] || [ -z "$ENCRYPTION" ] && sleep "$TIMEOUT" && continue
	[ "$SSID" = "$tmp_ssid" ] && [ "$BSSID" = "$tmp_bssid" ] && [ "$ENCRYPTION" = "$tmp_enc" ] && sleep "$TIMEOUT" && continue

	uci_set "wireless" "$SECTION" "ssid" "$SSID"
	uci_set "wireless" "$SECTION" "bssid" "$BSSID"
	uci_set "wireless" "$SECTION" "encryption" "$ENCRYPTION"
	[ "$ENCRYPTION" != "none" ] && [ "$ENCRYPTION" != "owe" ] && uci_set "wireless" "$SECTION" "key" "$KEY"
	uci_set "wireless" "$SECTION" "disabled" "0"
	uci_commit "wireless"
	wifi up
	sleep "$TIMEOUT"
done

exit 0
