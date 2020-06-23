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
local ft  = require "luci.tools.firewall"
require "teltonika_lua_functions"
local m, s, o

arg[1] = arg[1] or ""

m = Map("firewall",
	translate("Firewall - Port Forwarding"),
	translate("This page allows you to change advanced properties of the port forwarding entry. Although, in most cases there is no need to modify those settings."))

m.redirect = dsp.build_url("admin/network/firewall/forwards")

if m.uci:get("firewall", arg[1]) ~= "redirect" then
	luci.http.redirect(m.redirect)
	return
else
	local name = m:get(arg[1], "name") or m:get(arg[1], "_name")
	if not name or #name == 0 then
		name = translate("(Unnamed Entry)")
	end
	m.title = escapeHTML("%s - %s" %{ translate("Firewall - Port Forwards"), name })
end

local wan_zone = nil

m.uci:foreach("firewall", "zone",
	function(s)
		local n = s.network or s.name
		if n then
			local i
			for i in n:gmatch("%S+") do
				if i == "wan" then
					wan_zone = s.name
					return false
				end
			end
		end
	end)

s = m:section(NamedSection, arg[1], "redirect", "")
s.anonymous = true
s.addremove = false

-- ft.opt_enabled(s, Button)
o = s:option(Flag, "enabled", translate("Enable"), translate("To enable port forward rule configuration"))
ft.opt_name(s, Value, translate("Name"), translate("Name of the rule. Used for easier rules management purpose only"))

o.rmempty = true
o.default = o.enabled

o = s:option(Value, "proto", translate("Protocol"), translate("You may specify multiple by selecting (custom) and then entering protocols separated by space"))
o:value("tcp udp", translate("TCP+UDP"))
o:value("tcp", translate("TCP"))
o:value("udp", translate("UDP"))
o:value("icmp", translate("ICMP"))

function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	if not v or v == "tcpudp" then
		return "tcp udp"
	end
	return v
end


o = s:option(Value, "src", translate("Source zone"), translate("Match incoming traffic from this zone only"))
o.nocreate = true
o.default = "wan"
o.template = "cbi/firewall_zonelist"


o = s:option(DynamicList, "src_mac",
	translate("Source MAC address"),
	translate("Match incoming traffic from these MACs only"))
o.rmempty = true
o.datatype = "macaddr"
o.placeholder = translate("any")


o = s:option(Value, "src_ip",
	translate("Source IP address"),
	translate("Match incoming traffic from this IP or range only"))
o.rmempty = true
o.datatype = "neg(ip4addr)"
o.placeholder = translate("any")


o = s:option(Value, "src_port",
	translate("Source port"),
	translate("Match incoming traffic originating from the given source port or port range on the client host only"))
o.rmempty = true
o.datatype = "portrange"
o.placeholder = translate("any")


o = s:option(Value, "src_dip",
	translate("External IP address"),
	translate("Match incoming traffic directed at the given IP address only"))

o.rmempty = true
o.datatype = "ip4addr"
o.placeholder = translate("any")


o = s:option(Value, "src_dport", translate("External port"),
	translate("Match incoming traffic directed at the given destination port or port range on this host only"))
o.datatype = "portrange"



o = s:option(Value, "dest", translate("Internal zone"), translate("Redirect matched incoming traffic to the specified internal zone"))
o.nocreate = true
o.default = "lan"
o.template = "cbi/firewall_zonelist"


o = s:option(Value, "dest_ip", translate("Internal IP address"),
	translate("Redirect matched incoming traffic to the specified internal host"))
o.datatype = "ip4addr"
a={}
for i, dataset in ipairs(sys.net.arptable()) do
	a[#a+1]=dataset["IP address"]
end

table.sort(a)
for i, dataset in ipairs(a) do
	o:value(dataset)
end

o = s:option(Value, "dest_port",
	translate("Internal port"),
	translate("Redirect matched incoming traffic to the given port on the internal host"))
o.placeholder = translate("any")
o.datatype = "portrange"


o = s:option(Flag, "reflection", translate("Enable NAT loopback"), translate("NAT loopback enables your local network (i.e. behind your router/modem) to connect to a forward-facing IP address (such as 208.112.93.73) of a machine that it also on your local network"))
o.rmempty = true
o.default = o.enabled
o:depends("src", wan_zone)
o.cfgvalue = function(...)
	return Flag.cfgvalue(...) or "1"
end


s:option(Value, "extra",
	translate("Extra arguments"),
	translate("Passes additional arguments to iptables. Use with care!"))


return m
