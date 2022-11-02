#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0
[ -f "/etc/config/zerotier" ] || return 0

move_option() {
        local option="$1"
        local new_option="$2"
        local section="$3"

        config_get value "$section" "$option"
        [ -n "$value" ] || return 0

        uci_set zerotier "$section" "$new_option" "$value"
        uci delete zerotier."$section"."$option"
}

fix_zerotier() {
        local section="$1"

        move_option address node_id "$section"

        config_get value "$section" enabled
        [ -n "$value" ] || {
                uci_set zerotier "$section" enabled '1'
        }
}

config_load zerotier
config_foreach fix_zerotier zerotier
uci commit zerotier
