--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: pavyzdys.lua 7362 2017-08-23 13:16:27Z jow $
]]--

module("luci.controller.zerotier", package.seeall)


function index()
	entry({"admin", "services", "vpn", "zerotier"}, alias("admin", "services", "vpn", "zerotier", "zerotier"), _(translate("ZeroTier")), 8)
	entry({"admin", "services", "vpn","zerotier", "zerotier"}, cbi("zerotier"), _("ZeroTier General"), 1).leaf = true
	entry({"admin", "services", "vpn","zerotier", "zerotiervpn"}, cbi("zerotiervpn"), _("ZeroTier VPN"), 2).leaf = true
end


