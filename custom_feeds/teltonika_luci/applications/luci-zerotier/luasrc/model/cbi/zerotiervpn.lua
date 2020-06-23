--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: pavyzdys.lua 6984 2017-08-22 15:14:42Z soma $
]]--
local l=require"luci.sys"
map = Map("zerotier", translate("ZeroTier-One"))

x = uci.cursor()

v = map:section(NamedSection, "zerotier", "ZeroTier VPN", translate("ZeroTier VPN"))
v.addremove = false

vpnflag = v:option(Flag, "vpnenabled", translate("Enable VPN"), translate("Enable VPN"))

mode = v:option(ListValue, "mode", translate("Mode"), translate("Is this a server or a client"))
mode:value("server", translate("Server"))
mode:value("client", translate("Client"))

dummy = v:option(DummyValue, "getinfo_ip_source_status", translate(""))
dummy.default = translate("Use this option when multiwan is off")
dummy:depends("mode", "client")

server = v:option(ListValue, "selectedNetwork", "Connect to", translate("Connect to"))
server:depends("mode", "client")

local join = x:get("zerotier","zerotier","join")
if join then
	for _,i in ipairs(x:get("zerotier","zerotier","join")) do
		server:value(i, i)
	end
end

return map
