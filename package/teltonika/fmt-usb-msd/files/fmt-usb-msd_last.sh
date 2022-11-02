if [ -n "$DEVPATH" -a -z "${DEVPATH##*usb*}" -a "$ACTION" = "add" -a "$DEVTYPE" = "disk" -a -n "$DEVNAME" ]; then
	echo "/dev/$DEVNAME" > /tmp/.fmt-usb-msd_last
elif [ -n "$DEVPATH" -a -z "${DEVPATH##*mmc*}" -a "$ACTION" = "add" -a "$DEVTYPE" = "disk" -a -n "$DEVNAME" ]; then
	echo "/dev/$DEVNAME" > /tmp/.fmt-mmc-msd_last
fi
