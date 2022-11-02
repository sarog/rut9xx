#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	. /lib/functions/mobile.sh
	init_proto "$@"
}

proto_connm_init_config() {
	#~ This proto usable only with proto wwan
	available=1
	no_device=1
	disable_auto_up=1
	teardown_on_l3_link_down=1

	proto_config_add_string delay
	proto_config_add_string modes
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
	proto_config_add_string passthrough_mode
	proto_config_add_string leasetime
	proto_config_add_string mac
	proto_config_add_int mtu

	proto_config_add_defaults
}

proto_connm_setup() {
	#~ This proto usable only with proto wwan
	local interface="$1"

	local ipv4 ipv6 delay mtu deny_roaming leasetime mac
	local active_sim="1" options

	local connstat method
	local device apn pdp modem pdptype sim dhcp dhcpv6
	local $PROTO_DEFAULT_OPTIONS IFACE4 IFACE6 parameters4 parameters6 ip4table ip6table
	local pdh_4 pdh_6 cid cid_4 cid_6
	local RETRY_BEFORE_REINT

	json_get_vars apn pdp device modem pdptype sim delay method mtu dhcp dhcpv6 ip4table ip6table leasetime mac $PROTO_DEFAULT_OPTIONS

	check_modem_status "$interface" "$modem" || {
		ubus call network.interface down "{\"interface\":\"${interface}\"}"
		sleep 1
		return
	}

	[ -z "$pdp" ] && pdp="1"

	[ -n "$delay" ] || delay=$(( pdp + 3 ))
	sleep "$delay"

	check_pdp_context "$pdp" "$modem"

#~ Parameters part------------------------------------------------------

	service_name="wds"
	options="--timeout 3000"

	get_simcard_parameters() {
		local section="$1"
		local mdm
		config_get position "$section" position
		config_get mdm "$section" modem

		[ "$modem" = "$mdm" ] && \
		[ "$position" = "$active_sim" ] && {
			config_get deny_roaming "$section" deny_roaming "0"
		}
	}
	config_foreach get_simcard_parameters "sim"

#~ ---------------------------------------------------------------------

	device="/dev/cdc-wdm0"

	[ -n "$ctl_device" ] && device="$ctl_device"
	[ -z "$timeout" ] && timeout="300"
	[ -z "$metric" ] && metric="1"

	#~ Static ifname for trb
	ifname="rmnet0"
	qmimux="rmnet0"
	service_name="wds"
	options="--timeout 30000"

#~ Connectivity part----------------------------------------------------

	pdptype="$(echo "$pdptype" | awk '{print tolower($0)}')"
	[ "$pdptype" = "ip" ] || [ "$pdptype" = "ipv6" ] || [ "$pdptype" = "ipv4v6" ] || pdptype="ip"

	[ "$deny_roaming" -ne "0" ] && deny_roaming="yes" || deny_roaming="no"

	first_uqmi_call "uqmi -d $device --timeout 10000 --set-autoconnect disabled" || return 1

	apn="$(uci -q get network.$interface.apn)"
	echo "Starting network $interface using APN: $apn"

	auth="$(uci -q get network.$interface.auth)"
	[ -n "$auth" ] && [ "$auth" != "none" ] && {
		username="$(uci -q get network.$interface.username)"
		password="$(uci -q get network.$interface.password)"
	}

	cid="$(uqmi -d "$device" $options --get-client-id wds)"
	qmi_error_handle "$cid" "$modem" || return 1

#~ Do not add TABS!
call_uqmi_command "uqmi -d "$device" $options --set-client-id wds,"$cid" --release-client-id wds \
--modify-profile --profile-identifier 3gpp,${pdp} \
--profile-name profile${pdp} \
--roaming-disallowed-flag ${deny_roaming} \
${username:+ --username $username} \
${password:+ --password $password} \
${auth:+ --auth-type $auth} \
${apn:+ --apn $apn} \
${pdptype:+ --pdp-type $pdptype}"

	RETRY_BEFORE_REINT="$(cat /tmp/conn_retry)" 2>/dev/null
	[ -z "$RETRY_BEFORE_REINT" ] && RETRY_BEFORE_REINT="0"

	handle_retry() {
		local retry="$1"
		if [ "$retry" -ge 10 ]; then
			rm /tmp/conn_retry
			ubus call gsmd reinit_modems "{\"id\":\"$modem\"}"
		else
			retry="$((retry + 1))"
			rm /tmp/conn_retry
			echo "$retry" > /tmp/conn_retry
		fi
	}

	[ "$pdptype" = "ip" ] || [ "$pdptype" = "ipv4v6" ] && {

		cid_4=$(call_uqmi_command "uqmi -d "$device" $options --get-client-id wds")
		[ $? -ne 0 ] && return 1

		check_digits $cid_4
		if [ $? -ne 0 ]; then
			echo "Unable to obtain client IPV4 ID"
		fi
		echo "cid4: $cid_4"

		#~ Set ipv4 on CID
		call_uqmi_command "uqmi -d "$device" $options --set-ip-family ipv4 \
--set-client-id wds,\"$cid_4\""
		[ $? -ne 0 ] && return 1

		#~ Start PS call
		pdh_4=$(call_uqmi_command "uqmi -d "$device" $options --set-client-id wds,"$cid_4" \
--start-network ${apn:+--apn $apn} --profile $pdp ${auth:+ --auth-type $auth --username "$username" \
--password "$password"}")
		[ $? -ne 0 ] && return 1
		echo "pdh4: $pdh_4"

		check_digits $pdh_4
		if [ $? -ne 0 ]; then
		# pdh_4 is a numeric value on success
			echo "Unable to connect IPv4"
		else
			# Check data connection state
			connstat="$(uqmi -d $device $options --set-client-id wds,"$cid_4" \
					--get-data-status | awk -F '"' '{print $2}')"
			if [ "$connstat" = "connected" ]; then
				ipv4=1
			else
				echo "No IPV4 data link!"
			fi

			parameters4="$(uqmi -d $device $options --set-client-id wds,"$cid_4" \
					--get-current-settings)"
			[ -z "$mtu" ] && {
				mtu="$(echo "$parameters4" | grep mtu | awk -F ' ' '{print $2}' | \
					awk -F ',' '{print $1}')"
			}
		fi
	}

	[ "$pdptype" = "ipv6" ] || [ "$pdptype" = "ipv4v6" ] && {

		cid_6=$(call_uqmi_command "uqmi -d "$device" $options --get-client-id wds")
		[ $? -ne 0 ] && return 1

		check_digits $cid_6
		if [ $? -ne 0 ]; then
			echo "Unable to obtain client IPV6 ID"
		fi
		echo "cid6: $cid_6"

		#~ Set ipv6 on CID
		ret=$(call_uqmi_command "uqmi -d "$device" $options --set-ip-family ipv6 \
--set-client-id wds,"$cid_6"")
		[ $? -ne 0 ] && return 1

		#~ Start PS call
		pdh_6=$(call_uqmi_command "uqmi -d "$device" $options --set-client-id wds,"$cid_6" \
--start-network ${apn:+--apn $apn} --profile $pdp ${auth:+ --auth-type $auth --username "$username" \
--password "$password"}")
		[ $? -ne 0 ] && return 1
		echo "pdh6: $pdh_6"

		# pdh_6 is a numeric value on success
		check_digits $pdh_6
		if [ $? -ne 0 ]; then
			echo "Unable to connect IPv6"
		else
			# Check data connection state
			connstat="$(uqmi -d $device $options --set-client-id wds,"$cid_6" \
					--get-data-status | awk -F '"' '{print $2}')"
			if [ "$connstat" = "connected" ]; then
				ipv6=1
			else
				echo "No IPV6 data link!"
			fi

			parameters6="$(uqmi -d $device $options --set-client-id wds,"$cid_6" \
					--get-current-settings)"
			[ -z "$mtu" ] && {
				mtu="$(echo "$parameters6" | grep mtu | awk -F ' ' '{print $2}' | \
					awk -F ',' '{print $1}')"
			}
		fi
	}

	fail_timeout=$((delay+10))
	case "$pdptype" in
	ip)
		[ "$ipv4" != "1" ] && {
			echo "Releasing client-id ${cid_4} on ${device}"
			uqmi -d "$device" $options --set-client-id wds,$cid_4 --release-client-id wds
			echo "Failed to create IPV4 connection"
			sleep $fail_timeout
			proto_notify_error "$interface" FAILED_IPV4
			handle_retry "$RETRY_BEFORE_REINT"
			return 1
		}
		;;
	ipv6)
		[ "$ipv6" != "1" ] && {
			echo "Releasing client-id ${cid_6} on ${device}"
			uqmi -d "$device" $options --set-client-id wds,$cid_6 --release-client-id wds
			echo "Failed to create IPV6 connection"
			sleep $fail_timeout
			proto_notify_error "$interface" FAILED_IPV6
			handle_retry "$RETRY_BEFORE_REINT"
			return 1
		}
		;;
	ipv4v6)
		[ "$ipv4" != "1" ] && {
			echo "Releasing client-id ${cid_4} on ${device}"
			uqmi -d "$device" $options --set-client-id wds,$cid_4 --release-client-id wds
			proto_notify_error "$interface" FAILED_IPV4_IPV4V6
			cid_4=""
		}
		[ "$ipv6" != "1" ] && {
			echo "Releasing client-id ${cid_6} on ${device}"
			uqmi -d "$device" $options --set-client-id wds,$cid_6 --release-client-id wds
			proto_notify_error "$interface" FAILED_IPV6_IPV4V6
			cid_6=""
		}
		[ "$ipv4" != "1" ] && [ "$ipv6" != "1" ] && {
			echo "Failed to create IPV4V6 connection"
			proto_notify_error "$interface" FAILED_IPV4V6
			sleep $fail_timeout
			handle_retry "$RETRY_BEFORE_REINT"
			return 1
		}
		;;
	esac

	local registration_timeout=0
	local serving_system="$(uqmi -s -d "$device" $options --get-serving-system)"
	qmi_error_handle "$serving_system" "$modem" || return 1
	while [ "$(echo "$serving_system" | grep registration | \
		awk -F '\"' '{print $4}')" != "registered" ] && \
		[ "$( echo "$serving_system" | grep PS | awk -F ' ' '{print $2}' | \
		awk -F ',' '{print $1}')" != "attached" ]
	do
		[ -e "$device" ] || return 1
		if [ "$registration_timeout" -lt "$timeout" ]; then
			let "registration_timeout += 5"
			sleep 5
		else
			echo "Network registration failed"
			proto_notify_error "$interface" NO_NETWORK
			return 1
		fi
		serving_system="$(uqmi -s -d "$device" $options --get-serving-system)"
		qmi_error_handle "$serving_system" "$modem" || return 1
	done

	echo "Setting up $ifname"

	[ -z "$mtu" ] || {
		echo "Setting MTU: $mtu on $ifname"
		ip link set mtu "$mtu" "$ifname"
	}

	proto_init_update "$ifname" 1
	proto_set_keep 1
	proto_add_data

	[ -n "$pdh_4" ] && {
		json_add_string "cid_4" "$cid_4"
		json_add_string "pdh_4" "$pdh_4"
	}

	[ -n "$pdh_6" ] && {
		json_add_string "cid_6" "$cid_6"
		json_add_string "pdh_6" "$pdh_6"
	}

	json_add_string "pdp" "$pdp"
	json_add_string "method" "$method"
	proto_close_data
	proto_send_update "$interface"

	local zone="$(fw3 -q network "$interface" 2>/dev/null)"

	[ "$method" = "bridge" ] || [ "$method" = "passthrough" ] && [ -n "$cid_4" ] && {
		setup_bridge_v4 "$ifname"
		ethtool -r eth0
		iptables -A postrouting_rule -o rmnet0 -tnat -j ACCEPT

		#Passthrough
		[ "$method" = "passthrough" ] && {
			iptables -tnat -I postrouting_rule -o "$ifname" -j SNAT --to "$bridge_ipaddr"
			ip route add default dev "$ifname"
		}
	}

	[ "$method" != "bridge" ] && [ "$method" != "passthrough" ] && [ -n "$pdh_4" ] && {
		if [ "$dhcp" = 0 ]; then
			setup_static_v4 "$ifname"
		else
			setup_dhcp_v4 "$ifname"
		fi

		IFACE4="${interface}_4"
		ubus_set_interface_data "$modem" "$sim" "$zone" "$IFACE4"
	}

	[ -n "$pdh_6" ] && {
		if [ "$dhcpv6" = 0 ]; then
			setup_static_v6 "$ifname"
		else
			setup_dhcp_v6 "$ifname"
		fi

		IFACE6="${interface}_6"
		ubus_set_interface_data "$modem" "$sim" "$zone" "$IFACE6"
	}

	#cid is lost after script shutdown so we should create temp files for that
	mkdir -p "/var/run/qmux/"
	echo "$cid_4" > "/var/run/qmux/$interface.cid_4"
	echo "$cid_6" > "/var/run/qmux/$interface.cid_6"

	proto_export "IFACE4=$IFACE4"
	proto_export "IFACE6=$IFACE6"
	proto_export "OPTIONS=$options"
	proto_run_command "$interface" qmuxtrack "$device" "$cid_4" "$cid_6"
}

proto_connm_teardown() {
	local interface="$1"
	local device="/dev/cdc-wdm0" conn_proto
	local bridge_ipaddr

	[ -n "$ctl_device" ] && device=$ctl_device

	echo "Stopping network $interface"

	json_load "$(ubus call network.interface.$interface status)"
	json_select data
	json_get_vars pdh_4 pdh_6 method bridge_ipaddr

	conn_proto="qmux"

	clear_connection_values $interface $device "4" $conn_proto
	ubus call network.interface down "{\"interface\":\"${interface}_4\"}"

	clear_connection_values $interface $device "6" $conn_proto
	ubus call network.interface down "{\"interface\":\"${interface}_6\"}"

	[ "$method" = "bridge" ] || [ "$method" = "passthrough" ] && {
		ip rule del pref 5042
		ip rule del pref 5043
		ip route flush table 42
		ip route flush table 43
		ip route del "$bridge_ipaddr"
		ethtool -r eth0
	}

	#Clear passthrough and bridge params
	iptables -t nat -F postrouting_rule
	rm -f "/tmp/dnsmasq.d/bridge" 2>/dev/null
	ip neigh flush proxy

	proto_init_update "*" 0
	proto_send_update "$interface"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol connm
}
