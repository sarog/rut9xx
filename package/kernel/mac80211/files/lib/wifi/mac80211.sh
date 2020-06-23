#!/bin/sh
append DRIVERS "mac80211"

lookup_phy() {
	[ -n "$phy" ] && {
		[ -d /sys/class/ieee80211/$phy ] && return
	}

	local devpath
	config_get devpath "$device" path
	[ -n "$devpath" ] && {
		for phy in $(ls /sys/class/ieee80211 2>/dev/null); do
			case "$(readlink -f /sys/class/ieee80211/$phy/device)" in
				*$devpath) return;;
			esac
		done
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

detect_mac80211() {
	local ssid;
	devidx=0
	config_load wireless
	while :; do
		config_get type "radio$devidx" type
		[ -n "$type" ] || break
		devidx=$(($devidx + 1))
	done

	for _dev in /sys/class/ieee80211/*; do
		[ -e "$_dev" ] || continue

		dev="${_dev##*/}"

		found=0
		config_foreach check_mac80211_device wifi-device
		[ "$found" -gt 0 ] && continue

		mode_band="ng"
		channel="auto"
		htmode=""
		ht_capab=""

		iw phy "$dev" info | grep -q 'Capabilities:' && htmode=HT20
		iw phy "$dev" info | grep -q '2412 MHz' || { mode_band="a"; channel="36"; }

		vht_cap=$(iw phy "$dev" info | grep -c 'VHT Capabilities')
		[ "$vht_cap" -gt 0 ] && {
			mode_band="a";
			channel="36"
			htmode="VHT80"
		}

		[ -n $htmode ] && append ht_capab "	option htmode	$htmode" "$N"

		if [ -x /usr/bin/readlink ]; then
			path="$(readlink -f /sys/class/ieee80211/${dev}/device)"
			path="${path##/sys/devices/}"
			dev_id="	option path	'$path'"
		else
			dev_id="	option macaddr	$(cat /sys/class/ieee80211/${dev}/macaddress)"
		fi

		if [ `which brand` ]; then
			ssid=`brand 21`
		else
			ssid="Teltonika_Router"
		fi

		local router_name=$(/sbin/mnf_info name 2>/dev/null)
		local router_mac=$(cat /sys/class/ieee80211/${dev}/macaddress)
		local default_pass=$(/sbin/mnf_info wifi_pass 2>/dev/null)
		local wifi_auth_lines=""

		if [ -n "$router_name" ] && [ -n "$router_mac" ]; then
			router_name=$(echo -n ${router_name} | head -c 6)
			router_mac=$(echo -n ${router_mac} | sed 's/\://g' | tail -c 4 | tr '[a-f]' '[A-F]')
			ssid="${router_name}_${router_mac}"
		fi

		IFS='' read -r -d '' wifi_auth_lines <<EOF
	option encryption none
EOF

		[ -n "$default_pass" ] && [ ${#default_pass} -ge 8 ] && [ ${#default_pass} -le 64 ] && {
			IFS='' read -r -d '' wifi_auth_lines <<EOF
	option encryption 'psk2+tkip+ccmp'
	option key '${default_pass}'
EOF
		}

		cat <<EOF
config wifi-device	radio$devidx
	option type	mac80211
	option channel	${channel}
	option hwmode	11${mode_11n}${mode_band}
	option country	'00'
$dev_id
	list ht_capab	LDPC
	list ht_capab	SHORT-GI-20
	list ht_capab	SHORT-GI-40
	list ht_capab	TX-STBC
	list ht_capab	RX-STBC1
	list ht_capab	DSSS_CCK-40
$ht_capab


config wifi-iface
	option device	radio$devidx
	option network	lan
	option mode	ap
	option ssid	${ssid}
	option isolate	0
	option user_enable '1'
	option hotspotid 'hotspot1'
${wifi_auth_lines}
EOF
	devidx=$(($devidx + 1))
	done
}
