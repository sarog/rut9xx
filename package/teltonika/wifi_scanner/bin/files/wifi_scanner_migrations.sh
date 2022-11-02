#!/bin/sh

. /lib/functions.sh

config_load "wifi_scanner" || exit 0
config_get enabled "wifi_scan" enabled
if [ "$enabled" = "1" ]; then
	uci -q set "wifi_scanner.wifi_scan.two_g_enabled=1"
	uci -q set "wifi_scanner.wifi_scan.five_g_enabled=1"
	uci -q delete "wifi_scanner.wifi_scan.enabled"
	uci commit
fi
