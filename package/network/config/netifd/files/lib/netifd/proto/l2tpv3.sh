#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_l2tpv3_init_config() {
	proto_config_add_string 'ipaddr'
	proto_config_add_string 'netmask'
	proto_config_add_string 'ip6addr'
	proto_config_add_int 'mtu'
	proto_config_add_string 'bridge_to'

	proto_config_add_int 'peer_tunnel_id'
	proto_config_add_int 'tunnel_id'
	proto_config_add_string 'encap'
	proto_config_add_int 'udp_sport'
	proto_config_add_int 'udp_dport'

	proto_config_add_string 'localaddr'
	proto_config_add_string 'peeraddr'
	proto_config_add_int 'peer_session_id'
	proto_config_add_int 'session_id'
	proto_config_add_string 'cookie'
	proto_config_add_string 'peer_cookie'
	proto_config_add_string 'l2spec_type'

	available=1

	# XXX: this appears to disable `option type bridge`
	#       handler->proto.flags |= PROTO_FLAG_NODEV
	#no_device=1
	proto_config_add_defaults
}

proto_l2tpv3_setup() {
	local config="$1"
	local iface="$2"
	local link="l2v3-$config"

	#~ check if l2tp configurator is available
	ip help 2>&1 | grep -q l2tp || {
		echo "Cannot find ip-full configuration utility"
		proto_notify_error "$config" "NO_IP_FULL_UTILITY"
		proto_set_available "$config" 0
		return 1
	}

	local ipaddr netmask ip6addr mtu localaddr peeraddr tunnel_id \
		session_id peer_session_id peer_tunnel_id encap udp_sport udp_dport \
		bridge_to cookie peer_cookie l2spec_type wanif

	json_get_vars ipaddr netmask ip6addr mtu localaddr peeraddr tunnel_id \
		session_id peer_session_id peer_tunnel_id encap udp_sport udp_dport \
		bridge_to cookie peer_cookie l2spec_type

	[ -z "$peeraddr" ] && {
		proto_notify_error "$config" "MISSING_PEER_ADDRESS"
		proto_set_available "$config" 0
		return 1
	}

	remoteip=$(resolveip -t 5 "$peeraddr")

	[ -z "$remoteip" ] && {
		proto_notify_error "$config" "PEER_RESOLVE_FAILED"
		return 1
	}

	for ip in $remoteip; do
		peeraddr=$ip
		break
	done

	[ -z "$localaddr" ] && {

		network_find_wan wanif || {
			proto_notify_error "$config" "NO_WAN_LINK_IFACE"
			sleep 5
			return 1
		}

		network_get_ipaddr localaddr "$wanif" || {
			proto_notify_error "$cfg" "NO_WAN_LINK_IP"
			return 1
		}
	}

	if
		ip l2tp add tunnel \
		local "$localaddr" \
		remote "$peeraddr" \
		tunnel_id "$tunnel_id" \
		peer_tunnel_id "$peer_tunnel_id" \
		${encap:+encap "$encap"} \
		${udp_sport:+udp_sport "$udp_sport"} \
		${udp_dport:+udp_dport "$udp_dport"} \
		${udp_csum:+udp_csum "$udp_csum"}
	then
		echo "Tunnel $tunnel_id successfully created"
	else
		proto_notify_error "$config" "FAILED_TO_ADD_TUNNEL"
		sleep 5
		return 1
	fi

	if
		ip l2tp add session \
		name "$link" \
		tunnel_id "$tunnel_id" \
		session_id "$session_id" \
		peer_session_id "$peer_session_id" \
		${cookie:+cookie "$cookie"} \
		${peer_cookie:+peer_cookie "$peer_cookie"} \
		${l2spec_type:+l2spec_type "$l2spec_type"}
	then
		echo "Session $session_id successfully created"
	else
		proto_notify_error "$config" "FAILED_TO_ADD_SESSION"
		return 1
	fi

	[ -n "$mtu" ] && ip link set mtu "$mtu" dev "$link"

	proto_add_host_dependency "$config" "$peeraddr"

	echo "Setting up $link"
	proto_init_update "$link" 1
	proto_set_keep 1
	[ -n "$ipaddr" ] && [ -n "$netmask" ] && {
		proto_add_ipv4_address "$ipaddr" "$netmask"
	}
        [ -n "$ip6addr" ] && {
                proto_add_ipv6_address "$ip6addr"
        }
	proto_add_data
		json_add_string "tunnel_id" "$tunnel_id"
		json_add_string "session_id" "$session_id"
		json_add_string "peer_tunnel_id" "$peer_tunnel_id"
		json_add_string "peer_session_id" "$peer_session_id"
	proto_close_data
	proto_send_update "$config"
}

proto_l2tpv3_teardown() {
	local interface="$1"

	local session_id peeraddr tunnel_id
	json_get_vars tunnel_id session_id peeraddr

	ip l2tp del session \
		tunnel_id "$tunnel_id" session_id "$session_id"

	ip l2tp del tunnel \
		tunnel_id "$tunnel_id"

	remoteip=$(resolveip -t 1 "$peeraddr")
	ip route del "$peeraddr"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol l2tpv3
}
