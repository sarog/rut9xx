#!/bin/sh

. /lib/functions.sh
. /usr/share/libubox/jshn.sh

[ -f "/etc/config/teltonika" ] || return 0

interface=mob1s1a1
_gsm_sim1=
_wdcmda_sim1=
_lte_sim1=

prepare_convert_bands() {
	#Using option callback to parse value from option name :O
	#store values by type and sim
	config_cb() {
		local name="$2"
		if [ "$name" = "bands" ]; then
			option_cb() {
				local option="$1"
				local value="$2"
				local sim="${option##*sim}"
				local band
				[ "$value" = "1" ] || return

				case "$option" in
					gsm*)
						band="${option#gsm}"
						band="${band%%_sim*}"
						append "_gsm_sim$sim" "$band"
						;;
					wcdma*)
						band="${option#wcdma}"
						band="${band%%_sim*}"
						append "_wcdma_sim$sim" "$band"
						;;
					lte*)
						band="${option#lte}"
						band="${band%%_sim*}"
						append "_lte_sim$sim" "$band"
						;;
				esac
			}
		else {
			option_cb() { return; }
		}
		fi
	}
}

add_bands() {
	local section="$1"
	local sim="$2"
	local auto j
	config_get auto bands "auto_$sim"
	[ "$auto" = "disable" ] || return

	uci_set simcard "$section" band manual

	for j in $(eval echo -n \$_gsm_"$sim"); do
		uci_add_list simcard "$section" gsm "gsm_$j"
	done

	for j in $(eval echo -n \$_wcdma_"$sim"); do
		uci_add_list simcard "$section" umts "wcdma_$j"
	done

	for j in $(eval echo -n \$_lte_"$sim"); do
		uci_add_list simcard "$section" lte "lte_$j"
	done
}

parse_service_mode() {
	local mode="$1"
	local service_mode

	case "$mode" in
		gprs-only | gprs) service_mode="2g";;
		umts-only | umts) service_mode="3g";;
		lte-only | lte) service_mode="lte";;
		gprs-umts) service_mode="2g_3g";;
		gprs-lte) service_mode="2g_lte";;
		umts-lte) service_mode="3g_lte";;
		*) service_mode="$mode";;
	esac

	echo -n "$service_mode"
}

set_method() {
	local sec="$1" value="$2"

	case "$value" in
		pbridge)
			value="passthrough"
			;;
	esac

	uci_set network "$sec" method "$value"
}

remove_old_options() {
	uci_remove network "$interface" enabled
	uci_remove network "$interface" backup
	uci_remove network "$interface" cid
	uci_remove network "$interface" device
	uci_remove network "$interface" pppd_options
	uci_remove network "$interface" dialnumber
	uci_remove network "$interface" service
	uci_remove network "$interface" auth_mode
	uci_remove network "$interface" ifname
	uci_commit network

	uci_remove simcard ppp
	uci_remove simcard simcard
	uci_remove simcard rules
	uci_remove simcard option
	uci_remove simcard "sim2"
	uci_commit simcard
}

get_network_part() {
	config_get auth "$1" auth_mode
	config_get method "$1" method "nat"
}

get_simcard_part() {
	config_get auto_apn "$1" auto_apn
	config_get apn "$1" apn
	config_get pincode "$1" pincode
	config_get force_apn "$1" force_apn
	config_get mac "$1" bind_mac
	config_get username "$1" username
	config_get password "$1" password
	config_get roaming "$1" roaming
	config_get service "$1" service
	config_get pdptype "$1" pdptype
	config_get method "$1" method nat
	config_get mac "$1" bind_mac
	config_get passthrough_mode "$1" passthrough_mode
	config_get leasetime "$1" leasetime
	config_get letter "$1" letter
	config_get numeric "$1" numeric
	config_get mode "$1" mode

	#passthrough
	[ -z "$mac" ] && \
		config_get mac "$1" mac
}

add_simcard_part() {

	uci_add simcard sim
	uci_set simcard "$CONFIG_SECTION" position "$1"
	uci_set simcard "$CONFIG_SECTION" auto_apn "$auto_apn"
	uci_set simcard "$CONFIG_SECTION" pincode "$pincode"
	uci_set simcard "$CONFIG_SECTION" modem "$id"
	uci_set simcard "$CONFIG_SECTION" deny_roaming "$roaming"
	[ -n "$force_apn" ] && uci_set simcard "$CONFIG_SECTION" force_apn "$force_apn"
	uci_set simcard "$CONFIG_SECTION" band auto

	[ "$service" != "auto" ] && \
	service=$(parse_service_mode "$service")
	uci_set simcard "$CONFIG_SECTION" service "$service"

	[ "${default_sim:(-1):1}" = "$1" ] && \
	uci_set simcard "$CONFIG_SECTION" primary 1

	if [ -z "$mode" ] && [ -z "$numeric" ]; then
		uci_set simcard "$CONFIG_SECTION" operator "auto"
	elif [ -n "$mode" ]; then
		uci_set simcard "$CONFIG_SECTION" operator "manual-auto"
		uci_set simcard "$CONFIG_SECTION" opernum "$numeric"
	else
		uci_set simcard "$CONFIG_SECTION" operator "manual"
		uci_set simcard "$CONFIG_SECTION" opernum "$numeric"
	fi

	add_bands "$CONFIG_SECTION" "sim$1"
	uci_remove simcard "sim$1"
	uci_commit simcard
}

add_netowrk_part() {
	local intf="$interface"
	[ "$1" -gt 1 ] && intf="mob1s${1}a1"
	uci_add network interface "$intf"
	uci_set network "$intf" proto wwan
	uci_set network "$intf" sim "$1"
	uci_set network "$intf" pdp "1"
	uci_set network "$intf" auth "$auth"
	uci_set network "$intf" modem "$id"
	uci_set network "$intf" mac "$mac"
	uci_set network "$intf" apn "$apn"
	uci_set network "$intf" username "$username"
	uci_set network "$intf" password "$password"
	uci_set network "$intf" deny_roaming "$roaming"
	uci_set network "$intf" passthrough_mode "$passthrough_mode"
	uci_set network "$intf" leasetime "${leasetime}${letter}"
	set_method "$intf" "$method"

	[ -z "$pdptype" ] && {
		uci_set network "$intf" pdptype ipv4v6
	}

	uci_commit network
}

prepare_convert_bands
config_load simcard
config_get default_sim simcard default "1"

json_load_file /etc/board.json
json_select modems
json_select 1
json_get_var id id
json_cleanup

get_network_part "sim1"
get_simcard_part "sim1"

add_netowrk_part "1"
add_simcard_part "1"

remove_old_options
exit 0
