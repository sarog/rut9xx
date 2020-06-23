--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: forwards.lua 8117 2011-12-20 03:14:54Z jow $
]]--

--s = m:section(TypedSection, "input", translate("Input Rules"))
--s.template  = "cbi/tblsection"
--s.addremove = true
--s.anonymous = true
--s.sortable  = true
--s.extedit   = ds.build_url("admin/services/gps/input/%s")
--s.template_addremove = "gps/cbi_add_gps_input"
--s.novaluetext = translate("There are no gps rules created yet")

local ds = require "luci.dispatcher"
local ft = require "luci.tools.gps"

m = Map("gps", translate("GPS Geofencing"),	translate(""))

s = m:section(TypedSection, "geofencing", translate("Geofencing"))
	s.addremove = true
	s.template = "gps/tblsection_geofence"
	s.novaluetext = translate("There are no geofencing configurations yet")
	s.extedit = ds.build_url("admin/services/gps/geofencing/%s")
	s.sectionhead = "Name"

function s.create(self, section)
	created = TypedSection.create(self, section)
	self.map:set(section, "longitude", "0.000000")
	self.map:set(section, "latitude", "0.000000")
	self.map:set(section, "radius", "200")
	self.map:set(section, "generate_event", "on_exit")
end

o = s:option(DummyValue, "enabled", translate("Status"), translate(""))
function o.cfgvalue(self, s)
	local z = self.map:get(s, "enabled") or "0"
	if z == "0" then
		return translatef("Disabled")
	elseif z == "1" then
		return translatef("Enabled")
	else
		return translatef("N/A")
	end
end

o = s:option(DummyValue, "longitude", translate("Longitude (X)"), translate(""))

o = s:option(DummyValue, "latitude", translate("Latitude (Y)"), translate(""))

o = s:option(DummyValue, "radius", translate("Radius"), translate(""))

o = s:option(DummyValue, "generate_event", translate("Generate event on"), translate(""))
function o.cfgvalue(self, s)
	local z = self.map:get(s, "generate_event")
	if z == "on_exit" then
		return translatef("Exit")
	elseif z == "on_enter" then
		return translatef("Enter")
	elseif z == "on_both" then
		return translatef("Enter/Exit")
	else
		return translatef("N/A")
	end
end

return m
