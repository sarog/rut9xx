#!/bin/sh

#Check and encrypt password

pass=$(uci -q get sms_gateway.post_get.password)
[ -n "$pass" ] && [ "${pass:0:3}" != "\$1\$" ] && {
	salt=$(openssl rand -base64 6)
	[ -n "$salt" ] || return 0

	passwd=$(echo -n ${pass} | openssl passwd -1 -stdin -salt ${salt})
	[[ -n "$passwd" ]] && {
		uci	-q set sms_gateway.post_get.password="$passwd"
		uci -q commit sms_gateway
	}
}
