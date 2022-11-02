#!/bin/sh

. /usr/share/libubox/jshn.sh

json_init
json_add_array "rs232_usb"
ls /dev/rs232_usb_* >/dev/null 2>/dev/null && for d in /dev/rs232_usb_*; do
	json_add_string "" "${d##*rs232_usb_}"
done
json_close_array
json_close_object

json_dump -i
