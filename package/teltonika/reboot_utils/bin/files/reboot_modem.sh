#!/bin/sh

. /usr/share/libubox/jshn.sh

MODEM_ID=""

get_modem() {
    local modem modems id builtin primary
    local primary_modem=""
    local builtin_modem=""
    json_init
    json_load "$(cat /etc/board.json)"
    json_get_keys modems modems
    json_select modems

    for modem in $modems; do
        json_select "$modem"
        json_get_vars builtin primary id
        if [ "$builtin" -a "$id" ]; then
            builtin_modem=$id
        fi
        if [ "$primary" -a "$id" ]; then
            primary_modem=$id
            break
        fi
        json_select ..
    done

    if [ "$primary_modem" = "" ]; then
        if [ "$builtin_modem" = "" ]; then
            json_load "$(/bin/ubus call gsmd get_modems)"
            json_get_keys modems modems
            json_select modems

            for modem in $modems; do
                json_select "$modem"
                json_get_vars id
                primary_modem=$id
                break
            done
        else
            primary_modem=$builtin_modem
        fi
    fi
    MODEM_ID=$primary_modem
}

if [[ -z "$1" ]]; then
	get_modem
else
	MODEM_ID="$1"
fi

/bin/ubus call gsmd reinit_modems "{\"id\":\"$MODEM_ID\"}"
