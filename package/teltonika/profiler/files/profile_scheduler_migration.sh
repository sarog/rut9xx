#!/bin/sh

. /lib/functions.sh

DEFAULT_PID=0

get_day(){
	[[ -n "$1" ]] && \
		echo $(echo 'mon tue wed thu fri sat sun' | cut -d ' ' -f$1) || \
		echo "mon"

}

set_start_time(){
	local wday=$1 hour=$2 pid=$3 enbaled=$4

	[[ "$wday" -eq 7 ]] && wday=0
	[[ "$hour" -lt 10 ]] && hour="0$hour"

	uci -q add profiles scheduler
	uci -q set profiles.@scheduler[-1].enabled="$enbaled"
	uci -q set profiles.@scheduler[-1].profile_id="$pid"
	uci -q set profiles.@scheduler[-1].start_day="$wday"
	uci -q set profiles.@scheduler[-1].start_time="$hour:00"
	uci -q set profiles.@scheduler[-1].period="week"
}

set_stop_time(){
	local wday=$1 hour=$2

	[[ "$wday" -eq 7 ]] && wday=0
	[[ "$hour" -lt 10 ]] && hour="0$hour"

	uci -q set profiles.@scheduler[-1].end_day="$wday"
	uci -q set profiles.@scheduler[-1].end_time="$hour:00"

}

convert_scheduler(){
	local section=$1
	local enabled days hours
	local n=1 i=0 start=0 old_pid="$DEFAULT_PID"

	config_get enabled "$section" scheduler 0
	[ "$enabled" -eq 0 ] && [ "$general_enabled" -eq 1 ] && enabled=1
	config_get days "$section" days
	[[ -z "$days" ]] && {
		uci -q delete profiles."$section"
		uci -q commit
		return 0
	}

	for day in ${days}; do
		hours=$(echo "$day" | grep -o .)
		hour=0

		for pid in ${hours}; do
			if [[ "$start" -ne 1 ]]; then
				[[ "$pid" -ne "$old_pid" && "$pid" -ne "$DEFAULT_PID" ]] && {
					start=1
					old_pid="$pid"
					set_start_time "$n" "$hour" "$pid" "$enabled"
				}
			else
				[[ "$pid" -ne "$old_pid" ]] && {
					start=0

					set_stop_time "$n" "$hour"
					[[ "$pid" -ne "$DEFAULT_PID" ]] && {
						start=1
						set_start_time "$n" "$hour" "$pid" "$enabled"
					}
				}
			fi

			old_pid="$pid"
			hour=$((hour+1))
		done

		n=$((n+1))
	done

	[[ "$start" -eq 1 ]] && {
		start=0
		[[ "$n" -lt 7 ]] && wday=$((n+1)) || wday=1
		set_stop_time "$wday" 0
	}

	uci -q set profiles.general.enabled="$enabled"
	uci -q delete profiles."$section"
	uci -q commit
}

config_load profiles
config_get general_enabled general scheduler 0
convert_scheduler "scheduler"
