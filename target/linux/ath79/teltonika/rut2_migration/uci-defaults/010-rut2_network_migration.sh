#!/bin/sh

. /lib/functions.sh
. /lib/functions/board.sh

[ -f "/etc/config/teltonika" ] || return 0

chng_loaddbal_memb() {
	fix_memb() {
		[ "$2" = "$(config_get "$1" interface)" ] && \
		uci_set load_balancing "$1" interface "$3"
	}

	(
		config_load load_balancing
		config_foreach fix_memb member "$1" "$2"
		uci_commit load_balancing
	)
}

fix_route() {
	#change route interface with supplied names
	change_route() {
		local interface
		config_get interface "$1" interface
		list_contains interface "$2" && \
		uci_set network "$1" interface "${interface/$2/$3}"
	}

	(
		config_load network
		config_foreach change_route route "$1" "$2"
	)
}

fix_ddns() {
	change_ddns() {
		local section="$1"
		local old="$2"
		local new="$3"
		local interface

		config_get interface "$section" interface ""
		if [ "$old" = "$interface" ]; then
			return
		fi
		uci_set ddns "$section" interface "$new"
		case "$new" in
		mob1s1a1)
			uci_set ddns "$section" ip_network "${new}_4"
			;;
		*)
			uci_set ddns "$section" ip_network "$new"
			;;
		esac
	}

	(
		config_load ddns
		config_foreach change_ddns service "$1" "$2"
		uci_commit ddns
	)
}

move_sstp() {
	config_get ca "$1" ca 0
	local ca_dir="/etc/vuci-uploads/cbid.network.${1}.caca.crt"

	mkdir -p "/etc/vuci-uploads"

	[ "$ca" != 0 ] && mv "$ca" "$ca_dir" && uci_set network "$1" "ca" "$ca_dir"
}

move_auto() {
	config_get enabled "$1" enabled
	[ "$enabled" = "1" ] && uci_set network "$1" "auto" "1" || uci_set network "$1" "auto" "0"
}

move_wireguard(){
	config_get disabled "$1" disabled
	[ "$disabled" = "0" ] && uci_set network "$1" "auto" "1" || uci_set network "$1" "auto" "0"
}

move_gre() {
	json_init
	json_load "$(ubus call sim get)"
	json_get_var sim sim
	[ -z "$sim" ] && sim=1

	config_get tunlink "$1" tunlink
	[ "$tunlink" = "ppp_4" ] || [ "$tunlink" = "ppp" ] && uci_set network "$1" "tunlink" "mob1s${sim}a1_4"
}

move_interface() {
	local section="$1"
	local ifname proto enb

	config_get ifname "$section" ifname
	config_get proto "$section" proto
	config_get enabled "$section" enabled

	[ "$proto" = "sstp" ] && move_sstp "$section"
	[ "$proto" = "l2tp" ] && move_auto "$section"
	[ "$proto" = "pptp" ] && move_auto "$section"
	[ "$proto" = "gre" ] && move_gre "$section"
	[ "$proto" = "wireguard" ] && move_wireguard "$section"

	case "$section" in
	wan*)
		case "$ifname" in
		wwan0 | \
		3g-ppp)
			uci_remove network "$section"
			uci_rename multiwan "$section" mob1s1a1
			chng_loaddbal_memb "$section" mob1s1a1
			fix_ddns "$section" mob1s1a1
			fix_route "$section" mob1s1a1
			;;
		wlan0)
			[ "$enabled" -eq 0 ] && {
				uci delete network."$section"
				continue
			}
			uci_rename network "$section" wwan
			uci_rename multiwan "$section" wwan
			chng_loaddbal_memb "$section" wwan
			fix_ddns "$section" wwan
			fix_route "$section" wwan
			;;
		eth1)
			uci_rename network "$section" wan
			uci_rename multiwan "$section" wan
			chng_loaddbal_memb "$section" wan
			fix_ddns "$section" wan
			fix_route "$section" wan
			;;
		esac
	;;
	ppp)
		uci_rename network "$section" mob1s1a1
		fix_ddns "$section" mob1s1a1
		fix_route "$section" mob1s1a1
	;;
	stabridge)
		config_get enb "$section" enabled
		[ "$enb" = 0 ] && {
			uci_remove network "$section"
			return
		}
		uci_set network "$section" network 'lan wwan'
		uci_set network "$section" lan_mark 'lan'
		uci_rename network "$section" lan_repeater
	;;
	eth1v6)
		if [ -z "$proto" ]; then
			uci_set network "$section" ifname 'eth1'
			uci_set network "$section" proto 'dhcpv6'
			uci_set network "$section" metric '1'
		fi
		uci_rename network "$section" wan6
	;;
	*)
		[ -z "$proto" ] && \
		uci_remove network "$section"
	;;
	esac
}

find_used_vlan() {
	#~ find_used_vlan <section> <vid> <variable>
	#~ Export section name to $3 variable

	[ $(config_get "$1" vid) = "$2" ] && {
		[ -n "$3" ] && export "$3=$1"
		return 0
	}
	return 1
}

replace_ifname() {
	local ifname_list
	ifname_list=$(uci_get network "$1" ifname)
	list_contains ifname_list "$2" && \
	uci_set network "$1" ifname "${ifname_list/$2/$3}"
}

fix_vlans() {
	#~ Check if vlan0 exists
	#~ Check if vlan0 can be changed to vlan1
	#~ Else Assign available vid from <= vlan256
	local vlan_zero_section vlan_one_section _temp_vlan_section ports

	config_foreach find_used_vlan switch_vlan 0 vlan_zero_section

	[ -n "$vlan_zero_section" ] && {
		config_foreach find_used_vlan switch_vlan 1 vlan_one_section
		[ -n "$vlan_one_section" ] && {
			for i in $(seq 256 -1 1); do
				_temp_vlan_section=
				config_foreach find_used_vlan switch_vlan "$i" _temp_vlan_section
				[ -n "$_temp_vlan_section" ] || {
					ports=$(uci_get network "$vlan_zero_section" ports)
					uci_set network "$vlan_zero_section" ports "${ports/0/0t}"
					uci_set network "$vlan_zero_section" vid "$i"
					config_foreach replace_ifname interface eth0 "eth0.$i"
					return 0
				}
			done
		}

		ports=$(uci_get network "$vlan_zero_section" ports)
		uci_set network "$vlan_zero_section" ports "${ports/0/0t}"
		uci_set network "$vlan_zero_section" vid 1
		config_foreach replace_ifname interface eth0 "eth0.1"
		return 0
	}

	return 0
}

rearange_vlans() {
	uci_set network "$1" vlan "$vlan_counter"
	vlan_counter=$((vlan_counter +1))
}
vlan_counter=1

delete_switch_vlan() {
	uci_remove network "$1"
}

delete_switch() {
	uci_remove network "$1"
}

fix_untagged() {
        config_get ports "$1" ports 0
        if [ "$ports" = "0t 2" ]
        then
                config_get vid "$1" vid
                config_foreach replace_ifname interface eth0.$vid "eth0"
        fi
}

remove_tun(){
	local proto
	config_get proto tun proto
	if [ "$proto" = "none" ]; then
		uci_remove network "tun"
	fi
}

#Please check quota_limit migration script before doing any changes here.
config_load network
config_foreach fix_untagged switch_vlan
config_foreach delete_switch_vlan switch_vlan
config_foreach delete_switch switch
config_foreach move_interface interface
remove_tun
uci_commit network

exit 0
