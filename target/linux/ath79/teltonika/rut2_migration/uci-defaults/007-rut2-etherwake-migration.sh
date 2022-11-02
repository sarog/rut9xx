#!/bin/sh

. /lib/config/uci.sh

[ -f "/etc/config/teltonika" ] || return 0

uci_remove etherwake setup interface
uci_commit etherwake
