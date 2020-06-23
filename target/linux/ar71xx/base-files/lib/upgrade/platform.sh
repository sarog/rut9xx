#
# Copyright (C) 2011 OpenWrt.org
#

. /lib/ar71xx.sh

PART_NAME=${PART_NAME:-"firmware"}
RAMFS_COPY_DATA=/lib/ar71xx.sh

CI_BLKSZ=65536
CI_LDADR=0x80060000

tplink_get_image_hwid() {
	get_image "$@" | dd bs=4 count=1 skip=16 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

tplink_get_image_boot_size() {
	get_image "$@" | dd bs=4 count=1 skip=37 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

platform_check_image() {
	local board=$(ar71xx_board_name)
	local magic="$(get_magic_word "$1")"
	local magic_long="$(get_magic_long "$1")"

	[ "$ARGC" -gt 2 ] && return 1

	case "$board" in
	tlt-rut900)
		[ "$magic" != "0100" ] && {
			echo "Invalid image type."
			return 1
		}

		local hwid
		local imageid

		hwid=$(tplink_get_hwid $PART_NAME)
		imageid=$(tplink_get_image_hwid "$1")

		[ "$hwid" != "$imageid" ] && {
			echo "Invalid image, hardware ID mismatch, hw:$hwid image:$imageid."
			return 1
		}

		local boot_size

		boot_size=$(tplink_get_image_boot_size "$1")
		[ "$boot_size" != "00000000" ] && {
			echo "Invalid image, it contains a bootloader."
			return 1
		}

		#~ Check modem and allow to flash new firmware from common sources
		local modem_mnf="$(gsmctl -w)"

		[ "$modem_mnf" = "Quectel" ] && {
			. /usr/share/libubox/jshn.sh

			fwtool -q -i /tmp/sysupgrade.meta "$1"
			[ -s /tmp/sysupgrade.meta ] && {
				json_load "$(cat /tmp/sysupgrade.meta)"
				json_select supported_devices
				json_get_keys dev_keys
				for k in $dev_keys; do
					json_get_var dev "$k"
					[ "$dev" = "rut900" ] && return 0
				done
			}
		}

		local fw_revision=$(dd if="$1" bs=1 skip=28 count=6 2>/dev/null)

		[ "$fw_revision" != "RUT9xx" -a "$fw_revision" != "r40569" ] && {
			echo "Invalid image, not supported on this device."
			return 1
		}

		return 0
		;;
	esac

	echo "Sysupgrade is not yet supported on $board."
	return 1
}

platform_do_upgrade() {
	default_do_upgrade "$ARGV"
}

disable_watchdog() {
	killall watchdog
	( ps | grep -v 'grep' | grep '/dev/watchdog' ) && {
		echo 'Could not disable watchdog'
		return 1
	}
}

append sysupgrade_pre_upgrade disable_watchdog
