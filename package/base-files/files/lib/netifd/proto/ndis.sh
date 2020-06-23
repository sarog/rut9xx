#!/bin/sh
INCLUDE_ONLY=1

. /lib/teltonika-functions.sh
. /lib/netifd/ndis-general.sh
. ../netifd-proto.sh

init_proto "$@"

disconnect() {
	local cmd="AT^NDISDUP=1,0"
	device="$1"
	echo "Disconnecting NDIS"
	gsmctl -A "$cmd" > /dev/null
	ret=$?
	if [ "$ret" != "0" ]; then
		echo "Error sending NDIS disconnect command. Bypassing gsmd"
		echo -ne "$cmd\r" | microcom -s 115200 "$device" -t 100
	fi
}

proto_ndis_init_config() {
	echo "NDIS init_config: $1"
	proto_config_add_string "device"
	proto_config_add_string "apn"
	proto_config_add_string "service"
	proto_config_add_string "enabled"
	proto_config_add_string "auth_mode"
	proto_config_add_string "username"
	proto_config_add_string "password"
	proto_config_add_string "ifname"
	proto_config_add_string "bridge"
	proto_config_add_string "roaming"
	proto_config_add_string "backup"
	no_device=1
	available=1
}

proto_ndis_setup() {
	echo "NDIS setup: $1"
	json_get_var enabled enabled
	json_get_var roaming roaming

	if [ "$enabled" != "1" ] && [ ! -f "/tmp/mobileon" ]; then
		ifdown ppp
		return 0
	fi

	# if roaming - exit
	if [ "$roaming" == "1" ]; then
		local variable=`gsmctl -A "AT+CREG?"`
		local stat=${variable#*,}
		if [ "$stat" == "5" ]; then
			logger -t $0 "roaming detected"
			return 1
		fi
	fi
	proto_run_command "$1" /sbin/ndisconn.sh
}

proto_ndis_teardown() {
	echo "NDIS teardown: $1"

	#Stop connection manager
	proto_kill_command "$1"
	rm -f /var/run/ndisconn.pid
	#Stop connection
	device=`uci -q get network.$1.device`
	disconnect "$device"
	#Stop WAN to stop DHCP client
	ifname=`uci -q get network.$1.ifname`
	WAN_X=`ndis_get_wan_ifname "$ifname"`
	#ifdown "$WAN_X"
	ifup "$WAN_X"
	#ifconfig $ifname down
}

add_protocol ndis
