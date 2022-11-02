#!/bin/sh

. /lib/functions/migrate.sh

[ -f "/etc/config/teltonika" ] || return 0

UAM_DOMAIN_FILE="/etc/chilli/uamdomainfile_chilli"
_INSTANCE="hotspot1"

json_load_file /etc/board.json
json_select modems
json_select 1
json_get_var _MODEM_ID id
json_cleanup

move_net_cb() {
	local option="$1"
	local old_sec="$2"
	local sec="$3"

	config_get net "$old_sec" net
	[ -z "$net" ] && return 1

	eval "$(ipcalc.sh "$net")";ip_net="$NETWORK"
	uci_set chilli "$sec" uamlisten "$IP"
	_OPTION_VALUE="${NETWORK}/${PREFIX}"

	return 1
}

move_auth_poto_cb() {
	return 1
}

move_network_cb() {
	local old_sec="$2"
	local id=${old_sec:1,7}

	_OPTION_VALUE="wifi${id}"

	return 1
}

move_limit() {
	local option="$1"
	local sec="$2"
	local units

	config_get id "$section" id
	[ "$id" != "$_INSTANCE" ] && return
	config_get units "$sec" "$option"
}

move_uamallowed_cb() {
	local section="$1"
	local domain instance enabled

	config_get domain "$section" domain
	config_get instance "$section" instance
	config_get enabled "$section" enabled 0
	[ "$enabled" = "0" ] && return 0
	[ -n "$domain" ] && [ "$instance" = "$_INSTANCE" ] && {
		domain="${domain##https://}"
		domain="${domain##http://}"
		echo "$domain" >> "$UAM_DOMAIN_FILE"
	}
}

move_u_limit_cb() {
	local sec="$2"
	move_limit u_bandwidth_unit "$sec"
}

move_d_limit_cb() {
	local sec="$2"
	move_limit d_bandwidth_unit "$sec"
}

move_mode_cb() {
	local option="$1" old_sec="$2" new_sec="$3"

	config_get mode "$old_sec" mode
	case "$mode" in
		norad)
			_OPTION_VALUE="local" ;;
		sms)
			_OPTION_VALUE="sms_otp"
			uci_set "chilli" "$new_sec" "modem" "$_MODEM_ID"
			uci_set "chilli" "$new_sec" "dynexpirationtime" "0"
			uci_set "chilli" "$new_sec" "_dyn_users_group" "default"
			;;
		extrad)
			_OPTION_VALUE="radius" ;;
		add)
			_OPTION_VALUE="mac_auth" ;;
		intrad)
			_OPTION_VALUE="local" ;;
		mac)
			_OPTION_VALUE="mac_auth" ;;
	esac

	return 1
}

success_page_cb() {
	local option="$1" old_sec="$2" new_sec="$3"
	local success_url success addvert_address _mode

	config_get success_url "$old_sec" "$option"
	config_get addvert_address "$old_sec" addvert_address
	config_get _mode "$old_sec" _mode
	if [ "$_mode" = "add" ] && [ -n "$addvert_address" ]; then
		success="custom"
		uci_set chilli "$new_sec" _success_url "$addvert_address"
	elif [ -n "$success_url" ]; then
		success="custom"
		uci_set chilli "$new_sec" _success_url "$success_url"
	else
		success="uam"
	fi

	uci_set chilli "$new_sec" _success "$success"
}

chilli_fix_general() {
	config_get profile "$1" profile

	[ "$profile" != "custom" ] && return
	uci_set chilli chilli _profile "default"
}

chilli_fix_users() {
	local section="$1"
	local name

	config_get id "$section" id
	[ "$id" != "$_INSTANCE" ] && return

	config_get template "$section" template "default"
	config_get username "$section" username
	config_get password "$section" password
	config_get name "$template" name
	[ "$name" = "unlimited" ] && name="default"

	uci_add chilli user
	uci_set chilli "$CONFIG_SECTION" group "$name"
	uci_set chilli "$CONFIG_SECTION" username "$username"
	uci_set chilli "$CONFIG_SECTION" password "$password"
}

chilli_fix_sessions() {
	local section="$1"

	config_get name "$section" name
	config_get id "$section" id
	config_get defidletimeout "$section" defidletimeout
	config_get defsessiontimeout "$section" defsessiontimeout
	config_get downloadbandwidth "$section" downloadbandwidth
	config_get uploadbandwidth "$section" uploadbandwidth
	config_get downloadlimit "$section" downloadlimit
	config_get uploadlimit "$section" uploadlimit
	config_get period "$section" period
	config_get weekday "$section" weekday

	uci_add chilli group
	[ "$id" = "$_INSTANCE" ] && [ "$name" = "unlimited" ] && \
		[ "${section:0:9}" = "unlimited" ] && name="default"
	uci_set chilli "$CONFIG_SECTION" name "$name"
	uci_set chilli "$CONFIG_SECTION" defidletimeout "$defidletimeout"
	uci_set chilli "$CONFIG_SECTION" defsessiontimeout "$defsessiontimeout"
	uci_set chilli "$CONFIG_SECTION" downloadbandwidth "$downloadbandwidth"
	uci_set chilli "$CONFIG_SECTION" uploadbandwidth "$uploadbandwidth"
	uci_set chilli "$CONFIG_SECTION" period "$period"
	uci_set chilli "$CONFIG_SECTION" weekday "$weekday"

	[ -n "$downloadlimit" ] && {
		downloadlimit=$(($downloadlimit / 1048576 * 1000000))
		uci_set chilli "$CONFIG_SECTION" downloadlimit "$downloadlimit"
	}

	[ -n "$uploadlimit" ] && {
		uploadlimit=$(($uploadlimit / 1048576 * 1000000))
		uci_set chilli "$CONFIG_SECTION" uploadlimit "$uploadlimit"
	}


}

rm /etc/config/landingpage
migrate "/etc/migrate.conf/chilli_migration.json"

config_load coovachilli

get_section_name _INSTANCE "general"
chilli_fix_general "$_INSTANCE"
config_foreach chilli_fix_users users
config_foreach chilli_fix_sessions session
config_foreach move_uamallowed_cb uamallowed

uci_commit chilli

#RM unused files
rm /etc/config/coovachilli
rm -r /etc/freeradius2
rm -r /etc/chilli/www/themes/
rm /etc/chilli/www/hotspotlogin.tmpl
rm /www/luci-static/resources/loginpage.css
