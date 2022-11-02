#!/bin/sh

[ "$DEVTYPE" = usb_device -a "${DEVICENAME%%:*}" = "$DEVICENAME" ] && [ -e /var/run/boot-done ] || exit 0

[ "$ACTION" = add ] && {
	/etc/init.d/gpsd reload &
}

exit 0
