#!/bin/sh

. /lib/functions.sh

TIMEOUT=1
ITERATION=0
CONFIGURATION_ID=""
AP_SSID=""
STA_SSID=""
FAILURE_COUNT=0
USER_ENABLE="0"

get_sta() {
	config_get mode "$1" mode ""
	[[ "$mode" != "sta" ]] && return

	config_get ssid "$1" ssid ""
	[[ "$ssid" == "" ]] && return

	CONFIGURATION_ID="$1"
	STA_SSID="$ssid"
}

get_sta_usr_enabled() {
	config_get mode "$1" mode ""
	[[ "$mode" != "sta" ]] && return

	config_get user_enable "$1" user_enable ""
	[[ "$user_enable" != "1" ]] && return

	USER_ENABLE="1"
}

get_ap() {
	config_get mode "$1" mode ""
	[[ "$mode" != "ap" ]] && return

	config_get disabled "$1" disabled ""
	[[ "$disabled" == "1" ]] && return

	config_get ssid "$1" ssid ""
	[[ "$ssid" == "" ]] && return

	AP_SSID="$ssid"
}

disable_sta() {
	if [ "$(uci -q get wireless.$CONFIGURATION_ID.disabled)" != "1" ]; then
		logger -t "fix_sta_ap" "Disabling STA"
		uci -q set wireless.$CONFIGURATION_ID.disabled='1'
		wifi up
	fi
}

enable_sta() {
	if [ "$(uci -q get wireless.$CONFIGURATION_ID.disabled)" == "1" ]; then
		logger -t "fix_sta_ap" "Enabling STA"
		uci -q delete wireless.$CONFIGURATION_ID.disabled
		wifi up
	fi
}

scan_before_enable() {
	if [ "$(uci -q get wireless.$CONFIGURATION_ID.disabled)" == "1" ]; then
		results="$(iw dev wlan0 scan | grep "SSID: $STA_SSID" | cut -d':' -f2 | sed -e 's/^[ \t]*//')"
		error_code=$?
		if [ "$results" == "$STA_SSID" ]; then
			enable_sta
		elif [ "$error_code" != "0" ]; then
			#It should never reach this condition, because sed never generate error code if string is empty or not
			enable_sta
		fi
	fi
}

logger -t "fix_sta_ap" "fix_sta_ap.sh STARTING"

while [ 1 -ne 0 ]; do
	SLEEP=20
	#clear global variables, because functions return if conditions is not met
	AP_SSID=""
	CONFIGURATION_ID=""
	USER_ENABLE=""

	config_load "wireless"
	config_foreach get_ap "wifi-iface"
	config_foreach get_sta "wifi-iface"
	if [ "$AP_SSID" != "" ]; then
		if [ "$CONFIGURATION_ID" != "" ]; then
			if [ $(iwinfo | grep -c "ESSID: unknown") -ge 1 ]; then
				if [ $FAILURE_COUNT -ge 3 ]; then
					disable_sta
					FAILURE_COUNT=0
					sleep 10
				else
					let FAILURE_COUNT=$FAILURE_COUNT+1
					sleep 2
					SLEEP=0
				fi
			else
				if [ $ITERATION -ge $TIMEOUT ]; then
					scan_before_enable
					ITERATION=0
					sleep 10
				fi
			fi
		else
			ITERATION=0
		fi
		let ITERATION=$ITERATION+1
	else
		config_foreach get_sta_usr_enabled "wifi-iface"
		if [ "$USER_ENABLE" == "1" ] && [ "$CONFIGURATION_ID" != "" ]; then
			enable_sta
			uci commit
			exit 0
		fi
	fi
	sleep $SLEEP
done
