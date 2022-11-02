#!/bin/sh
. /lib/functions.sh
PACKAGE_FILE="/etc/package_restore.txt"

[ -s "$PACKAGE_FILE" ] && {
	sed -i "s/tlt_custom_pkg_//g" "$PACKAGE_FILE"
	sed -i "s/cotStreamApp/coStreamApp/g" "$PACKAGE_FILE"
	sed -i "s/cmstreamapp/cmStreamApp/g" "$PACKAGE_FILE"
	sed -i "s/twstreamapp/twStreamApp/g" "$PACKAGE_FILE"
	sed -i "s/snmpd/snmp/g" "$PACKAGE_FILE"
	sed -i "/telenor_mqtt/d" "$PACKAGE_FILE" #TODO: remove this line when telenor package is prepared
	sed -i "s/USB Tools/Network shares/g" "$PACKAGE_FILE"
	sed -i "s/luci-i18n/vuci-i18n/g" "$PACKAGE_FILE"
	sed -i "s/language set/language support/g" "$PACKAGE_FILE"
	sed -i "/wireguard/d" "$PACKAGE_FILE"
}

get_interface() {
	local enabled proto

	config_get enabled "$1" "$2" ""
	config_get proto "$1" proto ""

	[ "$enabled" = "$4" ] && [ "$proto" = "$3" ] && ENABLED=1
}

get_enabled() {
	local enabled

	config_get_bool enabled "$1" "$2" 0

	[ "$enabled" -eq 1 ] && ENABLED=1
}

check_config() {
	local config="$1"
	local section="$2"
	local enabled_option="$3"
	local pkg_name="$4"
	local full_name="$5"

	ENABLED=0
	config_load "$config"

	case "$config" in
	"network")
		config_foreach get_interface "$section" "$enabled_option" "$6" "$7"
		;;
	*)
		config_foreach get_enabled "$section" "$enabled_option"
		;;
	esac

	check=$(grep -w "$pkg_name" "$PACKAGE_FILE" 2>/dev/null | wc -l)
	[ "$ENABLED" -eq 1 ] && [ "$check" -eq 0 ] && echo "$pkg_name - $full_name" >>"$PACKAGE_FILE"
}

check_config "samba" "samba" "enabled" "samba36-server" "Network shares"
check_config "easycwmp" "local" "enable" "easycwmp" "TR-069"
check_config "chilli" "chilli" "enabled" "coova-chilli" "Hotspot"
check_config "vrrpd" "vrrpd" "enabled" "vrrpd" "VRRP"
check_config "dmvpn" "dmvpn" "enabled" "dmvpn" "DMVPN"
check_config "upnpd" "upnpd" "enabled" "miniupnpd" "UPNP"
check_config "snmpd" "agent" "enabled" "snmp" "SNMP"
check_config "stunnel" "globals" "enabled" "stunnel" "Stunnel"
check_config "etherwake" "etherwake" "broadcast" "etherwake" "Wake on LAN"
check_config "etherwake" "target" "wakeonboot" "etherwake" "Wake on LAN"
check_config "hostblock" "hostblock" "enabled" "web_filter" "WEB Filter"
check_config "privoxy" "privoxy" "enabled" "web_filter" "WEB Filter"
check_config "mqtt_pub" "mqtt_pub" "enabled" "mqtt_pub" "MQTT"
check_config "mosquitto" "mqtt" "enabled" "mqtt_pub" "MQTT"
check_config "ddns" "service" "enabled" "tlt-ddns" "DDNS"
check_config "ulogd" "ulogd" "enabled" "tlt-ulogd" "Traffic Logging"
check_config "system" "system" "tcp_dump" "tcpdump" "TCPdump"
check_config "network" "interface" "disabled" "sstp-client" "SSTP" "sstp" "0"
check_config "udprelay" "general" "enabled" "udprelay" "UDP Broadcast Relay"
check_config "network" "interface" "enabled" "relayd" "Relay configuration" "relay" "1"
check_config "frr" "bgp_general" "enabled" "frr-bgpd" "BGP daemon"
check_config "frr" "rip_general" "enabled" "frr-ripd" "RIP daemon"
check_config "frr" "ospf" "enabled" "frr-ospfd" "OSPFv2 daemon"
check_config "frr" "nhrp_general" "enabled" "frr-nhrp" "NHRP daemon"
check_config "modbus_serial_master" "rs232" "enabled" "modbus_serial_master" "Modbus Serial Master"
check_config "modbus_serial_master" "rs485" "enabled" "modbus_serial_master" "Modbus Serial Master"
check_config "smpp" "smpp" "enabled" "smpp" "SMPP"
check_config "qos" "interface" "enabled" "qos-scripts" "QoS scripts"

exit 0
