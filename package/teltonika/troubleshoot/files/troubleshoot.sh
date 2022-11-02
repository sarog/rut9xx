#!/bin/sh

. /lib/functions.sh
. /usr/share/libubox/jshn.sh
. /lib/functions/libtroubleshoot.sh

PACK_DIR="/tmp/troubleshoot/"
ROOT_DIR="${PACK_DIR}root/"
PACK_FILE="/tmp/troubleshoot.tar.gz"
CENSORED_STR="VALUE_REMOVED_FOR_SECURITY_REASONS"
WHITE_LIST="
dropbear.@dropbear[0].PasswordAuth
dropbear.@dropbear[0].RootPasswordAuth
luci.flash_keep.passwd
mosquitto.mqtt.password_file
openvpn.teltonika_auth_service.persist_key
openvpn.teltonika_auth_service.auth_user_pass
rpcd.@login[0].username
rpcd.@login[0].password
rpcd.@rms_login[0].username
rpcd.@rms_login[0].password
uhttpd.main.key
uhttpd.hotspot.key
rms_mqtt.rms_mqtt.key_file
"
DIR_LIST="/etc/config /etc/crontabs /etc/dropbear /etc/firewall.user /etc/group /etc/hosts \
/etc/inittab /etc/passwd /etc/profile /etc/rc.local /etc/shells /etc/sysctl.conf \
/etc/uhttpd.crt /etc/uhttpd.key /etc/board.json"

#ignore sshfs mount dir if it on tmp directory
config_load sshfs
config_get mount_point sshfs mount_point ""

IGNORE_DIR_LIST="troubleshoot luci-indexcache luci-indexcache luci-modulecache $mount_point"

generate_random_str() {
	local out="$(tr </dev/urandom -dc A-Za-z0-9 2>/dev/null | head -c $1)"
	local is_ascii="$(echo -ne "$out" | strings)"

	if [ ${#is_ascii} -eq $1 ]; then
		echo "$out"
	fi
}

secuire_tmp_config() {
	local value="$1"

	[ "${#value}" -lt 3 -o -f "$value" ] && return 0

	find "${ROOT_DIR}tmp/" \( -name "*.conf" -o -name "options*" -o -name "auth*" \) \
		-exec sed -i "s/ $value/$CENSORED_STR/g; s/\"$value\"/$CENSORED_STR/g" {} \;

	find "${ROOT_DIR}tmp/" \( -name "*.psk" -o -name "*secrets*" \) \
		-exec sed -i "s/$value/$CENSORED_STR/g" {} \;

	find "${ROOT_DIR}tmp/" -name "*localusers" \
		-exec sed -i "s/:$value:/:$CENSORED_STR:/g" {} \;

	find "${ROOT_DIR}tmp/" -name "hostapd*.conf" \
		-exec sed -i "s/=$value$/=$CENSORED_STR/g" {} \;
}

#FIXME: need optimization
secure_config() {
	local option value
	local lines="$(uci -c "$ROOT_DIR/etc/config" show |
		grep -iE "(\.)(.*)(pass|psw|pasw|psv|pasv|key|secret|username)(.*)=" |
		grep -iE "((([A-Za-z0-9]|\_|\@|\[|\]|\-)*\.){2})(.*)(pass|psw|pasw|psv|pasv|key|secret|username)(.*)=")"
	# local tmp_file=$(generate_random_str 64)

	OLD_IFS="$IFS"
	IFS=$'\n'
	for line in $lines; do
		option="${line%%=\'*}"
		value="${line#*=\'}"
		value="${value%\'}"

		[ -n "$option" ] || continue

		echo "$WHITE_LIST" | grep -iqFx "$option"
		[ $? -ne 1 ] && continue

		secuire_tmp_config "$value"
		uci -c "${ROOT_DIR}etc/config" set "${option}=${CENSORED_STR}"
	done
	IFS="$OLD_IFS"

	uci -c "$ROOT_DIR/etc/config" commit
}

get_mnf_info() {
	local log_file="$1"
	local MNF_LIST="mac maceth name wps sn batch hwver"

	for arg in ${MNF_LIST}; do
		echo -ne "$arg:   \t" >>"$log_file"
		mnf_info --$arg >>"$log_file" 2>&1
	done
}

system_hook() {
	local log_file="${PACK_DIR}sysinfo.log"

	troubleshoot_init_log "SYSTEM INFORMATION" "$log_file"
	get_mnf_info "$log_file"

	troubleshoot_init_log "Active SIM]" "$log_file"
	troubleshoot_add_log "$(ubus call sim get)" "$log_file"

	troubleshoot_init_log "Firmware version" "$log_file"
	troubleshoot_add_file_log "/etc/version" "$log_file"

	troubleshoot_init_log "Time" "$log_file"
	troubleshoot_add_log "$(date)" "$log_file"

	troubleshoot_init_log "Uptime" "$log_file"
	troubleshoot_add_log "$(uptime)" "$log_file"
	[ -e /proc/version ] && {
		troubleshoot_init_log "Build string" "$log_file"
		troubleshoot_add_file_log "/proc/version" "$log_file"
	}
	[ -e /proc/mtd ] && {
		troubleshoot_init_log "Flash partitions" "$log_file"
		troubleshoot_add_file_log "/proc/mtd" "$log_file"
	}
	[ -e /proc/meminfo ] && {
		troubleshoot_init_log "Memory usage" "$log_file"
		troubleshoot_add_file_log "/proc/meminfo" "$log_file"
	}

	troubleshoot_init_log "Filesystem usage statistics" "$log_file"
	troubleshoot_add_log "$(df -h)" "$log_file"

	[ -d /log ] && {
		troubleshoot_init_log "Log dir" "$log_file"
		troubleshoot_add_log "$(ls -al /log/)" "$log_file"
	}

	troubleshoot_init_log "USB DEVICES" "$log_file"
	troubleshoot_add_log "$(lsusb)" "$log_file"

	troubleshoot_init_log "UBUS METHODS" "$log_file"
	troubleshoot_add_log "$(ubus list)" "$log_file"

	troubleshoot_init_log "IP BLOCK" "$log_file"
	troubleshoot_add_log "$(ubus call ip_block show)" "$log_file"
}

switch_hook() {
	local ethernet
	local log_file="${PACK_DIR}switch.log"

	config_load hwinfo
	config_get ethernet hwinfo ethernet 0
	[ "$ethernet" -eq 1 ] && [ -n "$(which swconfig)" ] || return

	troubleshoot_init_log "Switch configuration" "$log_file"
	troubleshoot_add_log_ext "swconfig" "dev switch0 show" "$log_file"
}

wifi_hook() {
	local wifi __tmp devname
	local log_file="${PACK_DIR}wifi.log"

	config_load hwinfo
	config_get wifi hwinfo wifi 0

	[ "$wifi" -eq 1 ] && [ -n "$(which iw)" ] || return

	__tmp="$(ubus call network.wireless status 2>&1)"
	__cmd="$(jsonfilter -s "$__tmp" -e "wifaces=@.*.interfaces[@].ifname")"
	eval "$__cmd"

	[ -z "$wifaces" ] && return

	troubleshoot_init_log "WIFI clients" "$log_file"
	for devname in ${wifaces}; do
		troubleshoot_add_log "$(iw dev "$devname" station dump 2>/dev/null | grep Station)" "$log_file"
	done

}

systemlog_hook() {
	local log_flash_file
	local log_file="${PACK_DIR}system.log"

	troubleshoot_init_log "LOGGING INFORMATION" "$log_file"
	troubleshoot_init_log "Dmesg" "$log_file"
	troubleshoot_add_log "$(dmesg)" "$log_file"

	config_load system
	config_get log_flash_file system "log_file" ""

	troubleshoot_init_log "Logread" "$log_file"
	[ -n "$log_flash_file" ] && [ -f "$log_flash_file" ] &&
		troubleshoot_add_file_log "$log_flash_file" "$log_file" || troubleshoot_add_log "$(logread)" "$log_file"
}

services_secure_passwords() {
	local file="$1"
	local lines passwd

	lines=$(grep "ppp\.sh" "$file")

	OLD_IFS="$IFS"
	IFS=$'\n'
	for line in $lines; do
		passwd=${line#*password\":\"}
		passwd="${passwd%\"*}"

		sed -i "s/$passwd/$CENSORED_STR/g" "$file"
	done
	IFS="$OLD_IFS"

	passwd=$(uci -q get modbusgateway.gateway.pass)
	[ -n "$passwd" ] &&
		sed -i "s/\"$passwd\"/$CENSORED_STR/g" "$file"

	passwd=$(uci -q get iottw.thingworx.appkey)
	[ -n "$passwd" ] &&
		sed -i "s/\"$passwd\"/$CENSORED_STR/g" "$file"
}

services_hook() {
	local log_file="${PACK_DIR}services.log"

	troubleshoot_init_log "Process list" "$log_file"
	troubleshoot_add_log "$(ps -ww)" "$log_file"

	troubleshoot_init_log "SERVICES" "$log_file"
	troubleshoot_add_log "$(ubus call service list)" "$log_file"

	services_secure_passwords "$log_file"
}
generate_root_hook() {
	mkdir "$ROOT_DIR"
	mkdir "${ROOT_DIR}/etc"
	mkdir "${ROOT_DIR}/tmp"
	for file in $(ls /tmp/); do
		[ "$(echo ${IGNORE_DIR_LIST} | grep -wc ${file})" -gt 0 ] && continue

		cp -rf "/tmp/${file}" "${ROOT_DIR}/tmp"
	done

	cp -r ${DIR_LIST} "${ROOT_DIR}/etc"
	secure_config
}

generate_package() {
	cd /tmp || return
	tar -czf "$PACK_FILE" troubleshoot >/dev/null 2>&1
	rm -r "$PACK_DIR"
	rm -f "$TMP_LOG_FILE"
}

init() {
	rm -r "$PACK_DIR" >/dev/null 2>&1
	rm "$PACK_FILE" >/dev/null 2>&1
	mkdir "$PACK_DIR"
}

init

troubleshoot_hook_init system_hook
troubleshoot_hook_init switch_hook
troubleshoot_hook_init wifi_hook
troubleshoot_hook_init services_hook
troubleshoot_hook_init systemlog_hook

#init external hooks
[ -d "/lib/troubleshoot" ] && {
	for tr_source_file in /lib/troubleshoot/*; do
		. $tr_source_file
	done
}

troubleshoot_hook_init generate_root_hook
troubleshoot_run_hook_all

generate_package
