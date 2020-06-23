#!/bin/sh

DEBUG="1"
NAME="NCM.SH"

#Debug
debug(){
	if [ "$DEBUG" == "1" ]; then
		logger -t "$NAME" "$1"
	fi
}

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_ncm_init_config() {
	no_device=1
	available=1
	proto_config_add_string "device:device"
	proto_config_add_string apn
	proto_config_add_string enabled
	proto_config_add_string auth
	proto_config_add_string username
	proto_config_add_string password
	proto_config_add_string pincode
	proto_config_add_string delay
	proto_config_add_string mode
	proto_config_add_string pdptype
	proto_config_add_int profile
	proto_config_add_boolean ifname
	proto_config_add_defaults
}

proto_ncm_setup() {
	local interface="$1"

	local manufacturer initialize setmode connect finalize ifname devname devpath

	local device apn enabled auth username password pincode delay mode pdptype profile ifname $PROTO_DEFAULT_OPTIONS
	json_get_vars device apn enabled auth username password pincode delay mode pdptype profile ifname $PROTO_DEFAULT_OPTIONS

# 	debug "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ interface=$1"

	if [ "$enabled" != "1" ]; then
		ifdown ppp
		return 0
	fi

	attach=$(gsmctl -n -A 'AT+CGATT?')
	if [ $attach  = "+CGATT: 1" ]; then
		gsmctl -A 'AT+CGATT=0'
		sleep 2
	fi

	#for ncm protocol we hardcode IPv4
	pdptype="IP"
	json_cid="1"


	vid=$(uci -q get system.module.vid)
	pid=$(uci -q get system.module.pid)

	[ "$metric" = "" ] && metric="0"

	[ -n "$profile" ] || profile=1

	[ -n "$ctl_device" ] && device=$ctl_device

	[ -n "$device" ] || {
		echo "No control device specified"
		proto_notify_error "$interface" NO_DEVICE
		proto_set_available "$interface" 0
		return 1
	}

	[ -n "$delay" ] && sleep "$delay"

	if [ "$vid" == "1BC7" ] && [ "$pid" == "0036" ]; then
			#pataisymas del kabuciu ivedino i password
		gsmctl -A 'AT#SETHEXSTR=2'
	fi

	gsmctl -n -A "AT+CGDCONT=$json_cid,\"$pdptype\",\"$apn\""

	if [ -z "$auth_mode" -o "$auth_mode" == "none" ]; then
		auth_mode="0"
	elif [ "$auth_mode" == "pap" ]; then
		auth_mode="1"
	elif [ "$auth_mode" == "chap" ]; then
		auth_mode="2"
	fi

	gsmctl -A "AT#PDPAUTH=$json_cid,$auth_mode,\"$username\",\"$password\""

	gsmctl -A 'AT+CGACT=1,1'
	sleep 2

	echo "Connecting modem"
	if [ "$vid" == "1BC7" ] && [ "$pid" == "0036" ]; then
		gsmctl -A "AT#NCM=1,$json_cid"
# 		echo -en "AT+CGDATA=\"M-RAW_IP\",$json_cid\r\n")
		ret=$(gsmctl -n -A "AT+CGDATA=\"M-RAW_IP\",$json_cid")
	else
# 		echo -ne "AT#NCM=2,$json_cid\r\n")
		ret=$(gsmctl -n -A "AT#NCM=2,$json_cid")
	fi

	debug "${interface}_4: Connection state: $ret"

	IP=$(gsmctl -n -A AT+CGPADDR=1 | awk -F '"' '{print $2}')
	debug "${interface}_4: selected IP: $IP"

	if [ "$IP" = "" ]; then
		gsmctl -n -A "AT+CGDCONT=$json_cid"
		return 1
	fi

	CGCONTRDP=$(gsmctl -n -A AT+CGCONTRDP="$json_cid")
	if [ "$CGCONTRDP" = "" ]; then
		gsmctl -n -A "AT+CGDCONT=$json_cid"
		return 1
	fi

	GW=$(echo $CGCONTRDP | awk -F '"' '{print $6}')
	NETMASK=$(echo $CGCONTRDP | awk -F '"' '{print $4}' | awk -F '.' '{print $5"."$6"."$7"."$8}')
	DNS=$(echo $CGCONTRDP | awk -F '"' '{print $8}')
	DNS2=$(echo $CGCONTRDP | awk -F '"' '{print $10}')

	debug "${interface}_4: selected GW: $GW"
	debug "${interface}_4: selected NETMASK: $NETMASK"
	debug "${interface}_4: selected DNS: $DNS"
	debug "${interface}_4: selected DNS2: $DNS2"

	echo "Setting up $ifname"
	proto_init_update "$ifname" 1
	proto_set_keep 1
	proto_send_update "${interface}"

	echo "Setting up ${interface}_4 interface"
	local zone="$(fw3 -q network "$interface" 2>/dev/null)"

	json_init
	json_add_string name "${interface}_4"
	json_add_string ifname "@$interface"
	json_add_string proto "static"
	json_add_string gateway "$GW"

 	json_add_array ipaddr
		json_add_string "" "$IP/24"
 	json_close_array

 	json_add_array dns
		json_add_string "" "$DNS"
 	json_close_array

	[ -n "$zone" ] && {
		json_add_string zone "$zone"
	}

	ubus call network add_dynamic "$(json_dump)"
}

proto_ncm_teardown() {
	local interface="$1"

	local device profile
	json_get_vars device profile

	[ -n "$ctl_device" ] && device=$ctl_device

	[ -n "$profile" ] || profile=1

	echo "Stopping network $interface"

	logger "Deattaching"
	#disable ncm proto
	gsmctl -n -A "AT#NCMD=0"
	#turn off autoattch
	gsmctl -n -A "AT#AUTOATT=0"
	sleep 1
	gsmctl -n -A "AT+CGATT=0"
	sleep 1

	#deactivate context
	gsmctl -n -A "AT+CGACT=0,1"
	sleep 1


	logger "Clearing context"
	#clearing context
	gsmctl -n -A 'AT+CGDCONT=1'
	gsmctl -n -A 'AT+CGDCONT=3'
	gsmctl -n -A 'AT+CGDCONT=4'

	proto_init_update "*" 0
	proto_send_update "$interface"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol ncm
}
