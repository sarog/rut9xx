#!/bin/bash

SCRIPT_NAME=$(basename $0)
FIRMWARE_SIZE=16777216 # 0x1000000
ART_FILE="scripts/art_tlt_rut900.bin"
BOOTROM_FILE="scripts/uboot_for_tlt_rut9xx_3.2.5.bin"
CONFIG_FILE="scripts/config_tlt_rut900.bin"
WEBUI_FILE=""

while [ $# -ne 0 ]; do
	case "$1" in
		--art)
			ART_FILE="$2"
			shift
		;;
		--bootrom)
			BOOTROM_FILE="$2"
			shift
		;;
		--config)
			CONFIG_FILE="$2"
			shift
		;;
		--webui)
			WEBUI_FILE="$2"
			shift
		;;
		*)
			shift
		;;
	esac
done

if [ -s "$ART_FILE" ] && [ -s "$BOOTROM_FILE" ] && [ -s "$CONFIG_FILE" ] && [ -s "$WEBUI_FILE" ]; then
	rm -f master_nopad.img pad.img master_stendui.img
	# create an unpaded image by first layering the bootrom, config, art images immediately
	# followed by the firmware image created by the build proccess
	cat $BOOTROM_FILE $CONFIG_FILE $ART_FILE $WEBUI_FILE > master_nopad.img
	# grab the size of unpaded master image
	masterSize=`stat -c %s master_nopad.img`
	if [ $masterSize -gt $FIRMWARE_SIZE ]; then
		overboard=$(($masterSize - $FIRMWARE_SIZE))
		echo "Firmware size is too large by $overboard bytes!"
		exit 2
	fi
	# calculate the ammount of padding we will need
	padSize=$(( $FIRMWARE_SIZE - $masterSize ))
	# create the padding (just the padding) that we will use to top of the finalisation
	# master image
	tr "\000" "\377" < /dev/zero | dd ibs=1 count=$padSize of=pad.img
	BOOTLOADER_NAME="$(tail --byte 10 $BOOTROM_FILE | head --byte 5)"
	master_name=$(basename $WEBUI_FILE);
	master_name="${master_name%_*}_UBOOT_${BOOTLOADER_NAME}_MASTER_STENDUI.bin"
	# create the master image
	cat master_nopad.img pad.img > $master_name
	rm -f master_nopad.img pad.img
else
	echo "Execute $SCRIPT_NAME from project root"
	echo -e "Example: ./scripts/$SCRIPT_NAME --webui RUT_WEBUI.bin\n"

	if [ ! -s "$ART_FILE" ]; then
		echo "ART_FILE zero or doesn't exist, use --art to specify the file"
	fi

	if [ ! -s "$BOOTROM_FILE" ]; then
		echo "BOOTROM_FILE zero or doesn't exist, use --bootrom to specify the file"
	fi

	if [ ! -s "$CONFIG_FILE" ]; then
		echo "CONFIG_FILE zero or doesn't exist, use --config to specify the file"
	fi

	if [ ! -s "$WEBUI_FILE" ]; then
		echo "WEBUI_FILE zero or doesn't exist, use --webui to specify the file"
	fi
fi
