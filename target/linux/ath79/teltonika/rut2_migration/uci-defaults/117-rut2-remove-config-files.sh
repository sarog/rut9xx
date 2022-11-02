#!/bin/sh

[ -f "/etc/config/teltonika" ] || return 0

rm /etc/config/auto_update /etc/config/racoon \
    /etc/config/sim_idle_protection /etc/config/sim_switch \
    /etc/config/gps