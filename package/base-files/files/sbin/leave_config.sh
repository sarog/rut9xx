#!/bin/sh

#Deletes configuration leaving specified config files in /etc/config/.
#Used for restoring factory defaults while leaving some configuration.

ETC="/etc"
ETC_CONFIG="/etc/config"
TMP="/tmp/leavecfg"
TMP_CONF="$TMP/config"
VPN_CERT="/lib/uci/upload"
TLT_VPN="/etc/openvpn"

VPN="0"


usage()
{
	echo "Usage: $0 <config1> <config2> <config3> ..."
	echo "config* - config files to leave while deleting configuration"
}

if [ $# -lt 1 ]; then
	usage
	exit 1
fi

#Backup config
mkdir -p $TMP_CONF
for config in $*; do
	if [ "$config" = "openvpn" ]; then
		VPN="1"
		cp -r $TLT_VPN $TMP/
	fi
	#Change first_login before saving teltonika config
	#if [ "$config" = "teltonika" ]; then
	#	uci set teltonika.sys.first_login=1
	#	uci commit teltonika
	#fi
	cp "$ETC_CONFIG/$config" "$TMP_CONF/$config"
done

#Delete configuration
rm -rf $ETC/*
if [ "$VPN" = "0" ]; then
	mkdir -p $TLT_VPN
fi

#Restore config
mkdir -p $ETC_CONFIG
cp -r $TMP/* $ETC/
rm -rf $TMP


