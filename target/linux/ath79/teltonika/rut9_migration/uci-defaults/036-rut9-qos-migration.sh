#!/bin/sh

. /lib/functions.sh
. /lib/functions/network.sh

[ -f "/etc/config/teltonika" ] || return 0

config_load qos
config_get lan_enabled LAN enabled 0
config_get mob_enabled Mobile enabled 0

#~ We can't use qos on mobile so move to lan,
#~ should be no difference priorities still are same
[ "$mob_enabled" = 1 ] && {
	[ "$lan_enabled" = 0 ] && {
		uci_remove qos Mobile device
		uci_remove qos LAN
		uci_rename qos Mobile lan
	}
}

uci_remove qos Mobile
uci_commit qos

exit 0
