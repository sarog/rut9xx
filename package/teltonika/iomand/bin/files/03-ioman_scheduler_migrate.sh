#!/bin/sh

. /lib/functions.sh
. /lib/functions/teltonika-functions.sh

is_ios_enabled || exit 0
	
DEFAULT_STATE=0
ENABLED=0

get_day() {
	[ -n "$1" ] && \
		echo $(echo 'mon tue wed thu fri sat sun' | cut -d ' ' -f$1) || \
		echo "mon"
}

set_start_time() {
	local wday=$1 hour=$2

	[ "$wday" -eq 7 ] && wday=0
	[ "$hour" -lt 10 ] && hour="0$hour"

	uci -q set ioman.@scheduler[-1].start_day="$wday"
	uci -q set ioman.@scheduler[-1].start_time="$hour:00"
}

set_stop_time() {
	local wday=$1 hour=$2

	[ "$wday" -eq 7 ] && wday=0
	[ "$hour" -lt 10 ] && hour="0$hour"

	uci -q set ioman.@scheduler[-1].end_day="$wday"
	uci -q set ioman.@scheduler[-1].end_time="$hour:00"
}

init_new_section() {
	local enabled=$1 pin=$2

	uci -q add ioman scheduler
	uci -q set ioman.@scheduler[-1].enabled="$enabled"
	uci -q set ioman.@scheduler[-1].pin="$pin"
	uci -q set ioman.@scheduler[-1].period="week"
}

convert_scheduler() {
	local section=$1
	local enabled days hours pin
	local n=1 i=0 start=0 old_state="$DEFAULT_STATE" skip=0

	config_get enabled "$section" enabled 1
	config_get pin "$section" pin ""
	config_get days "$section" days
	[ -z "$days" ] && return 0
	[ "$enabled" -eq 1 ] && ENABLED=1

	#check if we have "from sunday to monday" case
	[ "${days:1:1}" != "$DEFAULT_STATE" ] && \
		[ "${days: -1}" != "$DEFAULT_STATE" ] && skip=1

	for day in ${days}; do
		hours=$(echo "$day" | grep -o .)
		hour=0

		for state in ${hours}; do
			[ "$skip" -eq 1 ] && {
				[ "$state" -eq "$DEFAULT_STATE" ] && skip=0 || continue
			}

			if [ "$start" -ne 1 ]; then
				[ "$state" -ne "$old_state" -a "$state" -ne "$DEFAULT_STATE" ] && {
					start=1 old_state="$state"

					init_new_section "$enabled" "$pin"
					set_start_time "$n" "$hour"
				}
			else
				[ "$state" -ne "$old_state" ] && {
					start=0

					set_stop_time "$n" "$hour"
				}
			fi

			old_state="$state"
			hour=$((hour+1))
		done

		n=$((n+1))
	done

	[ "$start" -eq 1 ] && {
		n=1
		for day in ${days}; do
			hours=$(echo "$day" | grep -o .)
			hour=0

			for state in ${hours}; do
				[ "$state" -ne "$old_state" ] && {
					set_stop_time "$n" "$hour"
					uci -q delete ioman."$section"
					return
				}

				hour=$((hour+1))
			done

			n=$((n+1))
		done
	}

	uci -q delete ioman."$section"
}

config_load "ioman" || exit 0
config_foreach convert_scheduler "scheduler"  "$is_old"

has_general=$(uci -q get ioman.scheduler_general)
[ -z "$has_general" ] && uci -q set ioman.scheduler_general=scheduler_general
[ "$ENABLED" -eq 1 ] && uci -q set ioman.scheduler_general.enabled=1

uci -q commit

exit 0
