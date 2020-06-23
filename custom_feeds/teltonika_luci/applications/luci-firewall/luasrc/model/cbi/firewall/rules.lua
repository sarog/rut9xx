--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: rules.lua 8117 2011-12-20 03:14:54Z jow $
]]--

local ds = require "luci.dispatcher"
local ft = require "luci.tools.firewall"

m = Map("firewall",
	translate("Firewall - Traffic Rules"),
	translate("Traffic rules define policies for packets traveling between different zones, for example to reject traffic between certain hosts or to open WAN ports on the router."))

--
-- Rules
--

s = m:section(TypedSection, "rule", translate("Traffic Rules"))
s.addremove = true
s.anonymous = true
s.sortable  = true
s.sorthint  = translate("All rules are executed in current list order")
s.template = "cbi/tblsection"
s.extedit   = ds.build_url("admin/network/firewall/rules/%s")
s.defaults.target = "ACCEPT"
s.template_addremove = "firewall/cbi_addrule"
s.novaluetext = translate("There are no traffic rules created yet")


function s.create(self, section)
	created = TypedSection.create(self, section)
end

function s.parse(self, ...)
	TypedSection.parse(self, ...)

	local i_n = m:formvalue("_newopen.name")
	local i_p = m:formvalue("_newopen.proto")
	local i_e = m:formvalue("_newopen.extport")
	local i_x = m:formvalue("_newopen.submit")

	local f_n = m:formvalue("_newfwd.name")
	local f_s = m:formvalue("_newfwd.src")
	local f_d = m:formvalue("_newfwd.dest")
	local f_x = m:formvalue("_newfwd.submit")

	if i_x then
		created = TypedSection.create(self, section)

		self.map:set(created, "target",    "ACCEPT")
		self.map:set(created, "src",       "wan")
		self.map:set(created, "proto",     (i_p ~= "other") and i_p or "all")
		self.map:set(created, "dest_port", i_e)
		self.map:set(created, "name",      i_n)

		if i_p ~= "other" and i_e and #i_e > 0 then
			created = nil
		end

	elseif f_x then
		created = TypedSection.create(self, section)

		self.map:set(created, "target", "ACCEPT")
		self.map:set(created, "src",    f_s)
		self.map:set(created, "proto", "tcp udp")
		self.map:set(created, "dest",   f_d)
		self.map:set(created, "name",   f_n)
	end

	if created then
		m.uci:save("firewall")
		luci.http.redirect(ds.build_url(
			"admin/network/firewall/rules", created
		))
	end
end

ft.opt_name(s, DummyValue, translate("Name"), translate("Name of the rule. Used for easier rules management purpose only"))
ft.width   = "1%"


proto = s:option(DummyValue, "proto", translate("Protocol"), translate("Protocol type of incoming or outgoing packet"))
proto.rawhtml = true
proto.width   = "20%"
function proto.cfgvalue(self, s)
	return ft.fmt_proto(self.map:get(s, "proto"), self.map:get(s, "icmp_type"))
		or "All"
end

src = s:option(DummyValue, "src", translate("Source"), translate("Match incoming traffic from this IP or range only"))
src.rawhtml = true
src.width   = "10%"
function src.cfgvalue(self, s)
	local z = ft.fmt_zone(self.map:get(s, "src"), translate("any zone"))
	local a = ft.fmt_ip(self.map:get(s, "src_ip"), translate("any host"))
	local p = ft.fmt_port(self.map:get(s, "src_port"))
	local m = ft.fmt_mac(self.map:get(s, "src_mac"))

	if p and m then
		return translatef("From %s in %s with source %s and %s", a, z, p, m)
	elseif p or m then
		return translatef("From %s in %s with source %s", a, z, p or m)
	else
		return translatef("From %s in %s", a, z)
	end
end

dest = s:option(DummyValue, "dest", translate("Destination"), translate("Redirect matched traffic to the the given IP address and destination port"))
dest.rawhtml = true
dest.width   = "10%"
function dest.cfgvalue(self, s)
	local z = ft.fmt_zone(self.map:get(s, "dest"))
	local p = ft.fmt_port(self.map:get(s, "dest_port"))

	-- Forward
	if z then
		local a = ft.fmt_ip(self.map:get(s, "dest_ip"), translate("any host"))
		if p then
			return translatef("To %s, %s in %s", a, p, z)
		else
			return translatef("To %s in %s", a, z)
		end

	-- Input
	else
		local a = ft.fmt_ip(self.map:get(s, "dest_ip"),
			translate("any router IP"))

		if p then
			return translatef("To %s at %s on <var>this device</var>", a, p)
		else
			return translatef("To %s on <var>this device</var>", a)
		end
	end
end


target = s:option(DummyValue, "target", translate("Action"), translate("Action to be taken for the packet if it matches the rule"))
target.rawhtml = true
target.width   = "1%"
function target.cfgvalue(self, s)
	local t = ft.fmt_target(self.map:get(s, "target"), self.map:get(s, "dest"))
	local l = ft.fmt_limit(self.map:get(s, "limit"),
		self.map:get(s, "limit_burst"))

	if l then
		return translatef("<var>%s</var> and limit to %s", t, l)
	else
		return "<var>%s</var>" % t
	end
end

ft.opt_enabled(s, Flag, translate("Enable"), translate("Make a rule active/inactive")).width = "1%"


--
-- SNAT
--

s = m:section(TypedSection, "redirect",
	translate("Source NAT"),
	translate("Source NAT is a specific form of masquerading which allows fine grained control over the source IP used for outgoing traffic, for example to map multiple WAN addresses to internal subnets."))
s.template  = "cbi/tblsection"
s.novaluetext = translate("There are no source NAT rules created yet")
s.addremove = true
s.anonymous = true
s.sorthint  = translate("All rules are executed in current list order")
s.extedit   = ds.build_url("admin/network/firewall/rules/%s")
s.template_addremove = "firewall/cbi_addsnat"

function s.create(self, section)
	created = TypedSection.create(self, section)
end

function s.parse(self, ...)
	TypedSection.parse(self, ...)

	local n = m:formvalue("_newsnat.name")
	local s = m:formvalue("_newsnat.src")
	local d = m:formvalue("_newsnat.dest")
	local a = m:formvalue("_newsnat.dip")
	local p = m:formvalue("_newsnat.dport")
	local x = m:formvalue("_newsnat.submit")

	if x and a and #a > 0 then
		created = TypedSection.create(self, section)

		self.map:set(created, "target",    "SNAT")
		self.map:set(created, "src",       s)
		self.map:set(created, "dest",      d)
		self.map:set(created, "proto",     "all")
		self.map:set(created, "src_dip",   a)
		self.map:set(created, "src_dport", p)
		self.map:set(created, "name",      n)
	end

	if created then
		m.uci:save("firewall")
		luci.http.redirect(ds.build_url(
			"admin/network/firewall/rules", created
		))
	end
end

function s.filter(self, sid)
	return (self.map:get(sid, "target") == "SNAT")
end

ft.opt_name(s, DummyValue, translate("Name"), translate("Name of the rule. Used for easier rules management purpose only"))

proto = s:option(DummyValue, "proto", translate("Protocol"), translate("Protocol type of incoming or outgoing packet"))
proto.rawhtml = true
proto.width   = "10%"
function proto.cfgvalue(self, s)
	return ft.fmt_proto(self.map:get(s, "proto")) or "All"
end


src = s:option(DummyValue, "src", translate("Source"), translate("Match incoming traffic from this IP or range only"))
src.rawhtml = true
src.width   = "10%"
function src.cfgvalue(self, s)
	local z = ft.fmt_zone(self.map:get(s, "src"), translate("any zone"))
	local a = ft.fmt_ip(self.map:get(s, "src_ip"), translate("any host"))
	local p = ft.fmt_port(self.map:get(s, "src_port"))
	local m = ft.fmt_mac(self.map:get(s, "src_mac"))

	if p and m then
		return translatef("From %s in %s with source %s and %s", a, z, p, m)
	elseif p or m then
		return translatef("From %s in %s with source %s", a, z, p or m)
	else
		return translatef("From %s in %s", a, z)
	end
end

dest = s:option(DummyValue, "dest", translate("Destination"), translate("Redirect matched traffic to the the given IP address and destination port"))
dest.rawhtml = true
dest.width   = "25%"
function dest.cfgvalue(self, s)
	local z = ft.fmt_zone(self.map:get(s, "dest"), translate("any zone"))
	local a = ft.fmt_ip(self.map:get(s, "dest_ip"), translate("any host"))
	local p = ft.fmt_port(self.map:get(s, "dest_port")) or
		ft.fmt_port(self.map:get(s, "src_dport"))

	if p then
		return translatef("To %s, %s in %s", a, p, z)
	else
		return translatef("To %s in %s", a, z)
	end
end

snat = s:option(DummyValue, "via", translate("SNAT"), translate("SNAT (Source Network Address Translation) rewrite packet\\'s source IP address and port"))
snat.rawhtml = true
snat.width   = "10%"
function snat.cfgvalue(self, s)
	local a = ft.fmt_ip(self.map:get(s, "src_dip"))
	local p = ft.fmt_port(self.map:get(s, "src_dport"))

	if a and p then
		return translatef("Rewrite to source %s, %s", a, p)
	elseif a then
		return translatef("Rewrite to source %s", a)
	elseif p then
		return translatef("Rewrite to source %s", p)
	else
		return translate("Rewrite to source")
	end
end

ft.opt_enabled(s, Flag, translate("Enable"), translate("Make a rule active/inactive")).width = "10%"


return m
