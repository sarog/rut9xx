#
# Copyright (C) 2021 Teltonika
#

PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1
BACKUP_INCOMPATIBLE=0

# return values for validating RUT legacy firmware
lg_valid=0   # firmware is legacy and compatible
lg_invalid=1 # firmware is legacy and incompatible
lg_rutos=2   # firmware is not legacy, manifest validation is needed

# check RUT legacy firmwares
check_rut_legacy() {
	local file="$1"

	# grab and check label in header
	local l_offset=4
	local label="$(dd if="$file" bs=1 skip="$l_offset" count=9 2>/dev/null)"

	# this is not a legacy firmware
	[ "$label" != "Teltonika" ] && return "$lg_rutos"

	# grab and check model in header
	local m_offset=28
	local model="$(dd if="$file" bs=1 skip="$m_offset" count=4 2>/dev/null)"
	local board="$(mnf_info -n | cut -c -6)"

	# rut952 do not have legacy fw
	[ "$board" = "RUT952" ] && return "$lg_invalid"

	board="$(mnf_info -n | cut -c -4)"

	# incompatible model
	[ "$model" != "$board" ] && return "$lg_invalid"

	# grab and check validation options
	local v_offset=60
	local valops="$(dd if="$file" bs=1 skip="$v_offset" count=4 2>/dev/null)"

	# check for metadata in the firmware
	[ "$(fwtool -q -i /dev/null $file; echo $?)" -ne 0 ] && {
		# all options should be set, otherwise image is incompatible
		[ "$valops" != "1111" ] && return "$lg_invalid"

		# this is firmware is confirmed to be legacy without metadata
		return "$lg_valid"
	}

	return "$lg_rutos"
}

# this hook runs before fwtool validation
platform_pre_check_image() {
	local stat="$(check_rut_legacy "$1"; echo $?)"

	case "$stat" in
	"$lg_valid")
		# skip metadata validation on legacy firmware
		REQUIRE_IMAGE_METADATA=0
		BACKUP_INCOMPATIBLE=1
		return 0
		;;
	"$lg_invalid")
		return 1
		;;
	"$lg_rutos")
		return 0
		;;
	esac
}

platform_check_hw_support() {
	local metadata="/tmp/sysupgrade.meta"
	local board="$(mnf_info -n | cut -c -4)"
	local modem="$(gsmctl -n -m)"
	local stat="$(check_rut_legacy "$1"; echo $?)"
	local ser_pid="$(cat /sys/bus/usb/devices/usb1/1-1/1-1.3/idProduct)"
	local mod_ftdi_set=0

	[ -e "$metadata" ] || ( fwtool -q -i $metadata $1 ) && {
		json_load_file "$metadata"

		if ( json_select hw_mods 1> /dev/null ); then

			json_select hw_mods
			json_get_values hw_mods

			echo "Mods found: $hw_mods"

			for mod in $hw_mods; do
				case $mod in
					"ftdi_new")
						mod_ftdi_set=1
					;;
				esac
			done
		fi
	}

	if [ $board == "TRB2" ] && [ "$ser_pid" == "6001" ] && [ "$mod_ftdi_set" -eq 0 ]; then
		echo "FTDI serial chip detected but fw does not support it"
		return 1
	fi

	case "$stat" in
	"$lg_valid")
		case "$board" in
		RUT9)
			case "$modem" in
			SLM750*)
				# Meig support
				[ "$(dd if=$1 bs=1 skip=59 count=1 2>/dev/null)" != "1" ] && return 1
				;;
			esac
			;;
		RUT2)
			case "$modem" in
			SLM750*)
				# Meig support
				[ "$(dd if=$1 bs=1 skip=60 count=1 2>/dev/null)" != "1" ] && return 1
				;;
			esac
			;;
		esac
		return 0
		;;
	"$lg_invalid")
		return 0
		;;
	"$lg_rutos")
		return 0
		;;
	esac

}

# this hook runs after fwtool validation
platform_check_image() {
	return 0
}
