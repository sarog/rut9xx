#!/bin/sh

. /lib/functions.sh

[ -f "/etc/config/teltonika" ] || return 0

config_load mosquitto
config_get tls_type mqtt tls_type
[ -n "$tls_type" ] || uci_set mosquitto mqtt tls_type "cert"

config_get bridge_protocol_version mqtt bridge_protocol_version
[ -n "$tls_type" ] || uci_set mosquitto mqtt bridge_protocol_version "mqttv31"

uci_commit mosquitto
