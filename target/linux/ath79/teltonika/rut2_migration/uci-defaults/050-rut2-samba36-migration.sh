#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || exit 0

rename() {
	local enable

	config_get enable "$section" "enable" ""
	[ -n "$enable" ] && uci_rename samba "$1" "enable" "enabled"
}

check_users() {
	local section=$1
	local username new_user_list

	config_get username "$section" "username" ""
	new_user_list=$(echo "$USERS_TO_ADD" | sed "/${username}/d" 2>/dev/null)
	USERS_TO_ADD=${new_user_list:-$USERS_TO_ADD}
}

config_load samba || exit 0
config_foreach rename

USERS_TO_ADD=$(cut -d':' -f1 /etc/samba/smbpasswd)
config_foreach check_users user

for username in $USERS_TO_ADD; do
	uci_add samba user
	uci_set samba "$CONFIG_SECTION" "username" "$username"
done

uci_commit
