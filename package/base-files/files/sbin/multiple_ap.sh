#!/bin/sh

. /lib/functions.sh

PRIORITY=999999
TEMP_PRIORITY=999999
SLEEP="$1"
BLOCK="$2"
STATION=""
ENCRYPTION=""
KEY=""
RETRY=0
TEMP_RETRY=0
SECTION=""
MULTI_SECTION=""
CONNECTED=0

get_section() {
	config_get mode "$1" mode ""
	[[ "$mode" != "sta" ]] && return
	SECTION="$1"
}

find_available() {
	config_get enabled "$1" enabled "0"
	[ "$enabled" == 0 ] && return

	config_get priority "$1" priority ""
	if [ "$priority" -lt "$PRIORITY" ]; then
		config_get ssid "$1" ssid ""
		config_get blocked "$1" blocked 0
		if [ "$blocked" -eq 1 ]; then
			timestamp=`date +%s`
			config_get blocked_time "$1" blocked_time ""
			if [ "$timestamp" -le "$blocked_time" ]; then
				return
			else
				uci -q delete multi_wifi."$1".blocked
				uci -q delete multi_wifi."$1".blocked_time
				uci commit multi_wifi
			fi
		fi
		result=`cat /tmp/available_wifi | grep -wc "$ssid"`
		if [ "$result" -gt 0 ]; then
			STATION="$ssid"
			config_get ENCRYPTION "$1" encryption ""
			config_get KEY "$1" key ""
			config_get RETRY "$1" retry ""
			PRIORITY="$priority"
			MULTI_SECTION="$1"
		fi
	fi
}

connect_to_wifi() {
	config_load "wireless"
	config_foreach get_section "wifi-iface"
	if [ "$SECTION" == "" ]; then
		wifi_sec=`uci -q add wireless wifi-iface`
		SECTION="$wifi_sec"
	fi
	uci -q delete wireless."$SECTION".disabled
	uci -q set wireless."$SECTION".network='wan'
	uci -q set wireless."$SECTION".encryption="$ENCRYPTION"
	uci -q set wireless."$SECTION".device='radio0'
	uci -q set wireless."$SECTION".key="$KEY"
	uci -q set wireless."$SECTION".user_enable='1'
	uci -q set wireless."$SECTION".mode='sta'
	uci -q set wireless."$SECTION".scan_sleep='10'
	uci -q set wireless."$SECTION".ssid="$STATION"
	uci commit wireless
	wifi up

	sleep 20
	check_if_connected
	if [ "$CONNECTED" -ne 1 ]; then
		TEMP_RETRY=$((TEMP_RETRY + 1))
		if [ "$TEMP_RETRY" -lt "$RETRY" ]; then
			connect_to_wifi
		else
			timestamp=`date +%s`
			uci -q set multi_wifi."$MULTI_SECTION".blocked=1
			num=$((timestamp + BLOCK))
			uci -q set multi_wifi."$MULTI_SECTION".blocked_time="$num"
			uci commit multi_wifi
			TEMP_RETRY=0
		fi
	else
		TEMP_RETRY=0
		TEMP_PRIORITY=$PRIORITY
	fi
}

check_if_connected() {
	result=`iwinfo | grep -wc "ESSID: \"$STATION\""`
	if [ "$result" -gt 0 ]; then
		CONNECTED=1
	else
		CONNECTED=0
	fi
}

while [ 1 ]; do
	if [ "$STATION" != "" ] && [ "$ENCRYPTION" != "" ]; then
		check_if_connected
	fi

	if [ "$CONNECTED" -eq 0 ]; then
		PRIORITY=999999
		TEMP_PRIORITY=999999
		STATION=""
	fi

	rm /tmp/available_wifi
	touch /tmp/available_wifi
	error=`iw dev wlan0 scan 2>&1`
	if [ `echo $error | grep "No such device" | wc -l` -ne 0 ]; then
		iw phy phy0 interface add wlan0 type managed; ifconfig wlan0 up
		sleep 5
		iw dev wlan0 scan | grep -w "SSID:" >> /tmp/available_wifi
	else
		echo "$error" | grep -w "SSID:" >> /tmp/available_wifi
	fi

	config_load "multi_wifi"
	config_foreach find_available "wifi-iface"
	if [ "$STATION" != "" ] && [ "$ENCRYPTION" != "" ] && [ "$PRIORITY" -ne "$TEMP_PRIORITY" ]; then
		connect_to_wifi
	fi
	sleep $SLEEP
done