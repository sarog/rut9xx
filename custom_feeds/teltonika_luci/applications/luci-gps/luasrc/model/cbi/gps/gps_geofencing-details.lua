local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()
local fw = require "luci.model.firewall"

local section_name

if arg[1] then
	section_name = arg[1]
else
	luci.http.redirect(luci.dispatcher.build_url("admin", "services", "gps", "geofencing"))
end

local m = Map("gps", translate("Geofencing details"), translate(""))

m.redirect = luci.dispatcher.build_url("admin", "services", "gps", "geofencing")
local s = m:section(NamedSection, arg[1], "geofencing", "")

o = s:option(Flag, "enabled", translate("Enable"), translate(""))

o = s:option(Value, "longitude", translate("Longitude (X)"), translate("Floating part of number must contain 6 digits (25.000000)"))
o.datatype = "geofencing(-180.000000,180.000000)"
o.default = "0.000000"

o = s:option(Value, "latitude", translate("Latitude (Y)"), translate("Floating part of number must contain 6 digits (25.000000)"))
o.datatype = "geofencing(-90.000000,90.000000)"
o.default = "0.000000"

o = s:option(Value, "radius", translate("Radius"), translate(""))
o.default = "200"
o.datatype = "range(1,999999)"

o = s:option(ListValue, "generate_event", translate("Generate event on"), translate(""))
o:value("on_exit", translate("Exit"))
o:value("on_enter", translate("Enter"))
o:value("on_both", translate("Enter/exit"))

o = s:option(DummyValue, "", "", "")
o.template  = "gps/imagesection"

return m
