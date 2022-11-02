#!/bin/sh

[ -f "/etc/config/teltonika" ] || return 0

cp /rom/etc/config/rpcd /etc/config/rpcd


