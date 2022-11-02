#Mobile configuration management lib

. /usr/share/libubox/jshn.sh
. /lib/functions.sh

check_pdp_context() {
	local pdp="$1"
	local modem_id="$2"

	cid=$(gsmctl -O "$modem_id" -A AT+CGDCONT? | grep "+CGDCONT: $pdp")

	if [ -z "$cid" ]; then
		echo "Creating context with PDP: $pdp"
		gsmctl -O "$modem_id" -A AT+CGDCONT=$pdp,\"ip\"
		gsmctl -O "$modem_id" -A AT+CFUN=4
		sleep 2
		gsmctl -O "$modem_id" -A AT+CFUN=1
	fi
}

check_modem_status() {
	local interface="$1"
	local modem_id="$2"
	logger "Looking /tmp/mobileon_$modem_id flag"

	if [ ! -f "/tmp/mobileon_$modem_id" ]; then
		logger "Modem not ready for work!"
		return 1
	fi
	logger "Modem is ready!"
}

gsm_soft_reset() {
	gsmctl -n -A at+cfun=4
	sleep 2
	gsmctl -n -A at+cfun=1
}

qmi_error_handle() {
	local error="$1"
	local modem_id="$2"

	$(echo "$error" | grep -q "error") && {
		echo "$error"
	}

	$(echo "$error" | grep -q "Client IDs exhausted") && {
			echo "ClientIdsExhausted! reseting counter..."
			proto_notify_error "$interface" NO_CID
			uqmi -s -d "$device" --sync
			return 1
	}

#	Reik papildyt ERROR handlinima
#	$(echo "$error" | grep -q "multiple-connection-to-same-pdn-not-allowed") && {
#			echo "Reseting due dublicated connection..."
#			qmicli -p -d "$device" --uim-sim-power-off=1
#			qmicli -p -d "$device" --uim-sim-power-on=1
#			return 1
#	}

#	$(echo "$error" | grep -q "Transaction timed out") && {
#			echo "Device not responding, restarting module"
#			gsmctl -O $modem_id -A at+cfun=1,1
#	}
#
#	$(echo "$error" | grep -q 'verbose call end reason (2,236)') && {
   #                     echo "Failed to start network, clearing all cids"
  #                      qmicli -p -d "$device" --wds-noop --device-open-sync
 #                       return 1
#        }

	$(echo "$error" | grep -q "Call Failed") && {
		echo "Device not responding, restarting module"
		sleep 10
		gsm_soft_reset
		return 1
	}

	$(echo "$error" | grep -q "Policy Mismatch") && {
		echo "Reseting network..."
		gsm_soft_reset
		return 1
	}

	$(echo "$error" | grep -q "Failed to connect to service") && {
		echo "Device not responding, restarting module"
		gsmctl -A at+cfun=1,1
		return 1
	}

	$(echo "$error" | grep -q "error") && {
		echo "$error"
	}

	return 0
}

first_uqmi_call()
{
	local cmd="$1"

	local count="0"
	local max_try="3"
	local timeout="2"
	local ret

	while [ 1 ]; do
		ret=$($cmd)
		if [ "$ret" = "\"Failed to connect to service\"" ]; then
			count=$((count+1))
			sleep $timeout
		else
			break
		fi
		if [ "$count" = "$max_try" ]; then
			qmi_error_handle "$ret" "$modem" || return 1
		fi
	done
}

sim1_pass=
sim2_pass=
get_passthrough_interfaces() {
	local sim method
	config_get method "$1" "method"
	config_get sim "$1" "sim"
	[ "$method" = "passthrough" ] && [ "$sim" = "1" ] && sim1_pass="$1"
	[ "$method" = "passthrough" ] && [ "$sim" = "2" ] && sim2_pass="$1"
}

passthrough_mode=
get_passthrough() {
	config_get primary "$1" primary
	[ "$primary" = "1" ] && {
		config_get sim "$1" position;
		passthrough_mode=$(eval uci -q get network.\${sim${sim}_pass}.passthrough_mode 2>/dev/null);
	}
}

setup_bridge_v4() {
	local dev="$1"
	local dhcp_param_file="/tmp/dnsmasq.d/bridge"
	echo "$parameters4"

	[[ "$dev" = "rmnet_data"* ]] && { ## TRB5 uses qmicli - different format
		bridge_ipaddr="$(echo "$parameters4" | sed -n "s_.*IPv4 address: \([0-9.]*\)_\1_p")"
		bridge_mask="$(echo "$parameters4" | sed -n "s_.*IPv4 subnet mask: \([0-9.]*\)_\1_p")"
		bridge_gateway="$(echo "$parameters4" | sed -n "s_.*IPv4 gateway address: \([0-9.]*\)_\1_p")"
		bridge_dns1="$(echo "$parameters4" | sed -n "s_.*IPv4 primary DNS: \([0-9.]*\)_\1_p")"
		bridge_dns2="$(echo "$parameters4" | sed -n "s_.*IPv4 secondary DNS: \([0-9.]*\)_\1_p")"
	} || {
		json_load "$parameters4"
		json_select "ipv4"
		json_get_var bridge_ipaddr ip
		json_get_var bridge_mask subnet
		json_get_var bridge_gateway gateway
		json_get_var bridge_dns1 dns1
		json_get_var bridge_dns2 dns2
	}

	json_init
	json_add_string name "${interface}_4"
	json_add_string ifname "$dev"
	json_add_string proto "none"
	json_add_object "data"
	ubus call network add_dynamic "$(json_dump)"
	IFACE4="${interface}_4"

	json_init
	json_add_string interface "${interface}_4"
	json_add_string zone "lan"
	ubus call network.interface set_data "$(json_dump)"

	json_init
	json_add_string interface "${interface}"
	json_add_string bridge_ipaddr "$bridge_ipaddr"
	ubus call network.interface set_data "$(json_dump)"

	json_init
	json_add_string modem "$modem"
	json_add_string sim "$sim"
	ubus call network.interface."${interface}_4" set_data "$(json_dump)"
	json_close_object

	ip route add default dev "$dev" table 42
	ip route add default dev br-lan table 43
	ip route add "$bridge_ipaddr" dev br-lan

	ip rule add pref 5042 from "$bridge_ipaddr" lookup 42
	ip rule add pref 5043 iif "$dev" lookup 43
	#sysctl -w net.ipv4.conf.br-lan.proxy_arp=1 #2>/dev/null
	ip neighbor add proxy "$bridge_gateway" dev br-lan
	ip neighbor add proxy "$bridge_ipaddr" dev "$dev" 2>/dev/null

        iptables -A postrouting_rule -m comment --comment "Bridge mode" -o "$dev" -j ACCEPT -tnat

	config_load network
	config_foreach get_passthrough_interfaces interface

	config_load simcard
	config_foreach get_passthrough sim

	> $dhcp_param_file
	[ -z "$mac" ] && mac="*:*:*:*:*:*"
	[ "$passthrough_mode" != "no_dhcp" ] && {
		echo "dhcp-range=tag:mobbridge,$bridge_ipaddr,static,$bridge_mask,${leasetime:-1h}" > "$dhcp_param_file"
		echo "shared-network=br-lan,$bridge_ipaddr" >> "$dhcp_param_file"
		echo "dhcp-host=$mac,set:mobbridge,$bridge_ipaddr" >> "$dhcp_param_file"
		echo "dhcp-option=tag:mobbridge,br-lan,3,$bridge_gateway" >> "$dhcp_param_file"
		echo "dhcp-option=tag:mobbridge,br-lan,6,$bridge_dns1${bridge_dns2:+,$bridge_dns2}" >> "$dhcp_param_file"
		echo "server=$bridge_dns1" >> "$dhcp_param_file"
		echo "server=$bridge_dns2" >> "$dhcp_param_file"
	}
	[ "$passthrough_mode" = "no_dhcp" ] && {
		echo "server=$bridge_dns1" >> "$dhcp_param_file"
		echo "server=$bridge_dns2" >> "$dhcp_param_file"
	}

	/etc/init.d/dnsmasq reload
	swconfig dev 'switch0' set soft_reset 5 &
}

setup_static_v4() {
	local dev="$1"
	echo "Setting up $dev V4 static"
	echo "$parameters4"

	[[ "$dev" = "rmnet_data"* ]] && { ## TRB5 uses qmicli - different format
		ip_4="$(echo "$parameters4" | sed -n "s_.*IPv4 address: \([0-9.]*\)_\1_p")"
		dns1_4="$(echo "$parameters4" | sed -n "s_.*IPv4 primary DNS: \([0-9.]*\)_\1_p")"
		dns2_4="$(echo "$parameters4" | sed -n "s_.*IPv4 secondary DNS: \([0-9.]*\)_\1_p")"
	} || {
		json_load "$parameters4"
		json_select "ipv4"
		json_get_var ip_4 ip
		json_get_var dns1_4 dns1
		json_get_var dns2_4 dns2
	}

	json_init
	json_add_string name "${interface}_4"
	json_add_string ifname "$dev"
	json_add_string proto static
	json_add_string gateway "0.0.0.0"

	json_add_array ipaddr
		json_add_string "" "$ip_4"
	json_close_array

	json_add_array dns
		[ -n "$dns1_4" ] && json_add_string "" "$dns1_4"
		[ -n "$dns2_4" ] && json_add_string "" "$dns2_4"
	json_close_array

	[ -n "$ip4table" ] && json_add_string ip4table "$ip4table"
	proto_add_dynamic_defaults

	ubus call network add_dynamic "$(json_dump)"
}

setup_dhcp_v4() {
	local dev="$1"
	echo "Setting up $dev V4 DCHP"
	json_init
	json_add_string name "${interface}_4"
	json_add_string ifname "$dev"
	json_add_string proto "dhcp"
	json_add_string script "/lib/netifd/dhcp_mobile.script"
	[ -n "$ip4table" ] && json_add_string ip4table "$ip4table"
	proto_add_dynamic_defaults
	[ -n "$zone" ] && json_add_string zone "$zone"
	ubus call network add_dynamic "$(json_dump)"
}

setup_dhcp_v6() {
	local dev="$1"
	echo "Setting up $dev V6 DHCP"
	json_init
	json_add_string name "${interface}_6"
	json_add_string ifname "$dev"
	json_add_string proto "dhcpv6"
	[ -n "$ip6table" ] && json_add_string ip6table "$ip6table"
	json_add_boolean ignore_valid 1
	proto_add_dynamic_defaults
	# RFC 7278: Extend an IPv6 /64 Prefix to LAN
	json_add_string extendprefix 1
	[ -n "$zone" ] && json_add_string zone "$zone"
	ubus call network add_dynamic "$(json_dump)"
}

setup_static_v6() {
	local dev="$1"
	echo "Setting up $dev V6 static"
	echo "$parameters6"

	[[ "$dev" = "rmnet_data"* ]] && { ## TRB5 uses qmicli - different format
		ip6_with_prefix="$(echo "$parameters6" | sed -n "s_.*IPv6 address: \([0-9a-f:]*\)_\1_p")"
		ip_6="${ip6_with_prefix%/*}"
		ip_pre_len="${ip6_with_prefix#*/}"
		ip6_gateway_with_prefix="$(echo "$parameters6" | sed -n "s_.*IPv6 gateway address: \([0-9a-f:]*\)_\1_p")"
		gateway_6="${ip6_gateway_with_prefix%/*}"
		dns1_6="$(echo "$parameters6" | sed -n "s_.*IPv6 primary DNS: \([0-9a-f:]*\)_\1_p")"
		dns2_6="$(echo "$parameters6" | sed -n "s_.*IPv6 secondary DNS: \([0-9a-f:]*\)_\1_p")"

	} || {
		json_load "$parameters6"
		json_select "ipv6"
		json_get_var ip6_with_prefix ip
		ip_6="${ip6_with_prefix%/*}"
		ip_prefix_length="${ip6_with_prefix#*/}"
		json_get_var ip6_gateway_with_prefix gateway
		gateway_6="${ip6_gateway_with_prefix%/*}"
		json_get_var dns1_6 dns1
		json_get_var dns2_6 dns2
		json_get_var ip_pre_len ip-prefix-length
	}

	json_init
	json_add_string name "${interface}_6"
	json_add_string ifname "$dev"
	json_add_string proto static
	json_add_string ip6gw "$gateway_6"

	json_add_array ip6prefix
		json_add_string "" "$ip6_with_prefix/$ip_pre_len"
	json_close_array

	json_add_array ip6addr
		json_add_string "" "$ip6_with_prefix/$ip_pre_len"
	json_close_array

	json_add_array dns
		[ -n "$dns1_6" ] && json_add_string "" "$dns1_6"
		[ -n "$dns2_6" ] && json_add_string "" "$dns2_6"
	json_close_array

	[ -n "$ip6table" ] && json_add_string ip6table "$ip6table"
	proto_add_dynamic_defaults

	ubus call network add_dynamic "$(json_dump)"
}

uqmi_modify_data_format() {
	QMI_PROTOCOL="$1"
	if [ "$QMI_PROTOCOL" = "qmi" ]; then
		command="uqmi -d "${device}" $options --wda-set-data-format --link-layer-protocol raw-ip \
--ul-protocol disabled --dl-protocol disabled"
	elif [ $dl_max_size -gt 16384 ]; then
		command="uqmi -d "$device" $options --wda-set-data-format --qos-format 0 \
--link-layer-protocol raw-ip --endpoint-type hsusb --endpoint-iface-number $ep_iface
--ul-protocol qmapv5 --dl-protocol qmapv5 --ul-max-datagrams 11 --ul-datagram-max-size 8192 \
--dl-max-datagrams $dl_max_datagrams --dl-datagram-max-size $dl_max_size --dl-min-padding 0"
	else
		command="uqmi -d "${device}" $options --wda-set-data-format --link-layer-protocol raw-ip \
--ul-protocol qmap --dl-protocol qmap --dl-max-datagrams $dl_max_datagrams \
--dl-datagram-max-size $dl_max_size"
	fi

	logger "$command"
	ret=$(eval $command)
	qmi_error_handle "$ret" "$modem" || return 1
}

call_uqmi_command() {
	command="$1"
	logger -t "netifd" "$command"
	ret=$(eval $command)
	[ "$ret" != "" ] && echo "$ret"
	qmi_error_handle "$ret" "$modem" || return 1
}

check_digits() {
	var="$1"
	echo "$var" | grep -E '^[+-]?[0-9]+$'
}

ubus_set_interface_data() {
	local modem sim zone iface_and_type
	modem="$1"
	sim="$2"
	zone="$3"
	iface_and_type="$4"

	json_init
	json_add_string modem "$modem"
	json_add_string sim "$sim"
	[ -n "$zone" ] && json_add_string zone "$zone"

	ubus call network.interface."${iface_and_type}" set_data "$(json_dump)"
}

clear_connection_values() {
	local interface device iface_and_type conn_proto
	interface="$1"
	device="$2"
	iface_and_type="$3"
	conn_proto="$4"

	cid="$(cat "/var/run/${conn_proto}/${interface}.cid_${iface_and_type}" 2>/dev/null)"

	[ -n "$cid" ] && {
		echo "Stopping network on ${device}!"
		call_uqmi_command "uqmi -s -d "${device}" -t 3000 --set-client-id wds,"${cid}" \
--stop-network 0xFFFFFFFF --autoconnect"
		echo "Releasing client-id ${cid} on ${device}"
		call_uqmi_command "uqmi -s -d "${device}" -t 3000 --set-client-id wds,"${cid}" \
--release-client-id wds"

		rm -f "/var/run/${conn_proto}/${interface}.cid_${iface_and_type}"
	}
}
