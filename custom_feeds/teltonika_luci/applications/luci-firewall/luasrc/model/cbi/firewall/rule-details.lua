--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: rule-details.lua 8169 2012-01-09 05:48:27Z jow $
]]--

local sys = require "luci.sys"
local dsp = require "luci.dispatcher"
local nxo = require "nixio"

local ft = require "luci.tools.firewall"
local nw = require "luci.model.network"
require "teltonika_lua_functions"
local m, s, o, k, v

arg[1] = arg[1] or ""

m = Map("firewall",
	translate("Firewall - Traffic Rules"),
	translate("This page allows you to change advanced properties of the traffic rule entry, such as matched source and destination hosts."))

m.redirect = dsp.build_url("admin/network/firewall/rules")

nw.init(m.uci)

local rule_type = m.uci:get("firewall", arg[1])
if rule_type == "redirect" and m:get(arg[1], "target") ~= "SNAT" then
	rule_type = nil
end

if not rule_type then
	luci.http.redirect(m.redirect)
	return

--
-- SNAT
--
elseif rule_type == "redirect" then

	local name = m:get(arg[1], "name") or m:get(arg[1], "_name")
	if not name or #name == 0 then
		name = translate("(Unnamed SNAT)")
	else
		name = "SNAT %s" % name
	end

	m.title = escapeHTML("%s - %s" %{ translate("Firewall - Traffic Rules"), name })

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


	--ft.opt_enabled(s, Button)
	--o = s:option(Flag, "enabled", translate("Enable"), translate("To enable traffic rule configuration"))
	ft.opt_enabled(s, Flag, translate("Enable"), translate("Make a rule active/inactive")).width = "10%"
	ft.opt_name(s, Value, translate("Name"), translate("Name of the rule. Used for easier rules management purpose only"))


	o = s:option(Value, "proto",
		translate("Protocol"),
		translate("You may specify multiple by selecting (custom) and then entering protocols separated by space"))

	o:value("all", translate("All protocols"))
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


	o = s:option(DynamicList, "src_mac", translate("Source MAC address"), translate("Match incoming traffic from these MACs only"))
	o.rmempty = true
	o.datatype = "neg(macaddr)"
	o.placeholder = translate("any")


	o = s:option(Value, "src_ip", translate("Source IP address"), translate("Match incoming traffic from this IP or range only"))
	o.rmempty = true
	o.datatype = "neg(ipaddr)"
	o.placeholder = translate("any")


	o = s:option(Value, "src_port",
		translate("Source port"),
		translate("Match incoming traffic originating from the given source port or port range on the client host only"))
	o.rmempty = true
	o.datatype = "neg(portrange)"
	o.placeholder = translate("any")


	o = s:option(Value, "dest", translate("Destination zone"), translate("Match forwarded traffic to the given destination zone only"))
	o.nocreate = true
	o.default = "lan"
	o.template = "cbi/firewall_zonelist"


	o = s:option(Value, "dest_ip", translate("Destination IP address"), translate("Match forwarded traffic from this IP or range only"))
	o.datatype = "neg(ip4addr)"

	for i, dataset in ipairs(luci.sys.net.arptable()) do
		o:value(dataset["IP address"])
	end


	o = s:option(Value, "dest_port",
		translate("Destination port"),
		translate("Match forwarded traffic to the given destination port or port range only"))

	o.rmempty = true
	o.placeholder = translate("any")
	o.datatype = "portrange"


	o = s:option(Value, "src_dip",
		translate("SNAT IP address"),
		translate("Rewrite matched traffic to the given IP address"))
	o.rmempty = false
	o.datatype = "ip4addr"

	for k, v in ipairs(nw:get_interfaces()) do
		local a
		for k, a in ipairs(v:ipaddrs()) do
			o:value(a:host():string(), '%s (%s)' %{
				a:host():string(), v:shortname()
			})
		end
	end


	o = s:option(Value, "src_dport", translate("SNAT port"),
		translate("Rewrite matched traffic to the given source port. May be left empty to only rewrite the IP address"))
	o.datatype = "portrange"
	o.rmempty = true
	o.placeholder = translate('Do not rewrite')


	s:option(Value, "extra",
		translate("Extra arguments"),
		translate("Passes additional arguments to iptables. Use with care!"))


--
-- Rule
--
else
	local name = m:get(arg[1], "name") or m:get(arg[1], "_name")
	if not name or #name == 0 then
		name = translate("(Unnamed Rule)")
	end

	m.title = escapeHTML("%s - %s" %{ translate("Firewall - Traffic Rules"), name })


	s = m:section(NamedSection, arg[1], "rule", "")
	s.anonymous = true
	s.addremove = false


	-- o = s:option(Flag, "enabled", translate("Enable"), translate("To enable traffic rule configuration"))
	ft.opt_enabled(s, Flag, translate("Enable"), translate("To enable traffic rule configuration")).width = "18%"
	ft.opt_name(s, Value, translate("Name"), translate("Name of the rule. Used for easier rules management purpose only"))


	o = s:option(ListValue, "family", translate("Restrict to address family"), translate("Match traffic from selected address family only"))
	o.rmempty = true
	o:value("", translate("IPv4 and IPv6"))
	o:value("ipv4", translate("IPv4 only"))
	o:value("ipv6", translate("IPv6 only"))


	o = s:option(Value, "proto", translate("Protocol"), translate("You may specify multiple by selecting (custom) and then entering protocols separated by space"))
	o:value("all", translate("All"))
	o:value("tcp udp", translate("TCP, UDP"))
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


	o = s:option(DynamicList, "icmp_type", translate("Match ICMP type"), translate("Match traffic with selected ICMP type only"))
	o:value("", translate("any"))
	o:value("echo-reply")
	o:value("destination-unreachable")
	o:value("network-unreachable")
	o:value("host-unreachable")
	o:value("protocol-unreachable")
	o:value("port-unreachable")
	o:value("fragmentation-needed")
	o:value("source-route-failed")
	o:value("network-unknown")
	o:value("host-unknown")
	o:value("network-prohibited")
	o:value("host-prohibited")
	o:value("TOS-network-unreachable")
	o:value("TOS-host-unreachable")
	o:value("communication-prohibited")
	o:value("host-precedence-violation")
	o:value("precedence-cutoff")
	o:value("source-quench")
	o:value("redirect")
	o:value("network-redirect")
	o:value("host-redirect")
	o:value("TOS-network-redirect")
	o:value("TOS-host-redirect")
	o:value("echo-request")
	o:value("router-advertisement")
	o:value("router-solicitation")
	o:value("time-exceeded")
	o:value("ttl-zero-during-transit")
	o:value("ttl-zero-during-reassembly")
	o:value("parameter-problem")
	o:value("ip-header-bad")
	o:value("required-option-missing")
	o:value("timestamp-request")
	o:value("timestamp-reply")
	o:value("address-mask-request")
	o:value("address-mask-reply")


	o = s:option(Value, "src", translate("Source zone"), translate("Match incoming traffic from this zone only"))
	o.nocreate = true
	o.allowany = true
	o.default = "wan"
	o.template = "cbi/firewall_zonelist"


	o = s:option(Value, "src_mac", translate("Source MAC address"), translate("Match incoming traffic from these MACs only"))
	o.datatype = "list(macaddr)"
	o.placeholder = translate("any")


	o = s:option(Value, "src_ip", translate("Source address"), translate("Match incoming traffic from this IP or range only"))
	o.datatype = "list(neg(ipaddr))"
	o.placeholder = translate("any")


	o = s:option(Value, "src_port", translate("Source port"), translate("Match incoming traffic originating from the given source port or port range on the client host only"))
	o.datatype = "list(neg(portrange))"
	o.placeholder = translate("any")


	o = s:option(Value, "dest", translate("Destination zone"), translate("Match forwarded traffic to the given destination zone only"))
	o.nocreate = true
	o.allowany = true
	o.allowlocal = true
	o.template = "cbi/firewall_zonelist"


	o = s:option(Value, "dest_ip", translate("Destination address"), translate("Match forwarded traffic to the given destination IP address or IP range only"))
	o.datatype = "list(neg(ipaddr))"
	o.placeholder = translate("any")


	o = s:option(Value, "dest_port", translate("Destination port"), translate("Match forwarded traffic to the given destination port or port range only"))
	o.datatype = "list(neg(portrange))"
	o.placeholder = translate("any")
        o:depends("proto", "tcp udp")
        o:depends("proto", "udp")
        o:depends("proto", "tcp")


	o = s:option(ListValue, "target", translate("Action"), translate("Perform action for matched packet"))
	o.default = "ACCEPT"
	o:value("DROP", translate("drop"))
	o:value("ACCEPT", translate("accept"))
	o:value("REJECT", translate("reject"))


	s:option(Value, "extra",
		translate("Extra arguments"),
		translate("Passes additional arguments to iptables. Use with care!"))
end

return m
