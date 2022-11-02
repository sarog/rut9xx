#!/bin/sh

#Script must be executed before 99-ioman_scheduler_migrate.sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

SCHEDULER_CONF="/etc/scheduler/config"
DEFAULT_PIN=0
ALL_PINS="dout1"

translate_pin() {
	local pin_num="$2"
	local pin

	case "$pin_num" in
		1)
			pin="dout1"
			;;
	esac

	eval export "${1}=\${pin}"
}

set_start_time() {
	local wday=$1 hour=$2 pins="$3"
	local section

	[ "$wday" -eq 7 ] && wday=0
	[ "$hour" -lt 10 ] && hour="0$hour"

	for pin in $pins; do
		eval section=\$IOSECTION_"$pin"
		uci -q set ioman."$section".start_day="$wday"
		uci -q set ioman."$section".start_time="$hour":00
	done
}

set_stop_time() {
	local wday=$1 hour=$2 skip=$3
	local section pin

	[ "$wday" -eq 7 ] && wday=0
	[ "$hour" -lt 10 ] && hour="0$hour"

	for pin in $ALL_PINS; do
		[ "$skip" = "$pin" ] && continue

		eval section=\$IOSECTION_"$pin"
		[ -z "$section" ] && continue
		uci -q set ioman."$section".end_day="$wday"
		uci -q set ioman."$section".end_time="$hour":00
		unset "IOSECTION_${pin}"
	done
}

init_new_section() {
	local enabled=$1 pins="$2"
	local pin section

	for pin in $pins; do
		eval section=\$IOSECTION_"$pin"
		[ -n "$section" ] && continue

		uci_add ioman scheduler
		uci -q set ioman."$CONFIG_SECTION".enabled="$enabled"
		uci -q set ioman."$CONFIG_SECTION".pin="$pin"
		uci -q set ioman."$CONFIG_SECTION".period=week

		export "IOSECTION_${pin}=${CONFIG_SECTION}"
	done
}

init_new_scheduler() {
	local pin="$1" wday="$2" hour="$3" enabled="$4"
	local tmp_pin

	translate_pin tmp_pin "$pin"
	init_new_section "$enabled" "$tmp_pin"
	set_start_time "$wday" "$hour" "$tmp_pin"
}

convert_scheduler() {
	local enabled days hours pin
	local n=1 start=0 old_pin="$DEFAULT_PIN" skip=0

	config_get enabled scheduler enabled 1

	days=$(cat "$SCHEDULER_CONF" 2>/dev/null | sed 's/ //g')
	[ -z "$days" ] && return 0
	uci_set ioman "scheduler_general" enabled "$enabled"

	#check if we have "from sunday to monday" case
	[ "${days:4:1}" != "$DEFAULT_PIN" ] && [ "${days:4:1}" = "${days: -1}" ] &&\
		[ "${days: -1}" != "$DEFAULT_PIN" ] && skip=1

	for day in ${days}; do
		hours=$(echo "${day:4}" | grep -o .)
		hour=0

		for pin in ${hours}; do
			[ "$skip" -eq 1 ] && {
				[ "$pin" -eq "$DEFAULT_PIN" ] && skip=0 || continue
			}

			if [ "$start" -ne 1 ]; then
				[ "$pin" -ne "$old_pin" ] && [ "$pin" -ne "$DEFAULT_PIN" ] && {
					start=1

					init_new_scheduler "$pin" "$n" \
						"$hour" "$enabled"
				}
			else
				[ "$pin" -ne "$old_pin" ] && {
					start=0

					translate_pin tmp_pin "$pin"
					set_stop_time "$n" "$hour" "$tmp_pin"

					[ "$pin" -ne "$DEFAULT_PIN" ] && {
						start=1

						init_new_scheduler "$pin" "$n" \
							"$hour" "$enabled"
					}
				}
			fi

			old_pin="$pin"
			hour=$((hour+1))
		done

		n=$((n+1))
	done

	#set stop time if have "from sunday to monday" case
	[ "$start" -eq 1 ] && {
		n=1
		for day in ${days}; do
			hours=$(echo "${day:4}" | grep -o .)
			hour=0

			for pin in ${hours}; do
				[ "$pin" -ne "$old_pin" ] && {
					set_stop_time "$n" "$hour"
					return
				}

				hour=$((hour+1))
			done

			n=$((n+1))
		done
	}
}

migrate_post_get() {
	local enabled username password

	config_get enabled post_get enabled 0
	config_get username post_get username
	config_get password post_get password

	uci_add ioman post_get post_get
	uci_set ioman post_get enabled "$enabled"
	uci_set ioman post_get username "$username"
	uci_set ioman post_get password "$password"
}

sed -i "/gpio.sh/d" /etc/crontabs/root

config_load output_control

convert_scheduler
migrate_post_get

uci_commit ioman
rm /etc/config/output_control
rm "$SCHEDULER_CONF"
