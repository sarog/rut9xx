#!/bin/sh 
# Copyright (C) 2014 Teltonika

. /lib/functions.sh
. /lib/teltonika-functions.sh
WIFI_MAN_SCRIPT="/sbin/wifi"
AUTO_UPDATE_SCRIPT="/usr/sbin/auto_update.sh"
in_list=false
in_senders_list=false
in_smtp_list=false
recipients=""

SendSMS() {
	gsmctl -S -s "$1 $2"
}

DeleteSMS() {
	gsmctl -S -d "$1"
}

Check_phone() {
	local group="$1"
	local tel="$2"
	group_addr=`uci show sms_utils | grep "sms_utils.@group" | grep ".name=$1" | awk -F '.' '{print $2}'`
	group_phone=`uci get -q sms_utils."$group_addr".tel`
	IFS=$' '
	for phone in $group_phone
	do
		if [ "$phone" == "$tel" ]; then
			return "0"
		fi
	done
	return "1"
}

ManageWifi() {
	local index
	local mode="$1"
	if [ "$2" -eq 1 ]; then
		local wan_name=$(uci get -q network.wan.ifname)
		if [ "$wan_name" == "wlan0" ]; then
			index="1"
		else
			index="0"
		fi
		uci set -q wireless.@wifi-iface["$index"].user_enable="$mode"
		if [ "$mode" -eq 1 ]; then
			uci delete -q wireless.@wifi-iface["$index"].disabled
		else
			uci set -q wireless.@wifi-iface["$index"].disabled=1
		fi
		uci commit wireless
		$WIFI_MAN_SCRIPT
	else
		if [ "$mode" == "1" ]; then
			$WIFI_MAN_SCRIPT "up"
		else
			$WIFI_MAN_SCRIPT "down"
		fi
	fi
}

Manage3G() {
	local mode
	local enabled=`wan_section_enabled type mobile`

	if [ "$enabled" == "1" ] ; then
		mode="$1"
		if [ "$2" == "1" ]; then
			if [ "$mode" == "1" ]; then
				uci set -q network.ppp.enabled=1
				uci commit network
				ifup ppp >/dev/null 2>/dev/null
			else
				uci set -q network.ppp.enabled=0
				uci commit network
				ifdown ppp >/dev/null 2>/dev/null
			fi
		else
			if [ "$mode" == "1" ]; then
				ifup ppp >/dev/null 2>/dev/null
			else
				ifdown ppp >/dev/null 2>/dev/null
			fi
		fi
	fi
}

SendStatus() {
	local text=""
	local message="$2"
	local phone="$1"

	text=`/usr/sbin/parse_msg "$message"`

	SendSMS "$phone" "$text"
}
