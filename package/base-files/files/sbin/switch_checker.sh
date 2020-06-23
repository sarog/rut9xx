#!/bin/sh

lock /tmp/sim_switch_protect_tmp

need_restore=0
is_switching_enabled=$(uci -q get sim_switch.sim_switch.enabled)

if [ $is_switching_enabled -a $is_switching_enabled -eq 1 ]; then
	need_restore=1
	`uci -q set sim_switch.sim_switch.enabled=0`
	`uci -q commit sim_switch`
fi

`/usr/sbin/gsmctl -A AT+COPS=? >/tmp/operators`

if [ $need_restore -eq 1 ]; then
	need_restore=0
	`uci -q set sim_switch.sim_switch.enabled=1`
	`uci -q commit sim_switch`
fi

lock -u /tmp/sim_switch_protect_tmp
