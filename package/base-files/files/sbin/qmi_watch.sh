#!/bin/sh

while [ 1 ]; do
	sleep 10
	conn_status=`uqmi -s -d /dev/cdc-wdm0 --get-data-status`
	if [ "$conn_status" != "\"connected\"" ]; then
		echo "Exit: $conn_status"
		break
	fi
done
