#!/bin/sh
. /lib/netifd/mac80211.sh

append DRIVERS "mac80211"

lookup_phy() {
	[ -n "$phy" ] && {
		[ -d /sys/class/ieee80211/$phy ] && return
	}

	local devpath
	config_get devpath "$device" path
	[ -n "$devpath" ] && {
		phy="$(mac80211_path_to_phy "$devpath")"
		[ -n "$phy" ] && return
	}

	local macaddr="$(config_get "$device" macaddr | tr 'A-Z' 'a-z')"
	[ -n "$macaddr" ] && {
		for _phy in /sys/class/ieee80211/*; do
			[ -e "$_phy" ] || continue

			[ "$macaddr" = "$(cat ${_phy}/macaddress)" ] || continue
			phy="${_phy##*/}"
			return
		done
	}
	phy=
	return
}

find_mac80211_phy() {
	local device="$1"

	config_get phy "$device" phy
	lookup_phy
	[ -n "$phy" -a -d "/sys/class/ieee80211/$phy" ] || {
		echo "PHY for wifi device $1 not found"
		return 1
	}
	config_set "$device" phy "$phy"

	config_get macaddr "$device" macaddr
	[ -z "$macaddr" ] && {
		config_set "$device" macaddr "$(cat /sys/class/ieee80211/${phy}/macaddress)"
	}

	return 0
}

check_mac80211_device() {
	config_get phy "$1" phy
	[ -z "$phy" ] && {
		find_mac80211_phy "$1" >/dev/null || return 0
		config_get phy "$1" phy
	}
	[ "$phy" = "$dev" ] && found=1
}

check_qcawifi_config() {
	config_get old_qca_type "wifi$devidx" type
	[ -n "$old_qca_type" -a  "$old_qca_type" = "qcawifi" ] && {
		old_qca_devidx=$(($old_qca_devidx + 1))
	}
}

convert_qcawifi_dev_opts() {
	config_get old_qcawifi_hwmode "$1" hwmode
	[ -n "$old_qcawifi_hwmode" ] && {
		case "$old_qcawifi_hwmode" in
			11ng )
				uci set  wireless."$1".hwmode="11g"
				device_name_2G="$1"
				;;
			11bg )
				uci set  wireless."$1".hwmode="11g"
				uci delete wireless."$1".htmode
				device_name_2G="$1"
				;;
			11na )
				uci set  wireless."$1".hwmode="11a"
				uci set  wireless."$1".htmode="HT20"
				device_name_5G="$1"
				;;
			11ac )
				uci set  wireless."$1".hwmode="11a"
				device_name_5G="$1"
				;;
		esac
	}

	config_get old_qcawifi_type "$1" type
	[ -n "$old_qcawifi_type" ] && {
		uci set  wireless."$1".type="mac80211"
	}

	config_get old_qcawifi_country "$1" country
	[ -n "$old_qcawifi_country" ] &&
	[ "$old_qcawifi_country" == "00" ] && {
		uci set wireless."$1".country="US"
	}

	uci delete wireless."$1".macaddr
	uci commit wireless
}

convert_qcawifi_section() {
	uci rename wireless."$device_name_2G"="radio0"
	uci rename wireless."$device_name_5G"="radio1"
}

convert_qcawifi_iface_opts() {
	config_get old_qcawifi_uapsd "$1" uapsd
	[ -n "$old_qcawifi_uapsd" ] && {
		uci delete wireless."$1".uapsd
	}
}

rename_iface_id_options() {
	local id device
	config_get uci_wifi_id "$1" wifi_id
	config_get device "$1" device
	[[ -n "$uci_wifi_id" ]] && {
		if [[ "radio" == ${uci_wifi_id:0:5} ]]; then
			id=${uci_wifi_id#radio}
			id_list="$id_list wifi$id"
			id=$((id + $2))
			uci set wireless."$1".wifi_id="wifi$id"
		else
			id_list="$id_list $uci_wifi_id"
		fi
	}

	[ -n "$device" ] && [ "wifi" = ${device:0:4} ] && {
		id=${device#wifi}
		uci set wireless."$1".device="radio$id"
	}
}

recount_iface_wifi_id() {
	local id
	config_get uci_wifi_id $1 wifi_id
	[[ -z "$uci_wifi_id" ]] && {
		for id in $(seq 0 99)
		do
			[[ $(echo "$id_list" | grep -wc "wifi${id}") -eq 0 ]] && break
		done

		id_list="$id_list wifi${id}"
		id=$((id + $2))
		uci -q set wireless."$1".wifi_id="wifi${id}"
	}
}

parse_qcawifi_config() {
	config_foreach convert_qcawifi_dev_opts wifi-device
	config_foreach convert_qcawifi_iface_opts wifi-iface

	convert_qcawifi_section
	uci set wireless.radio1.path="platform/soc/a800000.wifi"
	uci set wireless.radio0.path="platform/soc/a000000.wifi"

	config_load wireless
}

add_custom_wifi_iface() {
	[ -e "/etc/board.json" ] || return
	. /usr/share/libubox/jshn.sh

	local wireless
	local dvidx="$1"

	json_load_file "/etc/board.json"
	json_get_keys keys network

	for key in $keys; do
		json_select network
		json_select "$key"
		json_get_var wireless _wireless

		[ "$wireless" = "1" ] && {
			uci -q batch <<-EOF
			add  wireless wifi-iface
			set wireless.@wifi-iface[-1].device='radio${dvidx}'
			set wireless.@wifi-iface[-1].network='$key'
			set wireless.@wifi-iface[-1].mode=ap
			set wireless.@wifi-iface[-1].ssid='${key}_wifi'
			set wireless.@wifi-iface[-1].wifi_id='wifi${wifi_id}'
			set wireless.@wifi-iface[-1].encryption=none
			set wireless.@wifi-iface[-1].isolate=1
			set wireless.@wifi-iface[-1].disabled=1
			EOF
			wifi_id=$((wifi_id + 1))
		}

		json_select ..
		json_select ..
	done
}

detect_mac80211() {
	devidx=0
	old_qca_devidx=0
	old_qca_config=0
	device_name_2G=""
	device_name_5G=""
	local mac_add="0x2"
	local ssid
	local renamed=0
	local wifi_id=0
	local wps
	config_load wireless
	while :; do
		check_qcawifi_config
		config_get type "radio$devidx" type
		[ -n "$type" ] || break
		devidx=$(($devidx + 1))
	done

	[ "$old_qca_devidx" -gt 0 ] && {
		parse_qcawifi_config
	}

	for _dev in /sys/class/ieee80211/*; do
		[ -e "$_dev" ] || continue

		dev="${_dev##*/}"

		found=0
		config_foreach check_mac80211_device wifi-device
		[ "$found" -gt 0 ] && {
			[[ ${renamed} -eq 0 ]] && {
				config_foreach rename_iface_id_options wifi-iface wifi_id
				config_foreach recount_iface_wifi_id wifi-iface wifi_id
				uci commit
				renamed=1
			}

			wifi_id="$((wifi_id + 1))"
			continue
		}

		mode_band="g"
		channel="11"
		htmode=""
		ht_capab=""

		iw phy "$dev" info | grep -q 'Capabilities:' && htmode=HT20

		iw phy "$dev" info | grep -q '\* 5... MHz \[' && {
			mode_band="a"
			channel=$(iw phy "$dev" info | grep '\* 5... MHz \[' | grep '(disabled)' -v -m 1 | sed 's/[^[]*\[\|\].*//g')
			iw phy "$dev" info | grep -q 'VHT Capabilities' && htmode="VHT80"
		}

		iw phy "$dev" info | grep -q '\* 5.... MHz \[' && {
			mode_band="ad"
			channel=$(iw phy "$dev" info | grep '\* 5.... MHz \[' | grep '(disabled)' -v -m 1 | sed 's/[^[]*\[\|\|\].*//g')
			iw phy "$dev" info | grep -q 'Capabilities:' && htmode="HT20"
		}

		[ -n "$htmode" ] && ht_capab="set wireless.radio${devidx}.htmode=$htmode"

		path="$(mac80211_phy_to_path "$dev")"
		if [ -n "$path" ]; then
			dev_id="set wireless.radio${devidx}.path='$path'"
		else
			dev_id="set wireless.radio${devidx}.macaddr=$(cat /sys/class/ieee80211/${dev}/macaddress)"
		fi

		local router_mac=$(/sbin/mnf_info --mac 2>/dev/null)
		router_mac=$(printf "%X" $((0x$router_mac + $mac_add + $devidx)))
		if [ ${#router_mac} -lt 12 ]; then
			local zero_count=$(printf "%$((12 - ${#router_mac}))s")
			local zero_add=${zero_count// /0}
			router_mac=$zero_add$router_mac
		fi
		local wifi_mac=${router_mac:0:2}
		for i in 2 4 6 8 10; do
			wifi_mac=$wifi_mac:${router_mac:$i:2}
		done

		local default_pass=$(/sbin/mnf_info --wifi_pass 2>/dev/null)
		local wifi_auth_lines=""
		local router_mac_end=""

		if [ -n "$wifi_mac" ]; then
			router_mac_end=$(echo -n ${wifi_mac} | sed 's/\://g' | tail -c 4 | tr '[a-f]' '[A-F]')
			local dual_band_ssid=$(jsonfilter -i /etc/board.json -e '@.hwinfo.dual_band_ssid')
			local model=$(/sbin/mnf_info --name 2>/dev/null)
			if [ "$dual_band_ssid" != "true" ]; then
				ssid="${model:0:6}_${router_mac_end}"
			else
				if [ "$mode_band" = "g" ]; then
					ssid="${model:0:3}_${router_mac_end}_2G"
				elif [ "$mode_band" = "a"  ]; then
					ssid="${model:0:3}_${router_mac_end}_5G"
				fi
			fi
		fi

		IFS='' read -r -d '' wifi_auth_lines <<EOF
	set wireless.default_radio${devidx}.encryption=none
EOF

		[ -n "$default_pass" ] && [ ${#default_pass} -ge 8 ] && [ ${#default_pass} -le 64 ] && {
			IFS='' read -r -d '' wifi_auth_lines <<EOF
	set wireless.default_radio${devidx}.encryption=psk2+tkip+ccmp
	set wireless.default_radio${devidx}.key=${default_pass}
EOF
		}

		uci -q batch <<-EOF
			set wireless.radio${devidx}=wifi-device
			set wireless.radio${devidx}.type=mac80211
			set wireless.radio${devidx}.channel=${channel}
			set wireless.radio${devidx}.hwmode=11${mode_band}
			${dev_id}
			${ht_capab}
			set wireless.radio${devidx}.country=US
			#set wireless.radio${devidx}.disabled=1

			set wireless.default_radio${devidx}=wifi-iface
			set wireless.default_radio${devidx}.device=radio${devidx}
			set wireless.default_radio${devidx}.network=lan
			set wireless.default_radio${devidx}.mode=ap
			set wireless.default_radio${devidx}.ssid=${ssid}
			set wireless.default_radio${devidx}.wifi_id=wifi${wifi_id}
		${wifi_auth_lines}
EOF
		wifi_id="$((wifi_id + 1))"

		wps=$(jsonfilter -i /etc/board.json -e '@.hwinfo.wps')

		[ "$wps" = "true" -a "$mode_band" = "g" ] && {
			uci -q batch <<-EOF
				set wireless.default_radio${devidx}.wps_pushbutton=1
EOF
		}

		add_custom_wifi_iface "${devidx}"
		uci -q commit wireless

		devidx=$((devidx + 1))
	done
}
