--[[
LuCI - Lua Configuration Interface

Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: forward-details.lua 8117 2011-12-20 03:14:54Z jow $
]]--

local sys = require "luci.sys"
local dsp = require "luci.dispatcher"
local uci = require "luci.model.uci".cursor()
require "teltonika_lua_functions"
local m, s, o

local VPN_INST

if arg[1] then
	VPN_INST = arg[1]
else
	return nil
end

m = Map("bird4", translatef("Static Route %s",VPN_INST), translate("Route configuration"))

m.redirect = dsp.build_url("admin/network/routes/dynamic_routes/gen_proto/")
if m.uci:get("bird4", arg[1]) ~= "route" then
	luci.http.redirect(m.redirect)
	return
else
	local name = m:get(arg[1], "prefix") or m:get(arg[1], "_prefix")
	if not name or #name == 0 then
		name = translate("(Unnamed Entry)")
	end
	m.title = escapeHTML("%s - %s" %{ translate("Static Route"), name })
end

local wan_zone = nil

sect_routes = m:section(NamedSection, arg[1], "route", "")
sect_routes.anonymous = true
sect_routes.addremove = false

disabled = sect_routes:option(Flag, "disabled", translate("Disabled"), translate("If this option is true, the protocol will not be configured."))
disabled.rmempty = false
disabled.default=1

instance = sect_routes:option(ListValue, "instance", translate("Route instance"), translate(""))
i = 0

uci:foreach("bird4", "static",
	function (s)
		instance:value(s[".name"])
	end)

prefix = sect_routes:option(Value, "prefix", translate("Route prefix"), translate(""))

type = sect_routes:option(ListValue, "type", translate("Type of route"), translate(""))
type:value("router")
type:value("special")
type:value("iface")
type:value("recursive")
type:value("multipath")

valueVia = sect_routes:option(Value, "via", translate("Via"), translate(""))
valueVia.optional = false
valueVia:depends("type", "router")
valueVia.datatype = "ip4addr"

listVia = sect_routes:option(DynamicList, "l_via", translate("Via"), translate(""))
listVia:depends("type", "multipath")
listVia.optional=false
listVia.datatype = "ip4addr"

attribute = sect_routes:option(Value, "attribute", translate("Attribute"), translate("Types are: unreachable, prohibit and blackhole"))
attribute:depends("type", "special")

iface  = sect_routes:option(ListValue, "iface", translate("Interface"), translate(""))
iface:depends("type", "iface")

uci:foreach("wireless", "wifi-iface",
	function(section)
		iface:value(section[".name"])
	end)

ip =  sect_routes:option(Value, "ip", translate("IP address"), translate(""))
ip:depends("type", "ip")
ip.datatype = [[ or"ip4addr", "ip6addr" ]]

reject = sect_routes:option(Flag, "reject", translate("Reject"), translate(""))
reject.rmempty = false
reject.default=1

return m
