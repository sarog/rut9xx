#!/bin/bash
rm -r /tftpboot/luciwzd
cp -r applications/luci-cfgwzd /tftpboot/luciwzd

cp libs/web/luasrc/view/cbi/map.htm /tftpboot/map.htm