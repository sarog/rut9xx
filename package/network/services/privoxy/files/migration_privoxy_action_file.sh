#!/bin/sh

. /lib/functions.sh

LUCI_ACTION_FILE=/etc/privoxy/user_action_luci

copy_urls() {
	local section=$1

	# Loop over lines in LUCI_ACTION_FILE one by one
	while IFS="" read -r line || [ -n "$line" ]; do
		uci_add_list privoxy "$section" _url "$line"
	done <"$LUCI_ACTION_FILE"
}

[ -f "$LUCI_ACTION_FILE" ] || exit 0

config_load privoxy || exit 0
config_foreach copy_urls "privoxy"
uci_commit

rm "$LUCI_ACTION_FILE"
