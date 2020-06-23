#!/bin/sh

ENABLED=`uci get -q usb_to_serial.rs232.enabled`
DEV_PATH_HOLDER="/tmp/USB_to_serial_dev_path"
if [ -f "$DEV_PATH_HOLDER" ]; then
        USB_DEV="/dev/$(cat $DEV_PATH_HOLDER)"
        if [ -c "$USB_DEV" ]; then
                logger "USB character special file has been found"
                if [ "$ENABLED" = "1" ]; then
                    logger "USB to serial service is running, USB device has been changed, restarting service"
                    `/etc/init.d/usb_to_serial restart`
                fi
        else
                logger "USB device defined in $DEV_PATH_HOLDER not found, stoping the service"
                `/etc/init.d/usb_to_serial stop`
        fi
fi