#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/mobile.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_ncm_init_config() {
	available=1
	no_device=1
	disable_auto_up=1
	teardown_on_l3_link_down=1

	proto_config_add_boolean dhcp
	proto_config_add_boolean dhcpv6
	proto_config_add_int ip4table
	proto_config_add_int ip6table

	#teltonika specific
	proto_config_add_string modem
	proto_config_add_string pdp
	proto_config_add_string pdptype
	proto_config_add_string sim
	proto_config_add_string method
	proto_config_add_int mtu
	proto_config_add_string delay
	proto_config_add_defaults
}

get_next_ip () {
	local ip="$1" ip_hex next_ip next_ip_hex
	ip_hex=$(printf '%.2X%.2X%.2X%.2X\n' `echo ${ip} | sed -e 's/\./ /g'`)
	next_ip_hex=$(printf %.8X `echo $(( 0x${ip_hex} + 1 ))`)
	next_ip=$(printf '%d.%d.%d.%d\n' `echo ${next_ip_hex} | sed -r 's/(..)/0x\1 /g'`)
	echo "$next_ip"
}

get_netmask_gateway() {
	local ip="$1" ip_net="$1" ip_broad="$1" mask=30 gateway_net same_net=0
	gateway="$ip_broad"

	while [ "$ip_net" = "$ip" -o "$ip_broad" = "$gateway" -a "$same_net" -eq 1 ]; do
		eval "$(ipcalc.sh "$ip/$mask")"
		ip_net="$NETWORK"
		netmask="$NETMASK"
		ip_broad="$BROADCAST"
		gateway="$(get_next_ip ${ip})" # Get gateway IP address
		eval "$(ipcalc.sh "$gateway/$mask")" # Get gateway network IP address
		gateway_net="$NETWORK"
		[ "$gateway_net" = "$ip_net" ] && {
			same_net=1
		}
		let "mask-=1"
	done
}

parse_ipv4_information() {
	local pdp="$1"
	local parsed_ip all_dns primary_dns secondary_dns

	parsed_ip="$(gsmctl -A AT+CGPADDR=${pdp} 2>/dev/null)"
	[ "${parsed_ip::8}" != "+CGPADDR" ] && {
		echo "Can't parse IP address!"
		return 1
	}
	parsed_ip="$(echo ${parsed_ip} | cut -d'"' -f2)"
	[ "${#parsed_ip}" -gt 15 ] && {
		echo "Unable parse IPv4 because using IPv6!"
		return 1
	}

	all_dns="$(gsmctl -A AT+QIDNSCFG=${pdp} 2>/dev/null)"
	[ "${all_dns::9}" != "+QIDNSCFG" ] && {
		echo "Can't get primary and secondary DNS!"
		return 1
	}
	primary_dns="$(echo ${all_dns} | cut -d'"' -f2)"
	secondary_dns="$(echo ${all_dns} | cut -d'"' -f4)"

	get_netmask_gateway "$parsed_ip"

	json_init
	json_add_object "ipv4"
	json_add_string ip "$parsed_ip"
	json_add_string dns1 "$primary_dns"
	json_add_string dns2 "$secondary_dns"
	json_add_string gateway "$gateway"
	json_add_string subnet "$netmask"
	json_close_object

	parameters4="$(json_dump)"
	return 0
}

failure_notify() {
	local pdptype="$1"
	case "$pdptype" in
		ip)
			proto_notify_error "$interface" FAILED_IPV4
			exit 1
			;;
		ipv6)
			proto_notify_error "$interface" FAILED_IPV6
			exit 1
			;;
		ipv4v6)
			proto_notify_error "$interface" FAILED_IPV4V6
			exit 1
			;;
	esac
}

proto_ncm_setup() {
	local interface="$1"
	local mtu method device pdp modem pdptype sim dhcp dhcpv6 $PROTO_DEFAULT_OPTIONS IFACE4 IFACE6 delay ip4table ip6table
	local timeout=2 retries=0

	json_get_vars mtu method device pdp modem pdptype sim dhcp dhcpv6 delay ip4table ip6table $PROTO_DEFAULT_OPTIONS

	check_modem_status "$interface" "$modem" || {
		ubus call network.interface down "{\"interface\":\"${interface}\"}"
		sleep 1
		return
	}

	[ -n "$delay" ] || delay=5
	sleep "$delay"

	echo "Connection setup for ${interface} starting!"

	echo "Checking if SIM card is ready"

	#Check if SIM is ready
	while [ "$(gsmctl -u 2>/dev/null)" != "READY" ]
	do
		retries=$((retries+1))
		[ "$retries" -eq "3" ] && {
			echo "SIM is not inserted!"
			proto_block_restart "$interface"
			return
		}
		sleep $timeout
	done

	[ -z "$metric" ] && metric="1"
	[ -z "$pdp" ] && pdp="1"

        #~ Find interface name
        devname="$(basename "$device")"
        ifname="$(ls /sys/bus/usb/devices/$devname/*/net/)"

	[ -n "$ifname" ] || {
		echo "The interface could not be found."
		proto_notify_error "$interface" NO_IFACE
		return
	}

	pdptype="$(echo "$pdptype" | awk '{print tolower($0)}')"
	[ "$pdptype" = "ip" -o "$pdptype" = "ipv6" -o "$pdptype" = "ipv4v6" ] || pdptype="ip"

	echo "Starting activate PDP context!"
	local pdp_ctx="$(gsmctl -A AT+QIACT=${pdp} 2>/dev/null)"
	[ "${pdp_ctx::2}" != "OK" ] && {
		echo "Can't activate PDP context! Error: ${pdp_ctx}"
		failure_notify "$pdptype"
	}

	echo "Starting setup data call!"
	local pdp_act="$(gsmctl -A AT+QNETDEVCTL=1,${pdp},1 2>/dev/null)"
	[ "${pdp_act::2}" != "OK" ] && {
		echo "Data call failed! Error: ${pdp_act}"
		failure_notify "$pdptype"
	}

	[ -z "$mtu" ] || {
		echo "Setting MTU: ${mtu} on ${ifname}"
		ip link set mtu "$mtu" "$ifname"
	}

	proto_init_update "$ifname" 1
	proto_set_keep 1
	proto_add_data
	json_add_string "pdp" "$pdp"
	json_add_string "method" "$method"
	proto_close_data
	proto_send_update "$interface"

	local zone="$(fw3 -q network "$interface" 2>/dev/null)"

	[ "$method" = "bridge" -o "$method" = "passthrough" ] && \
	[ "$pdptype" = "ip" -o "$pdptype" = "ipv4v6" ] && {

		parse_ipv4_information "$pdp" && {
			setup_bridge_v4 "$ifname"

			#Passthrough
			[ "$method" = "passthrough" ] && {
				iptables -tnat -I postrouting_rule -o "$ifname" -j SNAT --to "$bridge_ipaddr"
				ip route add default dev "$ifname"
			}
		}
	}

	[ "$method" != "bridge" ] && [ "$method" != "passthrough" ] && \
	[ "$pdptype" = "ip" -o "$pdptype" = "ipv4v6" ] && {
		if [ "$dhcp" = 0 ]; then
			setup_static_v4 "$ifname"
		else
			setup_dhcp_v4 "$ifname"
		fi

		json_init
		json_add_string modem "$modem"
		json_add_string sim "$sim"
		[ -n "$zone" ] && json_add_string zone "$zone"

		ubus call network.interface."${interface}_4" set_data "$(json_dump)" 2>/dev/null
		IFACE4="${interface}_4"
	}

	[ "$pdptype" = "ipv6" -o "$pdptype" = "ipv4v6" ] && {
		if [ "$dhcpv6" = 0 ]; then
			setup_static_v6 "$ifname"
		else
			setup_dhcp_v6 "$ifname"
		fi

		json_init
		json_add_string modem "$modem"
		json_add_string sim "$sim"
		[ -n "$zone" ] && json_add_string zone "$zone"

		ubus call network.interface."${interface}_6" set_data "$(json_dump)" 2>/dev/null
		IFACE6="${interface}_6"
	}

	#Run udhcpc to obtain lease
	proto_export "IFACE4=$IFACE4"
	proto_export "IFACE6=$IFACE6"

	proto_run_command "$interface" ncm_conn.sh "$ifname" "$pdp"
}

proto_ncm_teardown() {
	local interface="$1" pdp bridge_ipaddr method
	json_get_vars pdp

	echo "Stopping network ${interface}"

	json_load "$(ubus call network.interface.$interface status 2>/dev/null)"
	json_select data
	json_get_vars method bridge_ipaddr

	#Kill udhcpc instance
	proto_kill_command "$interface"

	ubus call network.interface down "{\"interface\":\"${interface}_4\"}" 2>/dev/null
	ubus call network.interface down "{\"interface\":\"${interface}_6\"}" 2>/dev/null

	#Stop data call
	gsmctl -A AT+QNETDEVCTL=0,${pdp},1 &>/dev/null

	#Deactivate context
	gsmctl -A AT+QIDEACT=${pdp} &>/dev/null
	kill -9 $(cat /var/run/ncm_conn.pid 2>/dev/null) &>/dev/null
	rm -f /var/run/ncm_conn.pid &>/dev/null

	[ "$method" = "bridge" ] || [ "$method" = "passthrough" ] && {
		ip rule del pref 5042
		ip rule del pref 5043
		ip route flush table 42
		ip route flush table 43
		ip route del "$bridge_ipaddr"
		swconfig dev switch0 set soft_reset 5 &
	}

	#Clear passthrough and bridge params
	iptables -t nat -F postrouting_rule
	rm -f "/tmp/dnsmasq.d/bridge"
	ip neigh flush proxy

	proto_init_update "*" 0
	proto_send_update "$interface"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol ncm
}
