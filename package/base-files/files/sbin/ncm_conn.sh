#!/bin/sh

WHILE_INTERVAL=5
ifname="$1"
pdp="$2"

init() {
	[ -f /var/run/ncm_conn.pid ] && exit 0
	echo "$$" > /var/run/ncm_conn.pid
}

check_connection() {
	local retry=7
	local count=0
	local parsed_pdp parsed_status

	while true; do
		conn=$(gsmctl -A 'AT+QNETDEVCTL?' 2>/dev/null)
		parsed_pdp="$(echo "$conn" | cut -f2 -d,)"
		parsed_status="$(echo "$conn" | cut -f4 -d,)"

		if [ "${conn::11}" = "+QNETDEVCTL" ] && [ "$parsed_pdp" -eq "$pdp" ] && [ "$parsed_status" -eq 1 ]; then
			count=0
		else
			count=$((count+1))
			echo FAIL
			[ "$count" -gt "$retry" ] && {
				echo "Connection was lost!"
				network_state=$(gsmctl -g 2>/dev/null)
				echo "Network state: $network_state. Trying to reconnect."
				exit 1
			}
		fi

		sleep "$WHILE_INTERVAL"
	done
}

echo "Starting connection tracker for ${ifname}!"
init
check_connection
