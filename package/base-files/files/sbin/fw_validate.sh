#!/bin/sh

usage () {
	echo -e "Firmware validation check"
	echo -e "\tUsage: <FIRMWARE>"
}

VERIFY_BOOT=0

get_fw_version() {
	[ $# -ne 1 ] && return 1

	. /usr/share/libubox/jshn.sh

	if ! fwtool -q -i /tmp/sysupgrade.meta "$1"; then
		echo "Firmware metadata not found"
		return 1
	fi

	json_load_file "/tmp/sysupgrade.meta" || {
		echo "Invalid firmware metadata"
		return 1
	}

	json_get_var fw_version "version" || {
		echo "Missing version entry"
		return 1
	}

	echo "$fw_version"
}

fw_version_check () {
	[ "$#" -ne 1 ] && return 1

	local firmware="$1"

	local current_fw_version=$(cat /etc/version)
	if [ ! "$current_fw_version" ]; then
		return 0
	fi

	local img_fw_version=$(get_fw_version "$firmware")

	if [ "$?" -eq 1 -o ! "$img_fw_version" ]; then
		VERIFY_BOOT=0
		return 0
	fi

	local fw_major=$(echo "$img_fw_version" | awk -F . '{ print $2 }')
	local fw_minor=$(echo "$img_fw_version" | awk -F . '{ print $3 }')
	fw_minor=$(echo "$fw_minor" | awk -F _ '{ print $1 }')

	local device_name=$(mnf_info -n | cut -c1-4 2>/dev/null)
	local blver=$(mnf_info -v)
	local vb=$(echo $blver | grep '\-vb')

	if [ "$device_name" = "RUT9" ]; then
		[ -n "$vb" ] && [ "$img_fw_version" = "Firmware metadata not found" ] && {
			VERIFY_BOOT=1
			return 0
		}
		[ -n "$vb" ] && [ "$fw_major" -lt "07" ] && VERIFY_BOOT=1
	elif [ "$device_name" = "RUT2" ]; then
		[ -n "$vb" ] && [ "$img_fw_version" = "Firmware metadata not found" ] && {
			VERIFY_BOOT=1
			return 0
		}
		[ -n "$vb" ] && [ "$fw_major" -lt "07" ] && VERIFY_BOOT=1
		[ -n "$vb" ] && [ "$fw_major" -eq "07" ] && [ "$fw_minor" -lt "01" ] && VERIFY_BOOT=1
	fi

	local current_client=$(echo "$current_fw_version" | awk -F . '{ print $1 }')
	current_client=$(echo "$current_client" | awk -F _ '{ print $NF }')
	local fw_client=$(echo "$img_fw_version" | awk -F . '{ print $1 }')
	fw_client=$(echo "$fw_client" | awk -F _ '{ print $NF }')

	if [ ! "$current_client" ] || [ ! "$fw_client" ]; then
		return 0
	elif [ "$current_client" != "$fw_client" ]; then
		return 0
	fi

	local current_major=$(echo "$current_fw_version" | awk -F . '{ print $2 }')

	if [ ! "$current_major" ] || [ ! "$fw_major" ]; then
		return 0
	elif [ "$current_major" -gt "$fw_major" ]; then
		return 0
	elif [ "$fw_major" -gt "$current_major" ]; then
		return 1
	fi

	local current_minor=$(echo "$current_fw_version" | awk -F . '{ print $3 }')
	current_minor=$(echo "$current_minor" | awk -F _ '{ print $1 }')

	if [ ! "$current_minor" ] || [ ! "$fw_minor" ]; then
		return 0
	fi

	if [ "$fw_minor" -lt "$current_minor" ]; then
		return 0
	fi

	return 1
}

if [ "$#" -ne 1 ] || [ "$1" = "-h" -o "$1" = "--help" ]; then
	usage
	exit 1
fi

firmware="$1"

if [ ! -f "$firmware" ]; then
	echo "Firmware not found"
	exit 1
fi

fw_version_check "$firmware"
echo "FW_NEWER=$?"
echo "VERIFY_BOOT=$VERIFY_BOOT"

