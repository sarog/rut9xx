#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

move_device() {
	uci_set wireless "$1" path platform/ahb/18100000.wmac
	config_get hwmode "$1" hwmode
	[ "$hwmode" = "11b" ] && {
		uci set wireless."$1".hwmode="11g"
		uci set wireless."$1".legacy_rates="1"
	}

	config_get country "$1" country
	[ "$country" = "00" ] && uci_set wireless "$1" country "US"

	config_get htmode "$1" htmode
	uci set wireless."$1".htmode="${htmode%-*}"
}

move_iface() {
	config_get hotspotid "$1" hotspotid 0
	config_get mode "$1" mode 0

	uci delete wireless."$1".user_enable
	uci delete wireless."$1".ttl_increase
	[ "$hotspotid" -eq 0 ] || {
		hotspotid=${hotspotid:1,7}
		uci set wireless."$1".wifi_id="wifi$hotspotid"
	}
	uci delete wireless."$1".hotspotid

	uci delete wireless."$1".scan_sleep
	uci delete wireless."$1".signal_threshold
	uci delete wireless."$1".bgscan
	uci delete wireless."$1".signal_thresh
	uci delete wireless."$1".short_interval
	uci delete wireless."$1".long_interval

	[ "$mode" = "sta" ] && uci set wireless."$1".network=wwan
}

uci del wireless.radio1
uci del wireless.default_radio1

config_load wireless
config_foreach move_device wifi-device
config_foreach move_iface wifi-iface
uci commit
