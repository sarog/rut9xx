#!/bin/sh

. /lib/functions/teltonika-functions.sh

is_ios_enabled || exit 0

#Check and encrypt password

pass=$(uci -q get ioman.post_get.password)
[ -n "$pass" ] && [ "${pass:0:3}" != "\$1\$" ] && {
	salt=$(openssl rand -base64 6)
	[ -n "$salt" ] || return 0

	passwd=$(echo -n ${pass} | openssl passwd -1 -stdin -salt ${salt})
	[[ -n "$passwd" ]] && {
		uci	-q set ioman.post_get.password="$passwd"
		uci -q commit ioman
	}
}
