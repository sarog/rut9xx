#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"

}
CID_FILE="/tmp/qmi_cid_file"

proto_qmi_init_config() {
	available=1
	no_device=1
	proto_config_add_string "device:device"
	proto_config_add_string apn
	proto_config_add_string auth_mode
	proto_config_add_string username
	proto_config_add_string password
	proto_config_add_string delay
	proto_config_add_string modes
	proto_config_add_string method
	proto_config_add_string enabled
	proto_config_add_string "backup"
	proto_config_add_string roaming
}

qmi_disconnect() {
	# disable previous autoconnect state using the global handle
	# do not reuse previous wds client id to prevent hangs caused by stale data
	uqmi -s -d "$device" \
		--stop-network 0xffffffff \
		--autoconnect > /dev/null
	killall "uqmi" 2>/dev/null
}

qmi_wds_release() {
	[ -n "$cid" ] || return 0
	uqmi -s -d "$device" --set-client-id wds,"$cid" --release-client-id wds
	uci_revert_state network $interface cid
}

_proto_qmi_setup() {
	json_get_var enabled enabled
	local roaming_cap model
	local interface="$1"
	if [ "$enabled" != "1" ] && [ ! -f "/tmp/mobileon" ]; then
		ifdown ppp
		return 0
	fi
	local device apn auth_mode username password delay modes cid pdh method roaming
	json_get_vars device apn auth_mode username password delay modes method roaming
	[ -n "$ctl_device" ] && device=$ctl_device
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
	[ -n "$delay" ] && sleep "$delay"
	while uqmi -s -d "$device" --get-pin-status | grep '"UIM uninitialized"' > /dev/null; do
		sleep 1;
	done
	#[ -n "$pincode" ] && {
	#	uqmi -s -d "$device" --verify-pin1 "$pincode" || {
	#		echo "Unable to verify PIN"
	#		proto_notify_error "$interface" PIN_FAILED
	#		proto_block_restart "$interface"
	#		return 1
	#	}
	#}

	#[ -n "$apn" ] || {
	#	echo "No APN specified"
	#	proto_notify_error "$interface" NO_APN
	#	return 1
	#}
	conn_status=`uqmi -s -d "$device" --get-data-status`
	if [ "$conn_status" == "\"connected\"" ]; then
			echo "connected, disconnecting..."
			qmi_disconnect
	fi
	uqmi -s -d "$device" --set-data-format 802.3
	uqmi -s -d "$device" --wda-set-data-format 802.3

	#check if roaming capability is enabled in EC20 module, if not - enable roaming capability
	model=`gsmctl -m`
	if [ -n "$model" ] && [ "$model" == "EC20" ]; then
		roaming_cap=`gsmctl -A 'AT+QCFG="roamservice"'`
		if [ "$roaming_cap" != "+QCFG: \"roamservice\",255" ]; then
			gsmctl -A 'AT+QCFG="roamservice",255,1'
		fi
		sleep 1
	fi

	echo "Waiting for network registration"
	while uqmi -s -d "$device" --get-serving-system | grep '"searching"' > /dev/null; do
		sleep 5;
	done
	serving_system=`uqmi -s -d "$device" --get-serving-system | grep -c "\"registered\""` # check if registration is successful
	if [ "$serving_system" != "1" ]; then
			echo "Not registered, trying again"
			return 1
	fi
	serving_system=`uqmi -s -d "$device" --get-serving-system | grep -c "\"roaming\":true"` # check if roaming && roaming enabled
	if [ "$serving_system" == "1" -a "$roaming" == "1" ]; then
		echo "Roaming detected"
		return 1
	fi

	[ -n "$modes" ] && uqmi -s -d "$device" --set-network-modes "$modes"

	echo "Starting network $apn"
	cid=`uqmi -s -d "$device" --get-client-id wds`
	[ $? -ne 0 ] && {
		echo "Unable to obtain client ID"
		proto_notify_error "$interface" NO_CID
		return 1
	}
	echo "$cid" > "$CID_FILE"
	#workaround for APN
	gsmctl -A AT+CGDCONT=1,'"IP"',\"$apn\"
	#
	sleep 1

	#sleep 1
	killall "uqmi" 2>/dev/null
	uqmi -s -d "$device" --set-client-id wds,"$cid" \
		--start-network "$apn" \
		${auth_mode:+--auth-type $auth_mode} \
		${username:+--username $username} \
		${password:+--password $password} \
		--autoconnect > /dev/null

	if [ "$method" != "bridge" ]; then
		echo "Starting DHCP"
		proto_init_update "$ifname" 1
		proto_send_update "$interface"

		json_init
		json_add_string name "${interface}_dhcp"
		json_add_string ifname "@$interface"
		json_add_string proto "dhcp"
		json_close_object
		ubus call network add_dynamic "$(json_dump)"
		# IGNORE ERRORS
		return 0

		#json_init
		#json_add_string name "${interface}_dhcpv6"
		#json_add_string ifname "@$interface"
		#json_add_string proto "dhcpv6"
		#json_close_object
		#ubus call network add_dynamic "$(json_dump)"
	else
		echo "Bridge mode"
		proto_run_command "$1" /sbin/qmi_watch.sh
	fi
}

proto_qmi_setup() {
	local ret=1
	while [ $ret -ne 0 ]
	do
		_proto_qmi_setup $@
		ret=$?
		[ "$ret" = 0 ] || {
			logger "qmi bringup failed, retry in 2s"
			sleep 2
		}
	done
	return $ret
}

proto_qmi_teardown() {
	local interface="$1"
	local device
	json_get_vars device

	[ -n "$ctl_device" ] && device=$ctl_device

	local cid=$(uci_get_state network $interface cid)
	[ -z "$cid" ] && {
		cid="$(cat "$CID_FILE" 2>/dev/null)"
		rm -f "$CID_FILE"
	}

	echo "Stopping network"
	qmi_disconnect
	qmi_wds_release

	proto_init_update "*" 0
	proto_send_update "$interface"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol qmi
}
