#!/bin/sh

# $1 - gpio (DOUT1 or DOUT2)
# $2 - action (set or clear)
# $3 - timeout

/sbin/gpio.sh $2 $1

if [ -n "$3" ]; then
	sleep $3
	if [ "$2" = "set" ]; then
		/sbin/gpio.sh clear $1
	else
		/sbin/gpio.sh set $1
	fi
fi
