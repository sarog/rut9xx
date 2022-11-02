#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

PACKAGE_NAME="snmpd_tmp"
_OLD_SECTION_NAME=

init_section() {
	_SECTION_NEW_TYPE="$1"
	_SECTION_NEW="$2"

	[ -z "$_SECTION_NEW" ] && {
		uci_add "$PACKAGE_NAME" "$_SECTION_NEW_TYPE"
		_SECTION_NEW="$CONFIG_SECTION"
		return
	}

	uci_get "$PACKAGE_NAME" "$_SECTION_NEW" >/dev/null || {
		uci_add "$PACKAGE_NAME" "$_SECTION_NEW_TYPE" "$_SECTION_NEW"
	}

}

set_option() {
	local option="$1"
	local value="$2"

	uci_set "$PACKAGE_NAME" "$_SECTION_NEW" "$option" "$value"
}

init_user_section() {
	uci_get "$PACKAGE_NAME" "user" >/dev/null || {
		uci_add "$PACKAGE_NAME" "user" "user"
		uci_set "$PACKAGE_NAME" "user" "rights" "ro"
		uci_set "$PACKAGE_NAME" "user" "seclevel" "priv"
		uci_set "$PACKAGE_NAME" "user" "enabled" "1"
	}
}

agentaddress_agent_cb() {
	set_option "proto" "udp"
}

portNumber_agent_cb() {
	set_option "port" "$1"
}

version_agent_cb() {
	local value="$1"

	for version in $(echo $value | tr "/" "\n"); do
		[ "$version" = "v2" ] && \
			set_option "v2cmode" "1" || \
			set_option "${version}mode" "1"
	done
}

user_name_agent_cb(){
	init_user_section
	uci_set "$PACKAGE_NAME" "user" "username" "$1"
}

auth_type_agent_cb(){
	init_user_section
	uci_set "$PACKAGE_NAME" "user" "authtype" "$1"
}

auth_pass_agent_cb(){
	init_user_section
	uci_set "$PACKAGE_NAME" "user" "authpass" "$1"
}

encryption_type_agent_cb(){
	init_user_section
	uci_set "$PACKAGE_NAME" "user" "privtype" "$1"
}

encryption_pass_agent_cb() {
	init_user_section
	uci_set "$PACKAGE_NAME" "user" "privpass" "$1"
}

remoteAccess_agent_cb() {
	set_option "remoteAccess" "$1"
}

_community_agent_cb() {
	return 0
}

action_trap_cb() {
	local value="$1"
	local io_type io_name

	case "$value" in
		sigEnb)
			value="signalstrtrap"
		;;
		conEnb)
			value="conntypetrap"
		;;
		digIn)
			value="iotrap"
			io_type="gpio"
			io_name="din2"
		;;
		digOCIn)
			value="iotrap"
			io_type="gpio"
			io_name="iio"
		;;
		dig4PinIn)
			value="iotrap"
			io_type="gpio"
			io_name="din1"
		;;
		analog)
			value="iotrap"
			io_type="adc"
			io_name="adc0"
		;;
		digOut)
			value="iotrap"
			io_type="gpio"
			io_name="dout2"
		;;
		digRelayOut)
			value="iotrap"
			io_type="relay"
			io_name="relay0"
		;;
		dig4PinOut)
			value="iotrap"
			io_type="gpio"
			io_name="dout1"
		;;
	esac

	set_option "type" "$value"
	[ -n "$io_type" ] && \
		set_option "io_type" "$io_type"

	[ -n "$io_name" ] && \
		set_option "io_name" "$io_name"

}

volts_trap_cb() {
	local value="$1"

	set_option "voltage" "$value"
}

translate_relay_state() {
	local state=$1

	case "$state" in
		active)
			echo "closed"
		;;
		inactive)
			echo "open"
		;;
		*)
			echo "$state"
		;;
	esac
}

state_trap_cb() {
	local value="$1"
	local action state

	action=$(uci_get "snmpd" "$_OLD_SECTION_NAME" "action")
	case "$action" in
		digRelayOut)
			state=$(translate_relay_state "$value")
		;;
		*)
			state="$value"
		;;
	esac
	set_option "state" "$state"
}

analog_state_trap_cb() {
	return 0
}

move_trap2sink_otion() {
	local name="${1:5}"
	local value="$2"

	set_option "$name" "$value"
}

move_option() {
	local name="$1"
	local value="$2"

	set_option "$name" "$value"
}

config_cb() {
	local type="$1"
	local name="$2"

	option_cb() {
		return 0
	}

	case "$type" in
		agent)
			init_section agent general
			option_cb() {
				local name="$1"
				local value="$2"

				type "${name}_agent_cb" >/dev/null && \
					eval "${name}_agent_cb $value $name" || \
					move_option "$name" "$value"
			}
			;;
		com2sec)
			[ "$name" = "public" ] && return
			[ "$name" = "private" ] && return

			option_cb() {
				local name="$1"
				local value="$2"

				case "$name" in
					community|secname)
						uci_set "$PACKAGE_NAME" "public" "$name" "$value"
						;;
					*)
				esac
			}
			;;
		system)
			init_section system "@system[0]"
			option_cb() {
				local name="$1"
				local value="$2"

				move_option "$name" "$value"
			}
			;;
		trap)
			init_section "trap2sink"
			option_cb() {
				local name="$1"
				local value="$2"

				move_trap2sink_otion "$name" "$value"
			}
			;;
		rule)
			init_section "trap"
			_OLD_SECTION_NAME="$name"
			option_cb() {
				local name="$1"
				local value="$2"

				type "${name}_trap_cb" >/dev/null && \
					eval "${name}_trap_cb $value $name" || \
					move_option "$name" "$value"
			}
			;;
		*)
			option_cb() {
				return 0
			}
		;;
	esac
}

create_tmp_config() {
	touch "/etc/config/$PACKAGE_NAME"
	uci_add "$PACKAGE_NAME" agent general
	uci_set "$PACKAGE_NAME" general enabled "0"
	uci_set "$PACKAGE_NAME" general proto "UDP"
	uci_set "$PACKAGE_NAME" general ipfamily "ipv4"
	uci_set "$PACKAGE_NAME" general port "161"
	uci_set "$PACKAGE_NAME" general v1mode "1"
	uci_set "$PACKAGE_NAME" general v2cmode "1"
	uci_set "$PACKAGE_NAME" general mibfile "/etc/snmp/MIB.txt"

	uci_add "$PACKAGE_NAME" com2sec public
	uci_set "$PACKAGE_NAME" public secname "ro"
	uci_set "$PACKAGE_NAME" public source "default"
	uci_set "$PACKAGE_NAME" public community "public"
	uci_set "$PACKAGE_NAME" public ipaddr "0.0.0.0"
	uci_set "$PACKAGE_NAME" public netmask "0"
	uci_add "$PACKAGE_NAME" com2sec private
	uci_set "$PACKAGE_NAME" private secname "rw"
	uci_set "$PACKAGE_NAME" private source "localhost"
	uci_set "$PACKAGE_NAME" private community "private"
	uci_set "$PACKAGE_NAME" private ipaddr "127.0.0.1"
	uci_set "$PACKAGE_NAME" private netmask "32"

	uci_add "$PACKAGE_NAME" com2sec6
	uci_set "$PACKAGE_NAME" "$CONFIG_SECTION" secname "ro"
	uci_set "$PACKAGE_NAME" "$CONFIG_SECTION" source "default"
	uci_set "$PACKAGE_NAME" "$CONFIG_SECTION" community "public"
	uci_add "$PACKAGE_NAME" com2sec6
	uci_set "$PACKAGE_NAME" "$CONFIG_SECTION" secname "rw"
	uci_set "$PACKAGE_NAME" "$CONFIG_SECTION" source "default"
	uci_set "$PACKAGE_NAME" "$CONFIG_SECTION" community "private"

	uci_add "$PACKAGE_NAME" group public_v1
	uci_set "$PACKAGE_NAME" public_v1 group "public"
	uci_set "$PACKAGE_NAME" public_v1 version "v1"
	uci_set "$PACKAGE_NAME" public_v1 secname "ro"
	uci_add "$PACKAGE_NAME" group public_v2c
	uci_set "$PACKAGE_NAME" public_v2c group "public"
	uci_set "$PACKAGE_NAME" public_v2c version "v2c"
	uci_set "$PACKAGE_NAME" public_v2c secname "ro"
	uci_add "$PACKAGE_NAME" group public_usm
	uci_set "$PACKAGE_NAME" public_usm group "public"
	uci_set "$PACKAGE_NAME" public_usm version "usm"
	uci_set "$PACKAGE_NAME" public_usm secname "ro"

	uci_add "$PACKAGE_NAME" group private_v1
	uci_set "$PACKAGE_NAME" private_v1 group "private"
	uci_set "$PACKAGE_NAME" private_v1 version "v1"
	uci_set "$PACKAGE_NAME" private_v1 secname "rw"
	uci_add "$PACKAGE_NAME" group private_v2c
	uci_set "$PACKAGE_NAME" private_v2c group "private"
	uci_set "$PACKAGE_NAME" private_v2c version "v2c"
	uci_set "$PACKAGE_NAME" private_v2c secname "rw"
	uci_add "$PACKAGE_NAME" group private_usm
	uci_set "$PACKAGE_NAME" private_usm group "private"
	uci_set "$PACKAGE_NAME" private_usm version "usm"
	uci_set "$PACKAGE_NAME" private_usm secname "rw"

	uci_add "$PACKAGE_NAME" view all
	uci_set "$PACKAGE_NAME" all viewname "all"
	uci_set "$PACKAGE_NAME" all type "included"
	uci_set "$PACKAGE_NAME" all oid ".1"

	uci_add "$PACKAGE_NAME" access public_access
	uci_set "$PACKAGE_NAME" public_access group "public"
	uci_set "$PACKAGE_NAME" public_access context "none"
	uci_set "$PACKAGE_NAME" public_access version "any"
	uci_set "$PACKAGE_NAME" public_access level "noauth"
	uci_set "$PACKAGE_NAME" public_access prefix "exact"
	uci_set "$PACKAGE_NAME" public_access read "all"
	uci_set "$PACKAGE_NAME" public_access write "none"
	uci_set "$PACKAGE_NAME" public_access notify "none"
	uci_add "$PACKAGE_NAME" access private_access
	uci_set "$PACKAGE_NAME" private_access group "private"
	uci_set "$PACKAGE_NAME" private_access context "none"
	uci_set "$PACKAGE_NAME" private_access version "any"
	uci_set "$PACKAGE_NAME" private_access level "noauth"
	uci_set "$PACKAGE_NAME" private_access prefix "exact"
	uci_set "$PACKAGE_NAME" private_access read "all"
	uci_set "$PACKAGE_NAME" private_access write "all"
	uci_set "$PACKAGE_NAME" private_access notify "all"

	uci_add "$PACKAGE_NAME" system
	uci_set "$PACKAGE_NAME" "$CONFIG_SECTION" sysLocation "location"
	uci_set "$PACKAGE_NAME" "$CONFIG_SECTION" sysContact "email@example.com"
	uci_set "$PACKAGE_NAME" "$CONFIG_SECTION" sysName "name"
	uci_add "$PACKAGE_NAME" exec
	uci_set "$PACKAGE_NAME" "$CONFIG_SECTION" name "filedescriptors"
	uci_set "$PACKAGE_NAME" "$CONFIG_SECTION" prog "/bin/cat"
	uci_set "$PACKAGE_NAME" "$CONFIG_SECTION" args "/proc/sys/fs/file-nr"
}

create_tmp_config
config_load snmpd

uci_commit "$PACKAGE_NAME"
rm /etc/config/snmpd
mv /etc/config/snmpd_tmp /etc/config/snmpd
