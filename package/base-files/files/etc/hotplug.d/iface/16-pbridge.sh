#!/bin/sh
#
# Copyright (C), 2019 Teltonika
#

. /lib/netifd/netifd-proto.sh
. /lib/functions.sh
. /lib/functions/network.sh
. /lib/teltonika-functions.sh

DEBUG="0"
NAME="16--PBRIDGE.SH"

#Debug
debug(){
	if [ "$DEBUG" = "0" ]; then
		return
	fi

	if [ "$2" = "" ]; then
		logger -t "$NAME" "$1"
	else
		logger -t "$NAME: $1" "$2"
	fi
}

debug "hotplug usb: action='$ACTION' devicename='$DEVICENAME' devname='$DEVNAME' devpath='$DEVPATH' product='$PRODUCT' type='$TYPE'"
debug "AAAAAAAAAAA" "DEVICE: $DEVICE"
debug "AAAAAAAAAAA" "ACTION: $ACTION"
debug "AAAAAAAAAAA" "INTERFACE: $INTERFACE"

KEEP_SETTINGS_FILE="/usr/sbin/check_bridge_pbridge"
DNSMASQ_CONF="/tmp/dnsmasq.d/passthrough"
FOUND_SECTION="0"

check_forward()
{
	local section="$1"

	config_get name "$section" "name" ""

	if [ "$name" = "bridge" -a "$FOUND_SECTION" = "0"  ];then
		config_set "$section" src "wan"
		config_set "$section" dest "lan"

		uci commit firewall

		FOUND_SECTION="1"
	fi
}

delete_rule()
{
	local section="$1"

	config_get name "$section" "name" ""
		if [ "$name" = "bridge" ];then
			uci delete firewall.$section
			uci commit firewall
		fi

}
create_new_mode()
{
	. /usr/share/libubox/jshn.sh

	local WIP WGW SUBNET DNS NETMASK
	local CON_MODE=$1
	network_flush_cache

	network_get_ipaddr WIP "$INTERFACE"
	network_get_gateway WGW "$INTERFACE"
	network_get_subnet SUBNET "$INTERFACE"
	network_get_dnsserver DNS "$INTERFACE"

	network_get_ipaddr current_address "lan_passthrough"

	debug "KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK" "WIP: $WIP"
	debug "KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK" "WGW: $WGW"
	debug "KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK" "SUBNET: $SUBNET"
	debug "KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK" "DNS: $DNS"
	debug "KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK" "LAN PASSTHROUGH: $current_address"
	if [ -f "$KEEP_SETTINGS_FILE" ]; then
		$KEEP_SETTINGS_FILE
	fi
	if [ -z "$WIP" -o -z "$WGW" -o -z "$SUBNET" -o "$WIP" = "1.2.3.4" ]; then
		return 1
	fi


	#~ ip addr add $WGW/${SUBNET#*/} dev br-lan
	NETMASK="$(ipcalc.sh $WGW/${SUBNET#*/} | grep "NETMASK")"
	NETMASK="${NETMASK#*=}"

	rm /tmp/dhcp.leases

# 	iptables -tnat -I POSTROUTING -o wwan0 -j SNAT --to-source $WIP
	# enabling firewall rule

	config_load firewall
	config_foreach check_forward "forwarding"

	if [ "$FOUND_SECTION" = "0" ]; then
		section=$(uci add firewall forwarding)
		uci set firewall.$section.src='wan'
		uci set firewall.$section.dest='lan'
		uci set firewall.$section.name='bridge'
	fi

	sect=$(uci -q get firewall.M_PASSTH)
	if [ "$sect" = "" ]; then
		uci set firewall.M_PASSTH='redirect'
		uci -q set firewall.M_PASSTH.target='SNAT'
		uci -q set firewall.M_PASSTH.dest='wan'
		uci -q set firewall.M_PASSTH.proto='all'
		uci -q set firewall.M_PASSTH.name="Enable_Mobile_Passthrough"
	fi
	uci -q set firewall.M_PASSTH.enabled='1'
	uci -q set firewall.M_PASSTH.src_dip="$WIP"


	[ "$CON_MODE" == "bridge" ] && uci -q set firewall.M_PASSTH.src_ip="$WIP"

	uci commit firewall

	json_init
	json_load "$(ubus call sim get)"
	json_get_var current_sim sim
	if [ "$current_sim" = "" ]; then
		current_sim="1"
	fi

	dhcp_lease_num="$(uci -q get simcard.sim$current_sim.leasetime)"
		if [ "$dhcp_lease_num" = "" ]; then
			dhcp_lease_num="12"
		fi

	dhcp_lease_letter="$(uci -q get simcard.sim$current_sim.letter)"
		if [ "$dhcp_lease_letter" = "" ]; then
			dhcp_lease_letter="h"
		fi

	leasetime="$dhcp_lease_num""$dhcp_lease_letter"

	debug "KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK" "CON_MODE: $CON_MODE"
	if [ "$CON_MODE" == "bridge" ]; then
# 		uci set dhcp.lan.interface="lan_disabled"
# 		uci commit dhcp

		echo "dhcp-range=lan,$WIP,$WGW,$NETMASK,$leasetime" > $DNSMASQ_CONF
		local dns_servers=${DNS//' '/','}
		echo "dhcp-option=lan,6,$dns_servers" >> $DNSMASQ_CONF
		bind_mac="$(uci -q get network.ppp.bind_mac)"
		if [ "$bind_mac" = "" ]; then
			bind_mac="*:*:*:*:*:*"
		fi
		echo "dhcp-host=$bind_mac,$WIP" >> $DNSMASQ_CONF
	else
		dhcp_mode="$(uci -q get simcard.sim$current_sim.passthrough_mode)"
		if [ "$dhcp_mode" != "no_dhcp" ]; then
			uci set dhcp.passthrough="host"
			uci set dhcp.passthrough.ip="$WIP"
			uci set dhcp.passthrough.name="Passthrough"

			if [ "$dhcp_mode" = "static" ]; then
				static_mac="$(uci get -q simcard.sim$current_sim.mac)"
				uci set dhcp.passthrough.mac="$static_mac"

			elif [ "$dhcp_mode" = "dynamic" ]; then
				uci set dhcp.passthrough.mac='*:*:*:*:*:*'
			fi

			# disable for lan interface
# 			uci set dhcp.lan.interface="lan_disabled"
# 			uci commit dhcp

			echo "dhcp-range=lan,$WIP,$WGW,$NETMASK,$leasetime" > $DNSMASQ_CONF
		else
			. /lib/functions/network.sh
			network_get_device lan_ifname "lan" && \
				echo "no-dhcp-interface=$lan_ifname" > $DNSMASQ_CONF
		fi
	fi

	sysctl -w net.ipv4.conf.wwan0.route_localnet=1

	# setting wwan0 interface IP
	#~ ifconfig $DEVICE 1.2.3.4
	#~ route add default $DEVICE

	if [ "$(uci -q get network.ppp.proto)" = "ncm" ]; then
		echo "Setting up $DEVICE"
		proto_init_update "$DEVICE" 1
		proto_set_keep 1
		proto_send_update "ppp"

		echo "Setting up ppp_4 interface"
		local zone="$(fw3 -q network ppp 2>/dev/null)"

		json_init
		json_add_string name "ppp_4"
		json_add_string ifname "@ppp"
		json_add_string proto "static"
		json_add_string gateway "0.0.0.0"

		json_add_array ipaddr
			json_add_string "" "1.2.3.4/32"
		json_close_array

		json_add_array dns
			json_add_string "" "$DNS"
		json_close_array

		[ -n "$zone" ] && {
			json_add_string zone "$zone"
		}

		ubus call network add_dynamic "$(json_dump)"

	else
		proto_init_update "$DEVICE" 1
		proto_add_ipv4_address "1.2.3.4" "32"
		proto_add_ipv4_route "0.0.0.0" "0"
		proto_add_dns_server "$DNS"
		proto_send_update "$INTERFACE"

	fi

	#/etc/init.d/firewall reload &
	json_init
	json_add_string name "lan_passthrough"
	json_add_string ifname "@lan"
	json_add_string proto "static"
	json_add_array ipaddr
	json_add_string "" "$WGW/${SUBNET#*/}"
	json_close_array
	json_close_object
	ubus call network add_dynamic "$(json_dump)"
	/etc/init.d/firewall reload

	if [ "$dhcp_mode" != "no_dhcp" ] && [ "$WGW" != "$current_address" ]; then
		# reset switch in order to get new dhcp lease
		swconfig dev eth0 set reset 1
		swconfig dev eth0 set apply 1
	else
		debug "KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK" "Skip lan reset, address did not change!"
	fi

	/etc/init.d/dnsmasq restart

	debug "KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK" "Done."

}

if [ "$DEVICE" == "eth2" ] || [ "$DEVICE" == "3g-ppp" ] || [ "$DEVICE" == "wwan0" ]; then
	ppp_method=`uci get -q network.ppp.method`
	ppp_enabled=`uci get -q network.ppp.enabled`
	debug "KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK" "ppp_method:$ppp_method"
	debug "KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK" "ppp_enabled:$ppp_enabled"

	if [ "$ppp_method" = "pbridge" -a "$ppp_enabled" != "0" -o "$ppp_method" = "bridge" -a "$ppp_enabled" != "0" ]; then
		if [ "$ACTION" = "ifup" ] || [ "$ACTION" = "ifupdate" ]; then
			create_new_mode $ppp_method
		fi
	else
		# restore dhcp for lan interface
		if [ `uci get -q dhcp.lan.interface` == "lan_disabled" ]; then
			uci set dhcp.lan.interface="lan"
			uci commit dhcp

			# restart dnsmasq to use new config
			/etc/init.d/dnsmasq restart

			# reset switch in order to get new dhcp lease
			swconfig dev eth0 set reset 1
			swconfig dev eth0 set apply 1
		fi
	fi
fi

if [ "$ACTION" = "ifdown" -a "$INTERFACE" = "ppp" ]; then

	debug "HHHHHHHHHHHHHHHHHHHHHHHHHHH" "VEIKIA PTHROUGH DOWN"

	config_load firewall
	config_foreach delete_rule "forwarding"

	uci -q set firewall.M_PASSTH.enabled='0'
	uci -q del firewall.M_PASSTH.src_ip
	uci commit firewall
	ifdown lan_passthrough

	sysctl -w net.ipv4.conf.wwan0.route_localnet=0

	rm "$DNSMASQ_CONF"
	uci delete dhcp.passthrough
	/etc/init.d/firewall reload

	sleep 1
	/etc/init.d/dnsmasq restart
	sleep 2
fi
