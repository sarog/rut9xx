--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: forwards.lua 8117 2011-12-20 03:14:54Z jow $
]]--
local uci = require "luci.model.uci".cursor()
local ds = require "luci.dispatcher"
local ft = require "luci.tools.firewall"

m = Map("firewall", translate("Firewall - Port Forwarding"),
	translate("Port forwarding allows remote computers on the Internet to connect to a specific computer or service within the private LAN."))

--
-- Port Forwards
--

s = m:section(TypedSection, "redirect", translate("Port Forwarding Rules"))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.sortable  = true
s.sorthint  = translate("All rules are executed in current list order")
s.extedit   = ds.build_url("admin/network/firewall/forwards/%s")
s.template_addremove = "firewall/cbi_addforward"
s.novaluetext = translate("There are no port forwarding rules created yet")

function s.create(self, section)
	local n = m:formvalue("_newfwd.name")
	local p = m:formvalue("_newfwd.proto")
	local e = m:formvalue("_newfwd.extport")
	local a = m:formvalue("_newfwd.intaddr")
	local i = m:formvalue("_newfwd.intport")
	error=0
	e_port = e:split("-")
	if not e_port[2] then
		e_port[2]=e_port[1]
	end

	if e == "" then
		m.message = translate("err: Please enter external port");
		error=1
	else
		e_port[1]=tonumber(e_port[1])
		e_port[2]=tonumber(e_port[2])
		uci:foreach("firewall", "redirect", function(l)
			if l.src_dport then
				d_port = l.src_dport:split("-")
				if not d_port[2] then
					d_port[2]=d_port[1]
				end
				d_port[1]=tonumber(d_port[1])
				d_port[2]=tonumber(d_port[2])
				if (e_port[1] <= d_port[1] and e_port[2] >= d_port[1]) or (e_port[1] >= d_port[1] and e_port[1] <= d_port[2]) then
					if l.enabled == "1" then
						m.message = translate("wrn: Warning: Entered port/port range is already in use");
-- 						error=1
					end
				end
			end
		end	)
	end

	if error==0 then
		if p == "other" or (p and a) then
			created = TypedSection.create(self, section)

			self.map:set(created, "target",    "DNAT")
			self.map:set(created, "src",       "wan")
			self.map:set(created, "dest",      "lan")
			self.map:set(created, "proto",     (p ~= "other") and p or "all")
			self.map:set(created, "src_dport", e)
			self.map:set(created, "dest_ip",   a)
			self.map:set(created, "dest_port", i)
			self.map:set(created, "name",      n)
		end
	end
end

function s.parse(self, ...)
	TypedSection.parse(self, ...)
	if created then
		m.uci:save("firewall")
		m.uci:commit("firewall")
-- 		luci.http.redirect(ds.build_url(
-- 			"admin/network/firewall/forwards", created
-- 		))
	end
end
-- Atvaizduoja pagal firewall esanti target
function s.filter(self, sid)
	return (self.map:get(sid, "target") == "DNAT")
end
--[[
function s.filter(self, sid)
	return (self.map:get(sid, "target") ~= "DNAT")
end

function s.filter(self, sid)
	return (self.map:get(sid, "name") ~= "DMZ")
end
]]--
ft.opt_name(s, DummyValue, translate("Name"), translate("Name of the rule. Used for easier rules management purpose only"))


proto = s:option(DummyValue, "proto", translate("Protocol"), translate("Protocol type of incoming or outgoing packet"))
proto.rawhtml = true
proto.width   = "10%"
function proto.cfgvalue(self, s)
	return ft.fmt_proto(self.map:get(s, "proto")) or "Any"
end


src = s:option(DummyValue, "src", translate("Source"), translate("Match incoming traffic from this IP or range only"))
src.rawhtml = true
src.width   = "12%"
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

via = s:option(DummyValue, "via", translate("Via"), translate("Match incoming traffic directed at the given IP address and port only"))
via.rawhtml = true
via.width   = "18%"
function via.cfgvalue(self, s)
	local a = ft.fmt_ip(self.map:get(s, "src_dip"), translate("any router IP"))
	local p = ft.fmt_port(self.map:get(s, "src_dport"))

	if p then
		return translatef("To %s at %s", a, p)
	else
		return translatef("To %s", a)
	end
end

dest = s:option(DummyValue, "dest", translate("Destination"), translate("Redirect matched traffic to the the given IP address and destination port"))
dest.rawhtml = true
dest.width   = "20%"
function dest.cfgvalue(self, s)
	local z = ft.fmt_zone(self.map:get(s, "dest"), translate("any zone"))
	local a = ft.fmt_ip(self.map:get(s, "dest_ip"), translate("any host"))
	local p = ft.fmt_port(self.map:get(s, "dest_port")) or
		ft.fmt_port(self.map:get(s, "src_dport"))

	if p then
		return translatef("Forward to %s, %s in %s", a, p, z)
	else
		return translatef("Forward to %s in %s", a, z)
	end
end

ft.opt_enabled(s, Flag, translate("Enable"), translate("Make a rule active/inactive")).width = "1%"

return m
