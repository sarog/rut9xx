#!/bin/sh

#~ Script for mobile connection tracking
#~ Exit when connection lost
device="$1"
drop_timer=0
drop_timer_limit=45 #15 minutes 
shift

check_state() {
	local serv_status="$1"
	case "$serv_status" in
		*'"registration":"registered"'*'"PS":"attached"'*)
			conn_state=1
			;;
		*)
			conn_state=0
			if [ "$drop_timer" = "0" ]; then
				logger -t "qmux_track" "Connection lost."
			fi
			;;
	esac
}

while true; do
	for cid in $@; do
		connstat=$(uqmi -s -d $device -t 3000 --set-client-id wds,"$cid" --get-data-status | awk -F '"' '{print $2}')
		if [ "$connstat" != "connected" ]; then
			echo "Mobile connection lost!"
			exit 1
		fi
		serv_status=$(uqmi -s -d $device -t 3000 --set-client-id wds,"$cid" --get-serving-system)
		check_state "$serv_status"
		# If any of them are zero consider connection temporarily lost
		if [ "$conn_state" != 1 ]; then
			drop_timer=$((drop_timer + 1));
		else
			drop_timer=0
		fi

		#if drop timer limit passes limit consider connection lost.
		if [ "$drop_timer" = "$drop_timer_limit" ]; then
			echo "Mobile connection lost!"
			exit 1
		fi
	done
	for iface in $IFACE4 $IFACE6; do
		ifstatus "$iface" 2>&1 >/dev/null || exit 1
	done

	sleep 20
done
