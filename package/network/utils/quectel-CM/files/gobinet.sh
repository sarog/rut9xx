#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
    . /lib/functions.sh
    . ../netifd-proto.sh
    init_proto "$@"
}

proto_gobinet_init_config() {
    available=1
    no_device=1
    proto_config_add_string "device:device"
    proto_config_add_string apn
    proto_config_add_string pdp
    proto_config_add_defaults
}

proto_gobinet_setup() {
    local config="$1"
    local dataformat connstat
    local device apn auth username password pincode pdp $PROTO_DEFAULT_OPTIONS

    json_get_vars apn pdp $PROTO_DEFAULT_OPTIONS

    [ -n "$ctl_device" ] && device=$ctl_device

    proto_run_command "$config" quectel-CM ${apn:+-s $apn} ${device:+-i $device} ${config:+-c $config} ${pdp:+-n $pdp}
}

proto_gobinet_teardown() {
    local config="$1"
    echo "Stopping network $config"
    proto_init_update "*" 0 0
    proto_send_update "$config"
}

[ -n "$INCLUDE_ONLY" ] || {
    add_protocol gobinet
}
