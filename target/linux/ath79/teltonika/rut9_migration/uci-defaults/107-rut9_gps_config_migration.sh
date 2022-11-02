#!/bin/sh
 
. /lib/functions/migrate.sh

[ -f "/etc/config/teltonika" ] || return 0

fix_input() {
	local section=$1

	config_get input "$section" input

	case $input in
		digital1)
			uci_set gps $section "io_type" "gpio"
			uci_set gps $section "io_name" "din1"
		;;
		digital2)
			uci_set gps $section "io_type" "gpio"
			uci_set gps $section "io_name" "din2"
		;;
		digital3)
			uci_set gps $section "io_type" "gpio"
			uci_set gps $section "io_name" "iio"
		;;
		analog)
			uci_set gps $section "io_type" "adc"
			uci_set gps $section "io_name" "adc0"
		;;
	esac

	uci delete gps."$section".input
}

config_load gps
config_foreach fix_input input

uci_commit gps
