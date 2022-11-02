#!/bin/sh
(
	. /lib/functions.sh
	. /usr/share/libubox/jshn.sh

	get_iostatus_message() {
		# DO NOT CHANGE TO %gX. Different devices have different codes
		local msg_in="DIN - %di, Isolated DIN - %ii"
		local msg_in_pin="4Pin DIN - %pi"
		local msg_out="Analog input - %aiV, DOUT OC - %oc, Relay DOUT - %ro"
		local msg_out_pin="4Pin DOUT - %po"
		local msg_rut2="DIN - %di, DOUT - %oc"

		local name=$(mnf_info -n | cut -c -6)
		local four_pin ten_pin

		case "$name" in
		RUT9*)
			[ "$name" = "RUT955" ] || [ "$name" = "RUT956" ] && ten_pin=1
			four_pin=1
			IO_MAP=$IO_MAP_RUT9
			IOSTATUS_MESSAGE="${ten_pin:+"$msg_in"}${four_pin:+"${ten_pin:+", "}$msg_in_pin"}${ten_pin:+", $msg_out"}${four_pin:+", $msg_out_pin"}"
			;;
		RUT2*)
			IO_MAP=$IO_MAP_RUT2
			IOSTATUS_MESSAGE=$msg_rut2
			;;
		*)
			IO_MAP=$IO_MAP_RUTOS
			IOSTATUS_MESSAGE=""
			;;
		esac
	}

	migrate_io() {
		local action message io_var

		config_get action "$1" "action"
		[ "$action" = "iostatus" ] || return

		config_get message "$1" "message" "$IOSTATUS_MESSAGE"

		while which iomand && [ -z "$io_list" ]; do
			io_list=$(ubus list ioman.*)
			sleep 1
		done

		local n=0
		for io_ubus in $io_list; do
			json_get_var io_var "$io_ubus"
			[ -n "$io_var" ] && message=${message//%"$io_var"/%g"$n"}
			n=$((n + 1))
		done

		uci_set sms_utils "$1" message "$message"
		uci_commit
	}

	IO_MAP_RUTOS='{
	"ioman.acl.acl0": "cl",
	"ioman.adc.adc0": "ad",
	"ioman.dwi.dwi0": "dw",
	"ioman.dwi.dwi1": "wd",
	"ioman.gpio.dio0": "pi",
	"ioman.gpio.dio1": "po",
	"ioman.gpio.dio2": "pt",
	"ioman.gpio.iio": "ii",
	"ioman.gpio.din1": "di",
	"ioman.gpio.dout1": "do",
	"ioman.relay.relay0": "rb",
	"ioman.relay.relay1": "rl"
}'

	IO_MAP_RUT9='{
	"ioman.acl.acl0": "cl",
	"ioman.adc.adc0": "ai",
	"ioman.gpio.din1": "pi",
	"ioman.gpio.din2": "di",
	"ioman.gpio.dout2": "oc",
	"ioman.gpio.dout1": "po",
	"ioman.gpio.iio": "ii",
	"ioman.relay.relay0": "ro"
}'

	IO_MAP_RUT2='{
	"ioman.gpio.din1": "di",
	"ioman.gpio.dout1": "oc"
}'

	get_iostatus_message
	json_load "$IO_MAP"

	config_load "sms_utils"
	config_foreach migrate_io "rule"
) </dev/null >/dev/null 2>&1 &
# launch script in a subshell in background to not hang up the device if ioman is not yet available through ubus
