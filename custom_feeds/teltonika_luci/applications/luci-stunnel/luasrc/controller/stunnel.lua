--[[
LuCI - Lua Configuration Interface

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
$Id: stunnel.lua 7362 2011-08-12 13:16:27Z $
]]--

module("luci.controller.stunnel", package.seeall)
local uci = require "luci.model.uci"
local _uci_real = uci.cursor()

function index()
	entry( {"admin", "services", "vpn", "stunnel"},  arcombine(cbi("stunnel"), cbi("stunnel_edit")), _("Stunnel"), 7).leaf=true
	page = entry({"admin", "services", "vpn", "stunnel_delete"}, call("stun_delete"), nil)
	page.leaf = true
end


function stun_delete()
	local path  = luci.dispatcher.context.requestpath
	local ipsec = path[#path]
	if ipsec then
		_uci_real:delete("stunnel", ipsec)
		luci.http.redirect(luci.dispatcher.build_url("admin/services/vpn/stunnel"))
		_uci_real:commit("stunnel")
		luci.sys.call("/sbin/luci-reload & >/dev/null 2>/dev/null")
		return
	end
end