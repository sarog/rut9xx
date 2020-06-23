#!/bin/sh

#
# Indicate reset button pressed time
#
# (c) 2014 Teltonika
FOREVER=0
SLEEP_TIME=1000000
LEDBAR="/usr/sbin/ledbar.sh"

[ ! -z $1 ] && SLEEP_TIME=$1
[ -n "$2" -a "$2" = "-f" ] && FOREVER=1

while [ $FOREVER -eq 1 -o -z "$i" ]
do
	$LEDBAR
	for i in 0 1 2 3 4
	do
		usleep $SLEEP_TIME
		$LEDBAR $i
	done
done

exit 0
