#!/bin/sh
DEBUG_LEVEL="0"
debug() {
	if [ "$DEBUG_LEVEL" == "1" ]; then
		logger -t check_wget_reboot.sh "$1"
	fi
}

exist=`pidof wget_reboot.sh`

if [ "$exist" != "" ]; then
	debug "Aplication already running. Exiting..."
	exit 1
else
	debug "Starting wget_reboot.sh"
	/usr/sbin/wget_reboot.sh &
fi
