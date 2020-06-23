#!/bin/sh

#Store specified configuration to flash and retrieve it

name="$0"

#Configuration sections and lists of parameters
#g3_cfg="network.ppp"
#g3_list="apn pincode dialnumber auth_mode username password"
lan_cfg="network.lan"
lan_list="ipaddr netmask"
sim1_cfg="simcard.sim1"
sim1_list="_method ifname proto apn pincode dialnumber auth_mode username password service device method roaming pdptype mtu"
sim2_cfg="simcard.sim2"
sim2_list="_method ifname proto apn pincode dialnumber auth_mode username password service device method roaming pdptype mtu"
ppp_cfg="network.ppp"
ppp_list="ifname proto apn pincode dialnumber auth_mode username password service method roaming pdptype mtu pppd_options enabled disabled"

tmpfile="/tmp/keep_settings.tmp"
mtdfile="/tmp/keep_settings.mtd"

CONFIG_PART="config"
CONFIG_OFFSET="40960"	# 0xA000
CONFIG_LENGTH="4096"	# 0x1000

usage()
{
	echo "Reads/writes settings from/to '$CONFIG_PART' partition"
	echo "Usage: $name COMMAND SETTING1 SETTING2 ..."
	echo "Supported commands:"
	echo "  store - 	Store settings to flash"
	echo "  get    -	Get settings from flash"
	echo "  delete -	Detele all settings from flash"
	echo "Supported settings:"
	echo "  3g     -	Store/get 3g settings (PIN, APN, dialnumber ...)"
	echo "  lan    -	Store/get lan settings (IP, netmask)"
}

get_config_part() {
	local config_mtd_num=`cat /proc/mtd | grep -m 1 "\"$CONFIG_PART\"" | head -c 4 | tail -c 1`
	if [ -z "$config_mtd_num" ]; then
		echo "Partition '$CONFIG_PART' not found"
		exit 1
	fi
	
	echo "$config_mtd_num"
}

init_store()
{
	rm -f $tmpfile
}

#Stores configuration to tmp file for writing
# $1 - configuration section, e.g. "network.ppp"
# $2 - list of parameters to store, e.g. "apn pincode dialnumber"
store_cfg()
{
	cfg=$1
	list=$2
	pattern=""
	
	if [ -z "$cfg" ] || [ -z "$list" ]; then
		logger "$name. Missing parameters for $0"
		exit 1
	fi
	
	for item in $list; do
		#pattern=$pattern" -e $cfg.$item"
		param=`uci show $cfg.$item 2>/dev/null`
		if [ "$?" != "0" ] || [ -z "$param" ]; then
			continue
		fi
		echo "$param" >> $tmpfile
	done
}

#Performs actual write to flash
write_cfg()
{
	if [ ! -s "$tmpfile" ] && [ "$1" != "delete" ]; then
		logger "$name. Write error: file $tmpfile not found"
		exit 1
	fi
	
	config_mtd_num=$(get_config_part)
	
	#Generate 0xff file
	dd if=/dev/zero ibs=1 count=$CONFIG_LENGTH | tr "\000" "\377" > $mtdfile
	
	#Insert configuration for saving, unless delete is performed
	if [ "$1" != "delete" ]; then
		dd if=$tmpfile of=$mtdfile bs=1 count=$CONFIG_LENGTH conv=notrunc
	fi
	
	#Erase and write to mtd device
	dd if=$mtdfile of=/dev/mtdblock"$config_mtd_num" bs=1 count=$CONFIG_LENGTH seek=$CONFIG_OFFSET
	
	#Cleanup
	rm -f $tmpfile $mtdfile
}

#Deletes all configuration from flash
delete_cfg()
{
	write_cfg "delete"
}

#Read configuration from flash
# $1 - configuration section, e.g. "network.ppp"
get_cfg()
{
	cfg=$1
	config_mtd_num=$(get_config_part)
	list=`dd if=/dev/mtdblock"$config_mtd_num" bs=1 count=$CONFIG_LENGTH skip=$CONFIG_OFFSET 2>/dev/null | awk -F $'\xff' '{print $1}' | grep $cfg`
	for item in $list; do
		if [ -z "$item" ]; then
			continue;
		fi
		echo "$item"
	done
}

if [ "$1" != "store" ] && [ "$1" != "get" ] && [ "$1" != "delete" ]; then
	usage
	exit 1
fi

cmd=$1

if [ "$1" = "delete" ]; then
	delete_cfg
	return 0
fi

if [ $# -lt 2 ]; then
	usage
	exit 1
fi

init_store

#Allow combined settings list
i="1"
while [ -n "$2" ]; do
	if [ "$2" = "sim1" ]; then
		cfg="$sim1_cfg"
		list="$sim1_list"
	elif [ "$2" = "sim2" ]; then
		cfg="$sim2_cfg"
		list="$sim2_list"
	elif [ "$2" = "ppp" ]; then
		cfg="$ppp_cfg"
		list="$ppp_list"
	elif [ "$2" = "lan" ]; then
		cfg="$lan_cfg"
		list="$lan_list"
	elif [ "$i" = "1" ]; then
		#Print usage only if first config is wrong
		usage
		exit 1
	fi
	i="0"
	
	$cmd"_cfg" "$cfg" "$list"
	shift
done

if [ "$cmd" = "store" ]; then
	write_cfg
fi

return 0

