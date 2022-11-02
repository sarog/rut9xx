#!/bin/sh

. /lib/functions.sh

GPS_CONF="gps"
IOMAN_CONF="ioman"
AVL_CONF="avl"

GSM_TYPE="mobile"
GPIO_TYPE="gpio"
ANALOG_TYPE="adc"

handle_io_tavl() {
	local section=$1
	local type=$2
	local migrate_sec=$3

	config_get direction "$section" direction

	[ "$type" = "$GPIO_TYPE" -a -z "$direction" -o "$direction" = "out" ] && return

	local tavl_sec=$(uci add $GPS_CONF $migrate_sec)
	case $section in
		din1 | dio0)
			uci_set $GPS_CONF $tavl_sec "enabled" $send_digital_input1
		;;
		din2 | dio1)
			uci_set $GPS_CONF $tavl_sec "enabled" $send_digital_input2
		;;
		din3 | dio2 | iio)
			uci_set $GPS_CONF $tavl_sec "enabled" $send_digital_input3
		;;
		adc0)
			uci_set $GPS_CONF $tavl_sec "enabled" $send_analog_input
		;;
		*)
			uci_set $GPS_CONF $tavl_sec "enabled" 0
	esac
	uci_set $GPS_CONF $tavl_sec "type" $type
	uci_set $GPS_CONF $tavl_sec "name" $section
}

convert_tavl() {
	local section=$1
	local migrate_sec=$2

	config_get send_gsm_signal "$section" send_gsm_signal 0
	config_get send_digital_input1 "$section" send_digital_input1 0
	config_get send_digital_input2 "$section" send_digital_input2 0
	config_get send_digital_input3 "$section" send_digital_input3 0
	config_get send_analog_input "$section" send_analog_input 0

	local tavl_sec=$(uci add $GPS_CONF $migrate_sec)
	uci_set $GPS_CONF $tavl_sec "enabled" $send_gsm_signal
	uci_set $GPS_CONF $tavl_sec "type" $GSM_TYPE
	uci_set $GPS_CONF $tavl_sec "name" "signal"

	config_load $IOMAN_CONF || exit 0
	config_foreach handle_io_tavl $GPIO_TYPE $GPIO_TYPE $migrate_sec
	config_foreach handle_io_tavl $ANALOG_TYPE $ANALOG_TYPE $migrate_sec

	uci delete "$GPS_CONF"."$section"
}

move_avl_rules_to_avl_config() {
	local section=$1
	local section_name=$2

	if [ "$section" == "avl" ]; then
		config_get enabled "$section" enabled 0
		config_get hostname "$section" hostname ""
		config_get port "$section" port 0
		config_get proto "$section" proto "tcp"
		config_get con_cont "$section" con_cont 0
		config_get send_retry "$section" send_retry 0

		uci_set $AVL_CONF "avl" "enabled" $enabled
		uci_set $AVL_CONF "avl" "hostname" $hostname
		uci_set $AVL_CONF "avl" "port" $port
		uci_set $AVL_CONF "avl" "proto" $proto
		uci_set $AVL_CONF "avl" "con_cont" $con_cont
		uci_set $AVL_CONF "avl" "send_retry" $send_retry

		uci_remove $GPS_CONF $section

	elif [ "$section" == "avl_rule_main" ]; then
		config_get enabled "$section" enabled 0
		config_get priority "$section" priority "low"
		config_get distance "$section" distance 1
		config_get collect_period "$section" collect_period 1
		config_get angle "$section" angle 1
		config_get saved_records "$section" saved_records 1
		config_get send_period "$section" send_period 1

		uci_set $AVL_CONF "avl_rule_main" "enabled" $enabled
		uci_set $AVL_CONF "avl_rule_main" "priority" $priority
		uci_set $AVL_CONF "avl_rule_main" "distance" $distance
		uci_set $AVL_CONF "avl_rule_main" "collect_period" $collect_period
		uci_set $AVL_CONF "avl_rule_main" "angle" $angle
		uci_set $AVL_CONF "avl_rule_main" "saved_records" $saved_records
		uci_set $AVL_CONF "avl_rule_main" "send_period" $send_period

		uci_remove $GPS_CONF $section

	elif [ "$section_name" == "avl_rule" ]; then
		config_get enabled "$section" enabled 0
		config_get priority "$section" priority "low"
		config_get name "$section" name ""
		config_get distance "$section" distance 1
		config_get collect_period "$section" collect_period 1
		config_get angle "$section" angle 1
		config_get saved_records "$section" saved_records 1
		config_get send_period "$section" send_period 1
		config_get wan_status "$section" wan_status ""
		config_get din_status "$section" din_status ""
		config_get io_type "$section" io_type ""
		config_get io_name "$section" io_name ""

		local sec=$(uci add $AVL_CONF avl_rule)
		uci_set $AVL_CONF $sec "enabled" $enabled
		uci_set $AVL_CONF $sec "priority" $priority
		uci_set $AVL_CONF $sec "distance" $distance
		uci_set $AVL_CONF $sec "collect_period" $collect_period
		uci_set $AVL_CONF $sec "angle" $angle
		uci_set $AVL_CONF $sec "saved_records" $saved_records
		uci_set $AVL_CONF $sec "send_period" $send_period
		uci_set $AVL_CONF $sec "wan_status" $wan_status
		uci_set $AVL_CONF $sec "din_status" $din_status
		uci_set $AVL_CONF $sec "io_type" $io_type
		uci_set $AVL_CONF $sec "io_name" $io_name

		uci_remove $GPS_CONF $section

	elif [ "$section_name" == "tavl_rule" ]; then
		config_get enabled "$section" enabled 0
		config_get type "$section" type ""
		config_get name "$section" name ""

		local sec=$(uci add $AVL_CONF tavl_rule)
		uci_set $AVL_CONF $sec "enabled" $enabled
		uci_set $AVL_CONF $sec "type" $type
		uci_set $AVL_CONF $sec "name" $name

		uci_remove $GPS_CONF $section

	elif [ "$section_name" == "input" ]; then
		config_get enabled "$section" enabled 0
		config_get priority "$section" priority "low"
		config_get event "$section" event ""
		config_get io_type "$section" io_type ""
		config_get io_name "$section" io_name ""
		config_get min "$section" min ""
		config_get max "$section" max ""

		local sec=$(uci add $AVL_CONF input)
		uci_set $AVL_CONF $sec "enabled" $enabled
		uci_set $AVL_CONF $sec "priority" $priority
		uci_set $AVL_CONF $sec "event" $event
		uci_set $AVL_CONF $sec "io_type" $io_type
		uci_set $AVL_CONF $sec "io_name" $io_name

		[ -n "$min" ] && uci_set $AVL_CONF $sec "min" $min
		[ -n "$max" ] && uci_set $AVL_CONF $sec "max" $max

		uci_remove $GPS_CONF $section
	fi
}

remove_from_avl_conf() {
	local sec=$1

	uci_remove $AVL_CONF $sec
}

retain_https_rules() {
	local HTTPS_SEC="https"

	config_get enabled "$HTTPS_SEC" enabled 0
	config_get hostname "$HTTPS_SEC" hostname
	config_get interval "$HTTPS_SEC" interval 0
	config_get delay "$HTTPS_SEC" delay 0

	if [ "$interval" != 0 ]; then
		uci_set $GPS_CONF "$HTTPS_SEC" "interval" $interval
	elif [ "$delay" != 0 ]; then
		uci_set $GPS_CONF "$HTTPS_SEC" "interval" $delay
		uci_remove $GPS_CONF "$HTTPS_SEC" delay
	fi

	uci_set $GPS_CONF "$HTTPS_SEC" "enabled" $enabled
	uci_set $GPS_CONF "$HTTPS_SEC" "hostname" $hostname
}

config_load $GPS_CONF || exit 0
config_foreach convert_tavl "tavl" tavl_rule
config_load $GPS_CONF || exit 0
config_foreach convert_tavl "https_tavl" https_tavl_rule
uci_commit $GPS_CONF

config_load $GPS_CONF || exit 0
config_foreach move_avl_rules_to_avl_config "section"

if uci -q show "$GPS_CONF"."@avl_rule[0]"; then
	config_load $AVL_CONF || exit 0
	config_foreach remove_from_avl_conf "avl_rule"
	config_load $GPS_CONF || exit 0
	config_foreach move_avl_rules_to_avl_config "avl_rule" "avl_rule"
fi

if uci -q show "$GPS_CONF"."@tavl_rule[0]"; then
	config_load $AVL_CONF || exit 0
	config_foreach remove_from_avl_conf "tavl_rule"
	config_load $GPS_CONF || exit 0
	config_foreach move_avl_rules_to_avl_config "tavl_rule" "tavl_rule"
fi

if uci -q show "$GPS_CONF"."@input[0]"; then
	config_load $AVL_CONF || exit 0
	config_foreach remove_from_avl_conf "input"
	config_load $GPS_CONF || exit 0
	config_foreach move_avl_rules_to_avl_config "input" "input"
fi

if ! uci -q show "$AVL_CONF".avl.static_navigation; then
    uci set "$AVL_CONF".avl.static_navigation='0'
fi

retain_https_rules

uci_commit $GPS_CONF
uci_commit $AVL_CONF

exit 0
