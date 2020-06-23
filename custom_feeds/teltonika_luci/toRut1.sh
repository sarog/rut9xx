#!/bin/bash
cp libs/core/luasrc/model/network.lua /tftpboot/networkRework/networkHuge.lua
cp modules/admin-full/luasrc/model/cbi/admin_network/ifaces.lua /tftpboot/networkRework/ifaces.lua
cp modules/admin-full/luasrc/model/cbi/admin_network/ifacesWan.lua /tftpboot/networkRework/ifacesWan.lua
cp modules/admin-full/luasrc/model/cbi/admin_network/ifacesLan.lua /tftpboot/networkRework/ifacesLan.lua
cp libs/web/luasrc/view/cbi/network_ifacelist.htm /tftpboot/networkRework/network_ifacelist.htm
