#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

move_hosts() {
	local file="$1"

	while read host; do
  		uci_add hostblock block
  		uci_set hostblock "$CONFIG_SECTION" enabled "1"
  		uci_set hostblock "$CONFIG_SECTION" host "$host"
	done < "$file"
}

config_load hostblock
config_get site_blocking_hosts config site_blocking_hosts

[ -n "$site_blocking_hosts" ] && {
	move_hosts "$site_blocking_hosts"
	uci_remove hostblock config site_blocking_hosts
	rm -f "$site_blocking_hosts"
}
