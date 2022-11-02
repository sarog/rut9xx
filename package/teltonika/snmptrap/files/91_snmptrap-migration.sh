#!/bin/sh

. /lib/functions.sh

move_trap() {
	local sec=$1
	local type enabled io_type state iomane

	config_get type "$sec" type
	config_get enabled "$sec" enabled
	
	uci_add snmptrap trap
	uci_set snmptrap "$CONFIG_SECTION" enabled "$enabled"

	case $type in
		iotrap)
			config_get state "$sec" state
			config_get ioname "$sec" io_name
			config_get io_type "$sec" io_type

			uci_set snmptrap "$CONFIG_SECTION" name "$ioname"
			uci_set snmptrap "$CONFIG_SECTION" type "$type"
			uci_set snmptrap "$CONFIG_SECTION" state "$state"
			uci_set snmptrap "$CONFIG_SECTION" io_type "$io_type"
			;; 
		signalstrtrap|conntypetrap)
			config_get signal "$sec" signal

			uci_set snmptrap "$CONFIG_SECTION" type "gsm"
			uci_set snmptrap "$CONFIG_SECTION" name "$type"
			uci_set snmptrap "$CONFIG_SECTION" signal "$signal"
			;;
	esac

	uci_remove snmpd "$sec"

}

move_server() {
	local sec=$1
	local enabled host port community

	config_get enabled "$sec" enabled 0
	config_get host "$sec" host
	config_get port "$sec" port 162
	config_get community "$sec" community "Public"

	uci_add snmptrap server
	uci_set snmptrap "$CONFIG_SECTION" enabled "$enabled"
	uci_set snmptrap "$CONFIG_SECTION" host "$host"
	uci_set snmptrap "$CONFIG_SECTION" port "$port"
	uci_set snmptrap "$CONFIG_SECTION" community "$community"
	uci_set snmptrap "$CONFIG_SECTION" version "2c"

	uci_remove snmpd "$sec"
}

[ -f /etc/config/snmptrap ] || touch /etc/config/snmptrap

config_load snmpd
config_foreach move_server trap2sink
config_foreach move_trap trap

uci_commit snmptrap
uci_commit snmpd
