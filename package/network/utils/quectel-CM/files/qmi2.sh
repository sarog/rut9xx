#!/bin/sh
[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

#let netifd know which parameters does this protocol have. Later params are parsed with json_get_var/json_get_vars
#no_device=1 is used because carrier and operstate is not working for EC25 wwan0 iface,
#if no_device=0, netifd won't even try to start our profile until the device is already up. It waits for operstate=up and carrier=1, then starts the profile.
proto_qmi2_init_config() {
	proto_config_add_string "device:device"
	proto_config_add_string apn
	proto_config_add_string auth_mode
	proto_config_add_string username
	proto_config_add_string password
	proto_config_add_string method
	proto_config_add_string enabled
	proto_config_add_string roaming
	proto_config_add_string pdptype
	proto_config_add_string mtu
	proto_config_add_string cid
	proto_config_add_defaults
	no_device=1
	#if available=0, after every 'proto_set_available "$interface" 0'(swithcing between ppp/qmi or wan, unsuccessful connection attempt), there should be call 'proto_set_available "$interface" 1'.
	#Available=1 automatically sets 'proto_set_available "$interface" 1' if proto is changed in network.ppp.proto
	available=1
}

#Set up connection.
proto_qmi2_setup() {

	local interface="$1"
	local br=0
	local apn device username password auth_mode roaming method pdptype mtu cid ip4table $PROTO_DEFAULT_OPTIONS
	json_get_vars	apn device username password auth_mode roaming method pdptype mtu cid ip4table $PROTO_DEFAULT_OPTIONS

	echo "setup: $1"

	#Checking if interface is enabled and if not disabling it
	json_get_var enabled enabled
	if [ "$enabled" != "1" ] && [ ! -f "/tmp/mobileon" ]; then
		ifdown ppp
		return 0
	fi

	if [ "$(gsmctl -z)" != "inserted" ]; then
		ifdown ppp
		return 0
	fi

	#Checking modem existance and operation
	[ -n "$device" ] || {
		echo "No control device specified"
		proto_notify_error "$interface" NO_DEVICE
		proto_set_available "$interface" 0
		return 1
	}

	[ -c "$device" ] || {
		echo "The specified control device does not exist"
		proto_notify_error "$interface" NO_DEVICE
		proto_set_available "$interface" 0
		return 1
	}

	devname="$(basename "$device")"
	devpath="$(readlink -f /sys/class/usbmisc/$devname/device/)"
	ifname="$( ls "$devpath"/net )"
	[ -n "$ifname" ] || {
		echo "The interface could not be found."
		proto_notify_error "$interface" NO_IFACE
		proto_set_available "$interface" 0
		return 1
	}

	if [ -z "$roaming" -o "$roaming" = "0" ]; then
		gsmctl -A 'AT+QCFG="roamservice",2,1'
	else
		gsmctl -A 'AT+QCFG="roamservice",1,1'
	fi

	# take values form config again, after SIM is ready due to Auto-APN
	apn=$(uci -q get network.$interface.apn)
	auth=$(uci -q get network.$interface.auth)
	if [ "$auth" != "" -a "$auth" != "none" ]; then
		username=$(uci -q get network.$interface.username)
		password=$(uci -q get network.$interface.password)
	fi

	#pdptype values: (IP,IPV4V6,IPV6)
	if [ "$pdptype" = "" -o "$pdptype" = "1" -o "$pdptype" = "ip" ]; then
		pdptype="IP"
		iptype="1"
	else
		pdptype="IPV6"
		iptype="2"
	fi

	#auth_mode values: (0 - none; 1 - pap; 2 - chap)
	if [ -z "$auth_mode" -o "$auth_mode" = "none" ]; then
		auth_mode="0"
	elif [ "$auth_mode" = "pap" ]; then
		auth_mode="1"
	elif [ "$auth_mode" = "chap" ]; then
		auth_mode="2"
	fi

# 	cid=$(uci get -q network.ppp.cid)
	[ -z "$cid" ] && cid="1"

	# setting authorization before running connection manager
	# because of some operator on LTE network
	# Need first send authorized then connect to network -.-
	gsmctl -A "AT+QICSGP=$cid,1,\"$apn\",\"$username\",\"$password\",$auth_mode"
	sleep 1
	proto_run_command "$1" /usr/sbin/quectel-CM -n $cid -s "$apn" "$username" "$password" "$auth_mode" ${interface:+-c $interface}
}

proto_qmi2_teardown() {
	local interface="$1"
	echo "teardown: $1"
	#Stop connection manager. If proto_run_command is used, quectel-CM is killed automatically, when ifdown or connection failure happens.
	proto_init_update "*" 0
	proto_send_update "$interface"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol qmi2
}
