#!/bin/sh

usage () {
	echo -e "\tFirmware validation application"
	echo -e "\tUsage for router: <FIRMWARE>"
	echo -e "\tUsage for server: <FIRMWARE> <RUT_NAME>"
	echo -e "\tAvailable RUT_NAME's:"
	echo -e "\t\t\t tlt-rut900"
	echo -e "\t\t\t tlt-rut200"
	echo -e "\t\t\t tlt-rut850"
}

fw_version_is_new(){

	local version_offset=158
	local current_fw_version=`cat /etc/version`
	if [ ! $current_fw_version ]; then
		echo 0
		return
	fi

	local img_fw_version=`dd if="$FIRMWARE" bs=1 skip="$version_offset" count=32 2>/dev/null`
	if [ ! $img_fw_version ]; then
		echo 0
		return
	fi

	#Client number check
	local current_client=`echo "$current_fw_version" | awk -F . '{ print $1 }'`
	current_client=`echo $current_client | awk -F _ '{ print $NF }'`
	local fw_client=`echo "$img_fw_version" | awk -F . '{ print $1 }'`
	fw_client=`echo $fw_client | awk -F _ '{ print $NF }'`

	if [ ! $current_client ] || [ ! $fw_client ]; then
		echo 0
		return
	elif [ "$current_client" != "$fw_client" ]; then
		echo 0
		return
	fi

	#Major version check
	local current_major=`echo "$current_fw_version" | awk -F . '{ print $2 }'`
	local fw_major=`echo "$img_fw_version" | awk -F . '{ print $2 }'`

	if [ ! $current_major ] || [ ! $fw_major ]; then
		echo 0
		return
	elif [ $current_major -gt  $fw_major ]; then
		echo 0
		return
	elif [ $fw_major -gt $current_major ]; then
		echo 1
		return
	fi

	#Minor version check
	local current_minor=`echo "$current_fw_version" | awk -F . '{ print $3 }'`
	current_minor=`echo "$current_minor" | awk -F _ '{ print $1 }'`
	local fw_minor=`echo "$img_fw_version" | awk -F . '{ print $3 }'`
	fw_minor=`echo "$fw_minor" | awk -F _ '{ print $1 }'`

	if [ ! $current_minor ] || [ ! $fw_minor ]; then
		echo 0
		return
	fi

	#fw_minor=$(($fw_minor%1000))

	if [ $fw_minor -lt  $current_minor ]; then
		echo 0
		return
	fi

	echo 1
	return
}

get_fw_size(){

	local size=`wc -c < $FIRMWARE`

	if [ ! $size ]; then
		echo 0
	fi

	echo "$size"
}
image_supported(){
	#local ret=$( . /etc/functions.sh; include /lib/upgrade; platform_check_image "$FIRMWARE" 2>/dev/null )

	local magic=`dd if="$FIRMWARE" bs=2 count=1 2>/dev/null | hexdump -v -n 2 -e '1/1 "%02x"'`
	[ "$magic" != "0100" ] && {
		echo "Invalid image type."
		return
	}

	local boot_size=`dd if="$FIRMWARE" bs=4 count=1 skip=37 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'`
	[ "$boot_size" != "00000000" ] && {
		echo "Invalid image, it contains a bootloader."
		return
	}

	if [ ! $NAME ]; then
		local imageid=`dd if="$FIRMWARE" bs=4 count=1 skip=16 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'`
		local tmp=`cat /proc/mtd | grep firmware | wc -l`
		if [ "$tmp" -eq  "1" ]; then
			local part=`cat /proc/mtd | grep firmware | awk -F: '{print $1}'`
			local hwid=`dd if=/dev/$part bs=4 count=1 skip=16 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'` # tikrinam RUT
		fi

		[ "$hwid" != "$imageid" ] && {
			echo "Invalid image, hardware ID mismatch, hw:$hwid image:$imageid."
			return
		}

	fi

	local fw_revision=`dd if="$FIRMWARE" bs=1 skip=28 count=6 2>/dev/null`
	if [ ! $NAME ]; then
		local board=$( . /lib/ar71xx.sh; ar71xx_board_name) # tikrinam RUT
	else
		local board=$NAME
	fi

	case "$board" in
		tlt-rut900)
			[ "$fw_revision" != "RUT9xx" -a "$fw_revision" != "r40569" ] && {
				echo "Invalid image, not supported on this device."
				return
			}
		;;
		tlt-rut200)
			[ "$fw_revision" != "RUT2xx" ] && {
				echo "Invalid image, not supported on this device."
				return
			}
		;;
		tlt-rut850)
			[ "$fw_revision" != "RUT850" ] && {
				echo "Invalid image, not supported on this device."
				return
			}
		;;
		*)
		echo "Board not found."
		return
		;;
	esac

	echo "1"
	return
}

image_checksum(){

	local FWNAME="${2:-master}"
	local validate="${3:-1}"

	local file_size=`ls -la $FIRMWARE 2>/dev/null | awk '{ print $5}' 2>/dev/null`

	local check_length=`expr $file_size - 16 2>/dev/null`
	local name_offset=`expr $file_size - 22 2>/dev/null`

	local md5_calculated=`head -c $check_length $FIRMWARE 2>/dev/null | md5sum 2>/dev/null | awk '{print $1}' 2>/dev/null`
	local md5_extracted=`hexdump -s $check_length -n 16 -e '16/1 "%02x"' $FIRMWARE 2>/dev/null`
	local md5_len=${#md5_calculated}
	local fw_name=`dd if=$FIRMWARE bs=1 skip=$name_offset count=6 2>/dev/null`

	local gigadevice_flash_validate=`dd if=$FIRMWARE bs=1 skip=63 count=1 2>/dev/null`
	local i2c_validate=`dd if=$FIRMWARE bs=1 skip=62 count=1 2>/dev/null`
	local xt_flash_validate=`dd if=$FIRMWARE bs=1 skip=61 count=1 2>/dev/null`



	if [ ! $NAME ]; then
		local serial_number=`mnf_info sn`
		local device_flash_id=$(cat /sys/bus/spi/devices/spi0.0/flash_name)

		# GigaDevice flash support (from RUT9XX_R_00.03.491)
		if [ "$device_flash_id" == "gd25q128" -a "$gigadevice_flash_validate" != "$validate" ]; then
			echo "1"
			return
		fi

		# Serial number 10 length support (from RUT9XX_R_00.03.491)
		if [ "${#serial_number}" = "10" -a "$gigadevice_flash_validate" != "$validate" ]; then
			echo "1"
			return
		fi

		# Su `74x164 shift register` pakeitimu gpio direktorijos nebelieka (from RUT9XX_R_00.06.01)
		if [ ! -d "/sys/bus/i2c/devices/0-0074/gpio" ] && [ "$i2c_validate"  != "$validate" ]; then
			echo "2"
			return
		fi

		# XT flash support (from RUT9XX_R_00.06.01)
		if [ "$device_flash_id" == "xt25f128b" -a "$xt_flash_validate" != "$validate" ]; then
			echo "3"
			return
		fi
	fi

	if [ "$md5_calculated" = "$md5_extracted" -a  "$md5_len" = "32" -a "$fw_name" = "$FWNAME" ]; then
		echo "$md5_calculated"
	else
		echo "0"
	fi
}

storage_size(){
	local tmp=`cat /proc/mtd | grep firmware | wc -l`
	local tmp2=`cat /proc/mtd | grep linux | wc -l`
	local ret=0
	if [ "$tmp" -eq  "1" ]; then
		ret=`cat /proc/mtd | grep firmware | awk '{print $2}'`
	elif [ "$tmp2" -eq "1" ];then
		ret=`cat /proc/mtd | grep linux | awk '{print $2}'`
	fi

	ret=$((0x$ret))

	echo "$ret"
}

check_metadata() {

	local modem_mnf="$(gsmctl -w)"
	local allow=0

	[ "$modem_mnf" = "Quectel" ] && {
		. /usr/share/libubox/jshn.sh

		fwtool -q -i /tmp/sysupgrade.meta "$FIRMWARE"
		[ -s /tmp/sysupgrade.meta ] && {
			json_load "$(cat /tmp/sysupgrade.meta)"
			json_select supported_devices
			json_get_keys dev_keys
			for k in $dev_keys; do
				json_get_var dev "$k"
				[ "$dev" = "rut900" ] && allow=1
			done
		}
	}

	[ "$allow" -eq 1 ] && {
		echo "STORAGE_SIZE=$(storage_size)"
		echo "FW_IS_NEW=1"
		echo "IMAGE_SUPPORTED=1"
		echo "IMAGE_CHECKSUM=$(md5sum $FIRMWARE | awk '{ print $1 }')"
		exit 0
	}
}

#-------------------------MAIN------------------------------------------
	if [ "$#" != 1 -a "$#" != 2 ] || [ $1 = "-h"  -o $1 = "--help" ]; then
		usage
		exit 1
	fi

	FIRMWARE=$1

	if [ ! -f $FIRMWARE ]; then
		echo "Firmware not found"
		exit 1
	fi

	if [ "$#" = 2 ]; then
		NAME=$2
	fi

	ret=$( get_fw_size )
	echo "SIZE=$ret"

	#~ New firmware series passthrough
	check_metadata

	if [ ! $NAME ]; then
		ret=$( storage_size )
		echo "STORAGE_SIZE=$ret"

		ret=$( fw_version_is_new )
		echo "FW_IS_NEW=$ret"
	fi

	ret=$( image_supported )
	echo "IMAGE_SUPPORTED=$ret"

	ret=$( image_checksum )
	echo "IMAGE_CHECKSUM=$ret"

#-----------------------------------------------------------------------
