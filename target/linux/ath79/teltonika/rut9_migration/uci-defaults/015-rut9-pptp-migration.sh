#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

fix_server() {
        [ "$1" = "pptpd" ] && {
                config_get name "$1" _name
                uci rename pptpd."$1"=server_"$name"
                uci -q delete pptpd.server_"$name".remoteip
                uci_set pptpd server_"$name" type server
        }
}

config_load pptpd
config_foreach fix_server service
uci commit pptpd
