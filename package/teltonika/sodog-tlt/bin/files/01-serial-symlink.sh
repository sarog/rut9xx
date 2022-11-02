#!/bin/sh

# handles RUTX, RUTXR1, RUT3, RUT9, TRB2 and TRB1
# symlinks serial chips regardless of vendor and driver - differentiates by DEVPATH

[ "$SUBSYSTEM" != "tty" ] || \
[ -n "$DEVPATH" -a -z "${DEVPATH##*virtual*}" ] || \
[ "$ACTION" != "remove" -a "$ACTION" != "add" ] && return 0

rs485="/dev/rs485"
rs232="/dev/rs232"
rs232_usb="/dev/rs232_usb_"
tty_dev="/dev/$DEVICENAME"
mbus="/dev/mbus"

err() {
	logger -s -p 3 -t "$(basename $script)" "$@" 2> /dev/console
	exit 1
}

warn() {
	logger -p 4 -t "$(basename $script)" "$@"
}

is_modem() {
	local path_ids="/lib/network/wwan"
	[ -f "$path_ids/$2:$1" ]
}

handle_multi_symlink() {
	case "$ACTION" in
		add)
			local path="/sys$DEVPATH/../../../../"
			is_modem "$(cat ${path}idProduct)" "$(cat ${path}idVendor)" && return
			# attempts to create a unique symlink for each converter
			local descriptors="${path}descriptors"
			[ -e "$descriptors" ] || err "${DEVICENAME}: descriptors file not found"
			local strings="$(cat ${path}version ${path}serial ${path}manufacturer ${path}product 2>/dev/null)"

			local csum="$( (echo -ne "$strings"; cat "$descriptors") | sha256sum)"
			[ -e "$rs232_usb${csum:0:8}" ] && {
				warn "an identical converter is already plugged in" # so let's include the path
				csum="$( (echo -ne "$strings${DEVPATH%%/tty*}"; cat $descriptors) | sha256sum)"
			}

			ln -s "$tty_dev" "$rs232_usb${csum:0:8}" # /dev/rs232_usb_7e97db3b
		;;
		remove)
			for f in $rs232_usb*; do
				[ -h "$f" -a "$(readlink $f)" = "$tty_dev" ] || continue

				rm "$f"
				break
			done
		;;
	esac
}

handle_symlink() {
	local path="/sys$DEVPATH/../../../../"
	is_modem "$(cat ${path}idProduct)" "$(cat ${path}idVendor)" && return

	case "$ACTION" in
		add)
			ln -s "$tty_dev" "$1"
		;;
		remove)
			[ -h "$1" -a "$(readlink $1)" = "$tty_dev" ] && rm "$1"
		;;
	esac
}

case "$(mnf_info --name)" in
	RUTXR1*)
		case "$DEVPATH" in
			*/usb1/1-1/1-1.3/*)
				handle_symlink $rs232           # built-in chip
				/etc/init.d/rs232 reload
			;;
			*/usb1/1-1/1-1.2/*)
				handle_multi_symlink $rs232_usb # usb-to-serial adapter
				/etc/init.d/rs232_usb reload
			;;
		esac
	;;
	RUTX14*)
		case "$DEVPATH" in
			*/usb3/3-1/*)
				handle_multi_symlink $rs232_usb # usb-to-serial adapter
				/etc/init.d/rs232_usb reload
			;;
		esac
	;;
	RUTX*)
		case "$DEVPATH" in
			*/usb1/1-1/*)
				handle_multi_symlink $rs232_usb # usb-to-serial adapter
				/etc/init.d/rs232_usb reload
			;;
		esac
	;;
	RUT955*|RUT956*)
		case "$DEVPATH" in
			*/usb1/1-1/1-1.3/*)
				handle_symlink $rs232     # built-in chip
				/etc/init.d/rs232 reload
			;;
			*/usb1/1-1/1-1.1/*)
				handle_multi_symlink $rs232_usb # usb-to-serial adapter
				/etc/init.d/rs232_usb reload
			;;
		esac
	;;
	RUT30*)
		case "$DEVPATH" in
			*/usb1/1-1/*)
				handle_multi_symlink $rs232_usb # usb-to-serial adapter
				/etc/init.d/rs232_usb reload
			;;
		esac
	;;
	RUT9*)
		return 0
	;;
	TRB2*)
		case "$DEVPATH" in
			*/usb1/1-1/1-1.2/*)
				handle_symlink $rs485 # built-in chip
				/etc/init.d/rs485 reload
			;;
			*/usb1/1-1/1-1.3/*)
				handle_symlink $rs232 # built-in chip
				/etc/init.d/rs232 reload
			;;
		esac
	;;
	TRB145*)
		[ "$DEVNAME" = "ttyHS0" ] && {
			handle_symlink $rs485
			/etc/init.d/rs485 reload
		}
	;;
	TRB142*)
		[ "$DEVNAME" = "ttyHS0" ] && {
			handle_symlink $rs232
			/etc/init.d/rs232 reload
		}
	;;
	TRB143*)
		[ "$DEVNAME" = "ttyHS0" ] && {
			handle_symlink $mbus
		}
	;;
	TRB1*)
		return 0
	;;
	*)
        return 0
	;;
esac
