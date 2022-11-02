#!/bin/ash

. /lib/functions.sh
. /usr/share/libubox/jshn.sh

ucidef_add_static_modem_info() {
	#Parameters: model usb_id sim_count other_params
	local model usb_id count
	local modem_counter=0
	local sim_count=1

	model="$1"
	usb_id="$2"

	[ -n "$3" ] && sim_count="$3"

	json_get_keys count modems
	[ -n "$count" ] && modem_counter="$(echo "$count" | wc -w)"

	json_select_array "modems"
		json_add_object
			json_add_string id "$usb_id"
			json_add_string num "$((modem_counter + 1))"
			json_add_boolean builtin 1
			json_add_int simcount "$sim_count"

			for i in "$@"; do
				case "$i" in
					primary)
						json_add_boolean primary 1
						;;
					gps_out)
						json_add_boolean gps_out 1
					;;
				esac
			done

		json_close_object
	json_select ..
}

ucidef_add_serial_capabilities() {
	json_select_array serial
		json_add_object
			[ -n "$1" ] && {
				json_select_array devices
				for d in $1; do
					json_add_string "" $d
				done
				json_select ..
			}

			json_select_array bauds
			for b in $2; do
				json_add_string "" $b
			done
			json_select ..

			json_select_array data_bits
			for n in $3; do
				json_add_string "" $n
			done
			json_select ..
		json_close_object
	json_select ..
}

ucidef_set_hwinfo() {
	local function
	local dual_sim=0
	local wifi=0
	local dual_band_ssid=0
	local wps=0
	local mobile=0
	local gps=0
	local usb=0
	local bluetooth=0
	local ethernet=0
	local sfp_port=0
	local ios=0
	local sfp_switch=0
	local rs232=0
	local rs485=0
	local console=0

	for function in "$@"; do
		case "$function" in
			dual_sim)
				dual_sim=1
			;;
			wifi)
				wifi=1
			;;
			dual_band_ssid)
				dual_band_ssid=1
			;;
			wps)
				wps=1
			;;
			mobile)
				mobile=1
			;;
			gps)
				gps=1
			;;
			usb)
				usb=1
			;;
			bluetooth)
				bluetooth=1
			;;
			ethernet)
				ethernet=1
			;;
			sfp_port)
				sfp_port=1
			;;
			ios)
				ios=1
			;;
			at_sim)
				at_sim=1
			;;
			sfp_switch)
				sfp_switch=1
			;;
			rs232)
				rs232=1
			;;
			rs485)
				rs485=1
			;;
			console)
				console=1
			;;
		esac
	done

	json_select_object hwinfo
		json_add_boolean dual_sim "$dual_sim"
		json_add_boolean usb "$usb"
		json_add_boolean bluetooth "$bluetooth"
		json_add_boolean wifi "$wifi"
		json_add_boolean dual_band_ssid "$dual_band_ssid"
		json_add_boolean wps "$wps"
		json_add_boolean mobile "$mobile"
		json_add_boolean gps "$gps"
		json_add_boolean ethernet "$ethernet"
		json_add_boolean sfp_port "$sfp_port"
		json_add_boolean ios "$ios"
		json_add_boolean at_sim "$at_sim"
		json_add_boolean sfp_switch "$sfp_switch"
		json_add_boolean rs232 "$rs232"
		json_add_boolean rs485 "$rs485"
		json_add_boolean console "$console"
	json_select ..
}

ucidef_set_release_version() {
	json_add_string release_version "$1"
}
