--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

$Id: forwards.lua 8117 2011-12-20 03:14:54Z jow $
]]--

local ds = require "luci.dispatcher"
local ft = require "luci.tools.gps"
local fs = require "nixio.fs"

local hw_rev = fs.access("/sys/bus/i2c/devices/0-0074/gpio")

m = Map("gps", translate("AVL I/O Configuration"),      translate(""))

s = m:section(TypedSection, "avl_io", translate("Check Analog"))

interval = s:option(Value, "analog_interval", translate("Interval [sec]"), translate("Interval (in seconds) for checking analog input value"))
interval.default = "5"
interval.datatype = "range(1,999999)"
--
-- GPS MODE
--
s = m:section(TypedSection, "input", translate("Input Rules"))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.sortable  = true
s.extedit   = ds.build_url("admin/services/gps/input/%s")
s.template_addremove = "gps/cbi_add_gps_input"
s.novaluetext = translate("There are no gps rules created yet")

function s.create(self, section)
	local t = m:formvalue("_newinput.type")
	local tr = m:formvalue("_newinput.event")
	local tr2 = m:formvalue("_newinput.event2")
	local tr3 = m:formvalue("_newinput.event3")

	created = TypedSection.create(self, section)
	self.map:set(created, "input",   t)
	if tr then
		self.map:set(created, "event", tr)
	end
	if tr2 then
		self.map:set(created, "event", tr2)
	end
	if tr3 then
		self.map:set(created, "event", tr3)
	end
	self.map:set(created, "priority", "0")
	if created then
		m.uci:save("gps")
		luci.http.redirect(ds.build_url("admin/services/gps/input", created))
	end
end

src = s:option(DummyValue, "input", translate("Input"), translate("AVL rule input type"))
src.rawhtml = true
src.width   = "10%"
function src.cfgvalue(self, s)
	local z = self.map:get(s, "input")
	--os.execute("echo \"l"..z.."l\" >>/tmp/aaa")
	--return z
	if z == "digital1" then
		return translate("Digital input")
	elseif z == "digital2" then
		return translate("Digital isolated input")
	elseif hw_rev == nil and z == "digital3" then
		return translate("4PIN digital input")
	elseif z == "analog" then
		return translate("Analog")
	else
		return translate("N/A")
	end
end

src = s:option(DummyValue, "priority", translate("Priority"), translate("Rule priority"))
src.rawhtml = true
src.width   = "10%"
function src.cfgvalue(self, s)
	local z = self.map:get(s, "priority")
	if z == "low" then
		return translatef("Low")
	elseif z == "high" then
		return translatef("High")
	elseif z == "panic" then
		return translatef("Panic")
	else
		return translatef("N/A")
	end
end

src = s:option(DummyValue, "event", translate("Generate event"), translate("Input event for rule activation"))
src.rawhtml = true
src.width   = "18%"
function src.cfgvalue(self, s)
	local t = self.map:get(s, "event")
	local z = self.map:get(s, "input")
	local min_v = self.map:get(s, "min")
	local max_v = self.map:get(s, "max")
	local min_max_line = ""
	if min_v == nil and max_v == nil then
		min_max_line = "( 0V - 0V )"
	end
	if min_v == nil then
		min_v = "0"
	end
	if max_v == nil then
		max_v = "24"
	end
	if min_max_line == "" then
		min_max_line = "( " .. min_v .. "V - " .. max_v .. "V )"
	end
	local max_v = self.map:get(s, "max")
	if t == "no" and (z == "digital1" or z == "digital3") then
		return translatef("Input open")
	elseif t == "nc" and (z == "digital1" or z == "digital3") then
		return translatef("Input shorted")
	elseif t == "both" and (z == "digital1" or z == "digital3") then
		return translatef("Both")

	elseif t == "no" and z == "digital2" then
		return translatef("Low logic level")
	elseif t == "nc" and z == "digital2" then
		return translatef("High logic level")
	elseif t == "both" and z == "digital2" then
		return translatef("Both")

	elseif t == "in" and z == "analog" then
		return translatef("In" .. " " .. min_max_line)
	elseif t == "out" and z == "analog" then
		return translatef("Out" .. " " .. min_max_line)
	else
		return translatef("N/A")
	end
end

ft.opt_enabled(s, Flag, translate("Enable"), translate("Check to enable this rule")).width = "18%"

local save = m:formvalue("cbi.apply")
if save then
	--Delete all usr_enable from gps config
	m.uci:foreach("gps", "input", function(s)
	       gps_inst = s[".name"] or ""
	       gpsEnable = m:formvalue("cbid.gps." .. gps_inst .. ".enabled") or "0"
	       gps_enable = s.enabled or "0"
	       if gpsEnable ~= gps_enable then
		       m.uci:foreach("gps", "input", function(a)
		       gps_inst2 = a[".name"] or ""
		       local usr_enable = a.usr_enable or ""
		       if usr_enable == "1" then
			       m.uci:delete("gps", gps_inst2, "usr_enable")
		       end
	       end)
		end
	end)
m.uci:save("gps")
m.uci.commit("gps")
end

return m
