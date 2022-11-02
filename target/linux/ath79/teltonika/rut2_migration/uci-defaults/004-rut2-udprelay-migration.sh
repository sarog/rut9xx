#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

uci_rename udprelay br_lan lan
uci_commit udprelay

exit 0
