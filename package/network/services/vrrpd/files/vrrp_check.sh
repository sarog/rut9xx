#!/bin/sh

. /lib/functions/network.sh

 CONFIG_GET="uci get vrrpd.${1}_ping"

 HOST=`$CONFIG_GET.host`
 WAIT=`$CONFIG_GET.time_out`
 P_SIZE=`$CONFIG_GET.packet_size`
 ENABLE=`$CONFIG_GET.enabled`
 COUNT=`$CONFIG_GET.ping_attempts`
 RETRY=`$CONFIG_GET.retry`
 INTERVAL=`$CONFIG_GET.interval`
 DEBUG=`$CONFIG_GET.debug`
 INTERFACE="$(uci get vrrpd.${1}.interface)"


 
 STATUS_FILE="/tmp/vrrp_${1}.status"

debug() {
	[ $DEBUG -eq 1 ] && logger "$1"
}

network_get_device DEVICE $INTERFACE

[ "$ENABLE" = 1 ] || exit 0

echo "ping failed" > "$STATUS_FILE"

while :; do
	RUNNING="$(ps |grep "vrrpd -i ${DEVICE}" |grep -v grep)"

	ping -c ${COUNT:-1} -W ${WAIT:-5} -s ${P_SIZE:-56} ${HOST:-8.8.8.8} > /dev/null 2>&1

	case $? in
		0)
			debug "Ping to ${HOST:-8.8.8.8} successful"
			FAIL_COUNTER=0
			[ -e "$STATUS_FILE" ] && rm "$STATUS_FILE"
			if [ -z "$RUNNING" ]
			then
				debug "Starting vrrpd"
				/etc/init.d/vrrpd start "$1"
			else
				debug "vrrpd is running"
			fi
			;;
		1)
			if [ -n "$RUNNING" ]; then
				failure=$FAIL_COUNTER
				failure=$(( failure + 1 ))
				FAIL_COUNTER=$failure
				debug "PING failed. Retry $failure of $RETRY"

				[ "$failure" -ge "${RETRY:-5}" ] && {
					echo "ping failed" > "$STATUS_FILE"

					logger "Killing vrrpd instance ($1) after $failure unsuccessful retries"
					/etc/init.d/vrrpd stop "$1"

					ubus call log write_ext "{
						\"event\": \"Stopping vrrp. We are now a backup router\",
						\"sender\": \"VRRP\",
						\"table\": 0,
						\"write_db\": 1,
					}"
				}
			else
				debug "vrrpd is stopped"
			fi

			;;
		*)
			debug "Unknown code $?"
			;;
	esac
	sleep ${INTERVAL:-2}
done

