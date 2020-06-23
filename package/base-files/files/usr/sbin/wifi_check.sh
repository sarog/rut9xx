#!/bin/sh
aaa=$(ps)
bbb=$(echo "$aaa" | grep [w]ifi_check.sh -c)

if [ "$bbb" -eq "1" ]; then
	sleep 10
	wificonfig_wlan=`uci show wireless | grep "=wifi-iface" -c`
	ifconfig_wlan=`ifconfig | grep wlan0 -c`
	if [ "$wificonfig_wlan" -eq "$ifconfig_wlan" ]; then
		exit 0
	else
		wireless_disabled=`uci show wireless | grep "disabled='1'" -c`
		all=$(($ifconfig_wlan + $wireless_disabled));
		if [ "$wificonfig_wlan" -eq "$all" ]; then
			exit 0
		else
			wifi ifup
		fi
	fi
fi
