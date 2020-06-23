#!/bin/sh

if [ $# -lt 3  ] || [ $# -gt 4 ]
then
	echo "Usage: $0 <IP address> <period (s)> <interface> [<single>]"
	exit
fi

address=$1
period=$2
iface=$3
single=$4

if [ -n "$single" ]
then
	/bin/ping -c 1 -w 1 -q -I "$iface" "$address" > /dev/null 2>&1
	exit 0
fi

while [ 1 ]
do
	/bin/sleep "$period"
	/bin/ping -c 1 -w 1 -q -I "$iface" "$address" > /dev/null 2>&1
done

