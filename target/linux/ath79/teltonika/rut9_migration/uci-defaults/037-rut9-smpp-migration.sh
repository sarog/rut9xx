#!/bin/sh

[ -e "/etc/config/teltonika" ] || return

mv /etc/config/smpp_config /etc/config/smpp
