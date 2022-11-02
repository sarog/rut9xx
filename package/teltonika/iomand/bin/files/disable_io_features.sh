#!/bin/sh

. /lib/functions.sh
. /lib/functions/teltonika-functions.sh

sms_utils_cb() {
	config_get name $1 action
	[ "$name" = "io_set" -o "$name" = "iostatus" ] && {
		uci set sms_utils.$1.enabled=0
		do_commit=1
	}
}

is_ios_enabled || {
	/etc/init.d/ioman disable
	/etc/init.d/ioman_scheduler disable
	/etc/init.d/iojuggler disable
	rm /etc/config/iojuggler
	rm -f /etc/config/ioman

	config_load sms_utils
	config_foreach sms_utils_cb rule
	[ "$do_commit" = "1" ] && uci commit
}

exit 0
