. /lib/functions.sh

create_mobile_iface() {

	local intf="$1"
	local modem="$2"
	local modem_number="$3"
	local sim_card="$4"

	[ -n "$sim_card" ] || sim_card=1
	logger "Adding new interface: $intf modem $modem metric $((modem_number+1))"

	uci_add network interface "$intf"
	uci_set network $intf proto "wwan"
	uci_set network $intf modem "$modem"
	uci_set network $intf sim "$sim_card"
	uci_set network $intf metric "$((modem_number+1))"
	uci_set network $intf pdp "1"

	# just like this for now
	# probably we should merge connm with wwan
	if [ ! -e /dev/smd9 ]; then
		uci_set network $intf proto "wwan"
	else
		uci_set network $intf proto "connm"
		uci_set network $intf ifname "rmnet0"
	fi

	uci_commit network

	ubus call network reload
	return 0
}

update_firewall_zone(){
	local zone="$1"
	local intf="$2"

	local name network

	update_firewall(){
		local cfg="$1"
		local intf="$2"
		local name network

		config_get name "$cfg" name
		config_get network "$cfg" network

		if [ "$name" = "wan" ] && ! list_contains network "$intf"; then
			append network "$intf"
			uci_set firewall "$cfg" network "$network"
			uci_commit firewall
		fi
	}

	config_load "firewall"
	config_foreach update_firewall zone $intf
}

create_multiwan_iface(){

	[ -f /etc/config/mwan3 ] || return 0

	local intf="$1"
	local metric="$2"
	local section

	uci_add mwan3 interface "$intf"
	section="$CONFIG_SECTION"
	uci_set mwan3 $section interval '3'
	uci_set mwan3 $section enabled '0'
	uci_set mwan3 $section family 'ipv4'

	uci_add mwan3 condition
	section="$CONFIG_SECTION"
	uci_set mwan3 $section interface "$intf"
	uci_set mwan3 $section track_method 'ping'
	uci add_list mwan3.$section.track_ip="1.1.1.1"
	uci add_list mwan3.$section.track_ip="8.8.8.8"
	uci_set mwan3 $section reliability '1'
	uci_set mwan3 $section count '1'
	uci_set mwan3 $section timeout '2'
	uci_set mwan3 $section down '3'
	uci_set mwan3 $section up '3'

	uci_add mwan3 member "${intf}_member_mwan"
	section="$CONFIG_SECTION"
	uci_set mwan3 $section interface "$intf"
	uci_set mwan3 $section metric "$((metric + 1))"

	uci_add mwan3 member "${intf}_member_balance"
	section="$CONFIG_SECTION"
	uci_set mwan3 $section interface "$intf"
	uci_set mwan3 $section weight "1"

	uci add_list mwan3.mwan_default.use_member="${intf}_member_mwan"
	uci add_list mwan3.balance_default.use_member="${intf}_member_balance"
	uci_commit mwan3
}

MODEM_FOUND="0"
check_modem_id() {

	local cfg="$1"
	local current_modem_id="$2"
	local new_position="$3"
	local option="$4"
	local position

	config_get modem "$cfg" "$option" ""

	if [ "$modem" == "$current_modem_id" ]; then

		[ "$option" = "info_modem_id" ] && MODEM_FOUND="1" && return

		if [ "$new_position" = "" ]; then
			MODEM_FOUND="1"
			return
		fi

		config_get position "$cfg" position ""
		if [ "$new_position" = "$position" ]; then
			MODEM_FOUND="1"
		fi
	fi

}

add_simcard_config(){
	local pin
	local device="$1"
	local position="$2"
	local primary="$3"
	local builtin="$4"
	local volte="$5"

 	if [ -s /etc/config/simcard ]; then
		MODEM_FOUND="0"
		config_load "simcard"
		config_foreach check_modem_id sim "$device" "$position" modem
	fi

	if [ "$MODEM_FOUND" = "1" ]; then
		return
	fi

	[ -z "$position" ] && position=1

	uci_add simcard sim
	uci_set simcard $CONFIG_SECTION modem "$device"
	uci_set simcard $CONFIG_SECTION position "$position"
	uci_set simcard $CONFIG_SECTION volte "$volte"


	[ "$primary" -eq 1 ] && uci_set simcard $CONFIG_SECTION primary "$primary"

	[ "$builtin" = "1" ] && {
		pin="$(/sbin/mnf_info --simpin $position)"
		[ $pin -eq $pin ] && uci_set simcard $CONFIG_SECTION pincode $pin
	}
	uci_commit simcard

	[ -x "/bin/trigger_vuci_routes_reload" ] && /bin/trigger_vuci_routes_reload
}

add_sms_storage_config(){

	local device="$1"
	MODEM_FOUND="0"

	[ -f /etc/config/sms_gateway ] || return

	config_load "sms_gateway"
	config_foreach check_modem_id simman "$device" "1" info_modem_id
	
	[ "$MODEM_FOUND" = "1" ] && return

	uci_add sms_gateway simman
	uci_set sms_gateway "$CONFIG_SECTION" free "5"
	uci_set sms_gateway "$CONFIG_SECTION" info_modem_id "$device"
	uci_commit sms_gateway
}

add_sim_switch_config(){
	local device="$1"
	local position="$2"

	[ -f /etc/config/sim_switch ] || return 0
	[ -z "$position" ] && position=1

	uci_add sim_switch sim
	uci_set sim_switch $CONFIG_SECTION modem "$device"
	uci_set sim_switch $CONFIG_SECTION position "$position"

	uci_commit sim_switch

	[ -x "/bin/trigger_vuci_routes_reload" ] && /bin/trigger_vuci_routes_reload
}

add_quota_limit_config(){
	local interface="$1"
	touch /etc/config/quota_limit

	[ -z "$(uci_get quota_limit "$interface")" ] && {
		uci_add quota_limit interface "$interface"
		uci_commit quota_limit
	}
}

configure_modem(){
	local cfg="$1"
	local device="$2"
	local modem_cnt="$3"

	create_mobile_iface "$cfg" "$device" "$modem_cnt"
	update_firewall_zone "wan" "$cfg"
	create_multiwan_iface "$cfg" "$modem_cnt"
	add_simcard_config "$device" 1 1 "" "auto"
	add_sim_switch_config "$device" 1
	add_quota_limit_config "$cfg"
}
