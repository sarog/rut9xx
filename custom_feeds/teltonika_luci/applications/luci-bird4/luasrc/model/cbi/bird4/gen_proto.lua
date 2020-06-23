--[[
Copyright (C) 2014 - Eloi Carbó Solé (GSoC2014)
BGP/Bird integration with OpenWRT and QMP

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
--]]

require("luci.sys")
local http = require "luci.http"
local uci = require "luci.model.uci".cursor()
local ds = require "luci.dispatcher"

m=Map("bird4", translate("General Protocols Configuration"))

-- Optional parameters lists
local protoptions = {
	{["name"]="table", ["help"]="Auxiliar table for routing", ["depends"]={"static","kernel"}},
	{["name"]="import", ["help"]="Set if the protocol must import routes", ["depends"]={"kernel"}},
	{["name"]="export", ["help"]="Set if the protocol must export routes", ["depends"]={"kernel"}},
	{["name"]="scan_time", ["help"]="Time between scans", ["depends"]={"kernel","device"}},
	{["name"]="kernel_table", ["help"]="Set which table must be used as auxiliar kernel table", ["depends"]={"kernel"}},
	{["name"]="learn", ["help"]="Learn routes", ["depends"]={"kernel"}},
	{["name"]="persist", ["help"]="Store routes. After a restart, routes will be still configured", ["depends"]={"kernel"}}
}

local routeroptions = {
	{["name"]="prefix",["help"]="",["depends"]={"router","special","iface","multipath","recursive"}},
	{["name"]="via",["help"]="",["depends"]={"router","multipath"}},
	{["name"]="attribute",["help"]="",["depends"]={"special"}},
	{["name"]="iface",["help"]="",["depends"]={"iface"}},
	{["name"]="ip",["help"]="",["depends"]={"recursive"}}
}

--
-- KERNEL PROTOCOL
--

sect_kernel_protos = m:section(TypedSection, "kernel", translate("Kernel Options"), translate(""))
-- sect_kernel_protos.addremove = true
sect_kernel_protos.anonymous = false

-- Default kernel parameters

disabled = sect_kernel_protos:option(Flag, "enabled", translate("Enable"), translate(""))
disabled.default= "0"

learn = sect_kernel_protos:option(Flag, "learn", translate("Learn"), translate("Learn routes"))
-- learn.default= "1"

persist = sect_kernel_protos:option(Flag, "persist", translate("Persist"), translate("Store routes. After a restart, routes will be still configured"))
-- persist.default= "1"

scan_time = sect_kernel_protos:option(Value, "scan_time", translate("Scan time"), translate("Time between scans"))
scan_time.default= "10"

import = sect_kernel_protos:option(Value, "import", translate("Import"), translate("Set if the protocol must import routes"))
import.optional = true
import:value("all", translate("All"))
import:value("none", translate("None"))
import.default= "all"

export = sect_kernel_protos:option(Value, "export", translate("Export"), translate("Set if the protocol must export routes"))
export.optional = true
export:value("all", translate("All"))
export:value("none", translate("None"))
export.default= "all"
-- -- Optional parameters
-- for _,o in ipairs(protoptions) do
-- 	if o.name ~= nil then
-- 		for _, d in ipairs(o.depends) do
-- 			if d == "kernel" then
-- 				if o.name == "learn" or o.name == "persist" then
-- 					value = sect_kernel_protos:option(Flag, o.name, translate(o.name), translate(o.help))
-- 				elseif o.name == "table" then
-- 					value = sect_kernel_protos:option(ListValue, o.name, translate(o.name), translate(o.help))
-- 					uciout:foreach("bird4", "table",
-- 						function (s)
-- 							value:value(s.name)
-- 						end)
-- 					value:value("")
-- 				else
-- 					value = sect_kernel_protos:option(Value, o.name, translate(o.name), translate(o.help))
-- 				end
-- 				value.optional = true
-- 				value.rmempty = true
-- 			end
-- 		end
--
-- 	end
-- end

--
-- DEVICE PROTOCOL
--

sect_device_protos = m:section(TypedSection, "device", translate("Device Options"), translate(""))
-- sect_device_protos.addremove = true
-- sect_device_protos.anonymous = false

-- Default kernel parameters

disabled = sect_device_protos:option(Flag, "enabled", translate("Enable"), translate("If this option is true, the protocol will not be configured."))
disabled.default=1

time = sect_device_protos:option(Value, "scan_time", translate("Scan time"), translate("Time between scans"))
time.optional = true
time.rmempty = true

-- Optional parameters
-- for _,o in ipairs(protoptions) do
-- 	if o.name ~= nil then
-- 		for _, d in ipairs(o.depends) do
-- 			if d == "device" then
-- 				value = sect_device_protos:option(Value, o.name, translate(o.name), translate(o.help))
-- 				value.optional = true
-- 				value.rmempty = true
-- 			end
-- 		end
-- 	end
-- end


--[[
--
-- STATIC PROTOCOL
--

sect_static_protos = m:section(TypedSection, "static", translate("Static options"), translate("Configuration of the static protocols."))
sect_static_protos.addremove = true
sect_static_protos.anonymous = false

-- Default kernel parameters

disabled = sect_static_protos:option(Flag, "disabled", translate("Disabled"), translate("If this option is true, the protocol will not be configured."))
disabled.default=0

-- Optional parameters
for _,o in ipairs(protoptions) do
	if o.name ~= nil then
		for _, d in ipairs(o.depends) do
			if d == "static" then
				if o.name == "table" then
					value = sect_static_protos:option(ListValue, o.name, translate(o.name), translate(o.help))
					uciout:foreach("bird4", "table",
						function (s)
							value:value(s.name)
						end)
					value:value("")
				else
					value = sect_static_protos:option(Value, o.name, translate(o.name), translate(o.help))
				end
					value.optional = true
					value.rmempty = true
			end
		end
	end
end]]

--
-- ROUTES FOR STATIC PROTOCOL
--


s = m:section(TypedSection, "route", translate("Static Routes"), translate(""))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.sorthint  = translate("All rules are executed in current list order")
s.extedit   = ds.build_url("admin/network/routes/dynamic_routes/routes_details/%s")
s.template_addremove = "bird4/add_routes"
s.novaluetext = translate("There are no static routes created yet")

function s.create(self, section)
	local p = m:formvalue("_newroute.prefix")
	local t = m:formvalue("_newroute.type")

	created = TypedSection.create(self, section)
	self.map:set(created, "prefix", p)
	self.map:set(created, "type", t)

	m.uci:save("bird4")
	m.uci:commit("bird4")
end

-- function s.parse(self, ...)
-- 	TypedSection.parse(self, ...)
-- 	if created then
-- 		luci.http.redirect(ds.build_url("admin/network/bird4/route-details", created))
-- 	end
-- end

-- Atvaizduoja pagal routes esancius typus
function s.filter(self, sid)
	return (self.map:get(sid, "type") == "router" or "iface" or "special" or "recursive" or "multipath")
end

prefix = s:option(DummyValue, "prefix", translate("Prefix"), translate("Protocol type of incoming or outgoing packet"))
prefix.rawhtml = true
prefix.width   = "30%"
function prefix.cfgvalue(self, s)
	return self.map:get(s, "prefix")
end


proto = s:option(DummyValue, "type", translate("Type"), translate("Protocol type of incoming or outgoing packet"))
proto.rawhtml = true
proto.width   = "30%"
function proto.cfgvalue(self, s)
	return self.map:get(s, "type")
end

function m.on_commit(self,map)
        luci.sys.call('/etc/init.d/bird4 stop; /etc/init.d/bird4 start')
	check = luci.http.formvalue("cbid.bird4.kernel.enabled") or "0"
	if check then
		if check == "1" then
			m.uci:set("bird4", "kernel", "disabled", "0")
		else
			m.uci:set("bird4", "kernel", "disabled", "1")
		end
	end

	check = luci.http.formvalue("cbid.bird4.device.enabled") or "0"
	if check then
		if check == "1" then
			m.uci:set("bird4", "device", "disabled", "0")
		else
			m.uci:set("bird4", "device", "disabled", "1")
		end
	end
			m.uci:save("bird4")
			m.uci:commit("bird4")
end

return m
