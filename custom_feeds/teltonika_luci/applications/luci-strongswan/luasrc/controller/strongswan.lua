--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: openvpn.lua 7362 2011-08-12 13:16:27Z jow $
]]--

module("luci.controller.strongswan", package.seeall)
local uci = require "luci.model.uci"
local _uci_real = uci.cursor()

function index()
	entry( {"admin", "services", "vpn", "ipsec"},  arcombine(cbi("strongswan_add"), cbi("strongswan_edit")), _("IPsec"), 2).leaf=true
	page = entry({"admin", "services", "vpn", "ipsec_delete"}, call("ipsec_delete"), nil)
	page.leaf = true
end


function ipsec_delete()
	local path  = luci.dispatcher.context.requestpath
	local ipsec = path[#path]
	if ipsec then
		_uci_real:delete("strongswan", ipsec)
		luci.http.redirect(luci.dispatcher.build_url("admin/services/vpn/ipsec"))
		_uci_real:commit("strongswan")
		luci.sys.call("/sbin/luci-reload & >/dev/null 2>/dev/null")
		return
	end
end
