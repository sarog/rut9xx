#!/bin/sh

. /lib/functions.sh
. /lib/config/uci.sh

login_cb() {
    local sec="$1"
    local username password

    config_get username "$sec" username
    [ "$username" != "admin" ] && return

    config_get password "$sec" password
    [ "$username" != "$p$root" ] && \
        uci_set rpcd "$sec" password "\$p\$admin"
}

config_load rpcd
config_foreach login_cb login
config_foreach login_cb superuser
uci_commit

exit 0
