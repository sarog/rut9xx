#!/bin/sh

if [ $# -ne 1 ]; then
	echo "Usage: $0 password"
	exit 1
fi

pass=$1
root_shadow=$(cat /etc/shadow | grep root | cut -d ':' -f2)
salt=$(echo ${root_shadow} | cut -d '$' -f 3)
check_shadow=$(echo -n ${pass} | openssl passwd -1 -stdin -salt ${salt})

if [ "$check_shadow" = "$root_shadow" ]; then
        exit 0
else
        exit 1
fi
