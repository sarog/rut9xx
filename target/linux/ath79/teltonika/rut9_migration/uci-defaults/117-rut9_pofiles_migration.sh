#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

check_profile() {
	local archive id
	config_get archive "$1" archive
	config_get id "$1" id

	[ "$archive" = "$2" ] && created=1
	[ "$id" -gt "$ID" ] && ID="$id"
}

create_section() {
	file=$(echo "$1" | awk -F "." '{print $1}')
	time=$(echo "$file" | awk -F "_" '{print $NF}')
	name=$(echo "$file" | awk -F "_$time" '{print $1}')

	uci_add profiles profile "$name"
	uci_set profiles "$name" archive "$1"
	uci_set profiles "$name" updated "$time"
	uci_set profiles "$name" md5file "$file.md5"
	uci_set profiles "$name" id "$((ID + 1))"
	uci_commit profiles
}

profiles=$(ls /etc/profiles | grep ".tar.gz")
config_load profiles
ID=0

for profile in $profiles; do
	[ "$profile" = "default.tar.gz" -o "$profile" = "template.tar.gz" ] && continue
	created=0
	config_foreach check_profile profile "$profile"
	[ "$created" -eq 0 ] && create_section "$profile"
done

exit 0
