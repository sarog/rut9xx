#!/bin/sh

LOG_FILE_PATH="/tmp/"
MAX_LOG_FILE_SIZE=102400	#100 KB

state="$1"
lease_mac="$2"
ip="$3"
dev_name="$4"
conn_inter=""
small_mac=""

if [ "$ip" != "" ]; then
	small_mac=`cat /var/dhcp.leases | grep $ip | awk -F ' ' '{print $2}'`
fi

mac=$(echo "$small_mac" | awk '{print toupper($0)}')

if [ "$small_mac" != "" ]; then
	conn_inter=`iw dev wlan0 station dump | grep $small_mac`
fi

if [ "$conn_inter" != "" ]; then
	conn_inter="WiFi"
else
	conn_inter="LAN"
fi

if [ -n "$dev_name" ] && [ "$dev_name" != "" ]; then
	/usr/bin/eventslog -i -t EVENTS -n "DHCP" -e "Leased $ip IP address for client $mac - $dev_name in $conn_inter"
else
	/usr/bin/eventslog -i -t EVENTS -n "DHCP" -e "Leased $ip IP address for $mac in $conn_inter"
fi

