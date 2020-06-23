--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: ntpc.lua 6065 2010-04-14 11:36:13Z ben $
]]--
require("luci.sys")
require("luci.sys.zoneinfo")
require("luci.tools.webadmin")
require("luci.fs")
require("luci.config")

local utl = require "luci.util"
local sys = require "luci.sys"

local has_gps = utl.trim(luci.sys.exec("uci get hwinfo.hwinfo.gps"))
local port

local function cecho(string)
	luci.sys.call("echo \"" .. string .. "\" >> /tmp/log.log")
end

m = Map("ntpclient", translate("Time Synchronization"), translate(""))

--------- General

s = m:section(TypedSection, "ntpclient", translate("General"))
s.anonymous = true
s.addremove = false


--s:option(DummyValue, "_time", translate("Current system time")).value = os.date("%c")

o = s:option(DummyValue, "_time", translate("Current system time"), translate("Device\\'s current system time. Format [year-month-day, hours:minutes:seconds]"))
o.template = "admin_system/clock_status"

local tzone = s:option(ListValue, "zoneName", translate("Time zone"), translate("Time zone of your country"))
tzone:value(translate("UTC"))
for i, zone in ipairs(luci.sys.zoneinfo.TZ) do
	local time_value = zone[1]
	if time_value:find("Etc") then
		time_value = time_value:gsub("Etc/", "")
		if time_value:find("+") then
			time_value = time_value:gsub("+", "-")
		else
			time_value = time_value:gsub("-", "+")
		end
	end
	tzone:value(zone[1], time_value)
end

function tzone.write(self, section, value)
	local cfgName
	local cfgTimezone

	Value.write(self, section, value)

	local function lookup_zone(title)
		for _, zone in ipairs(luci.sys.zoneinfo.TZ) do
			if zone[1] == title then return zone[2] end
		end
	end

	m.uci:foreach("system", "system", function(s)
		cfgName = s[".name"]
		cfgTimezone = s.timezone
	end)

	local timezone = lookup_zone(value) or "GMT0"
	m.uci:set("system", cfgName, "timezone", timezone)
	m.uci:save("system")
	m.uci:commit("system")
	luci.fs.writefile("/etc/TZ", timezone .. "\n")
end

s:option(Flag, "enabled", translate("Enable NTP"), translate("Enable system\\'s time synchronization with time server using NTP (Network Time Protocol)"))

s:option(Flag, "force", translate("Force servers"), translate("Force unreliable NTP servers"))

el1 = s:option(Value, "interval", translate("Update interval (in seconds)"), translate("How often the router should update system\\'s time"))
el1.rmempty = true
el1.datatype = "integer"

el = s:option(Value, "save", translate("Save time to flash"), translate("Save last synchronized time to flash memory"))
el.template = "cbi/flag"

function el1.validate(self, value, section)
	aaa=luci.http.formvalue("cbid.ntpclient.cfg0c8036.save")
	if tonumber(aaa) == 1 then
		if tonumber(value) >= 3600 then
			return value
		else
			return nil, "The value is invalid because min value 3600"
		end
	else
		if tonumber(value) >= 10 then
			return value
		else
			return nil, "The value is invalid because  min value 10"
		end
	end
end

a = s:option(Value, "count", translate("Count of time synchronizations"), translate("How many time synchronizations NTP (Network Time Protocol) client should perform. Empty value - infinite"))
a.datatype = "fieldvalidation('^[0-9]+$',0)"
a.rmempty = true

------ GPS synchronisation
if has_gps == "1" then
	gps = s:option(Flag, "gps_sync", translate("GPS synchronization"), translate("Enable periodic time synchronization of the system, using GPS module (does not require internet connection)"))
	gps_int = s:option(ListValue, "gps_interval", translate("GPS time update interval"), translate("Update period for updating system time from GPS module"))
	gps_int:value("300", translate("Every 5 minutes"))
	gps_int:value("1800", translate("Every 30 minutes"))
	gps_int:value("3600", translate("Every hour"))
	gps_int:value("21600", translate("Every 6 hours"))
	gps_int:value("43200", translate("Every 12 hours"))
	gps_int:value("86400", translate("Every 24 hours"))
	gps_int:value("604800", translate("Every week"))
	gps_int:value("2592000", translate("Every month"))
	gps_int:depends("gps_sync", "1")
	gps_int.default = "86400"
	gps_int.rmempty = true
	gps_int.datatype = "integer"
end

------- Clock Adjustment
s2 = m:section(TypedSection, "ntpdrift", translate("Clock Adjustment"))
s2.anonymous = true
s2.addremove = false
b = s2:option(Value, "freq", translate("Offset frequency"), translate("Adjust the drift of the local clock to make it run more accurately"))
b.datatype = "fieldvalidation('^[0-9]+$',0)"
b.rmempty = true

m2 = Map("ntpserver", translate("NTP Server"), translate(""))

s3 = m2:section(NamedSection, "general", "ntpserver", translate("General"))

srv = s3:option(Flag, "enabled", translate("Enable"), translate("Enable NTP server"))

function m.on_after_commit(self)
	luci.sys.exec("export ACTION=ifdown; sh /etc/hotplug.d/iface/20-ntpclient")
	luci.sys.exec("export ACTION=; sh /etc/hotplug.d/iface/20-ntpclient")
	
	if has_gps == "1" then
		local gps_service_enabled = m.uci:get("gps", "gpsd", "enabled") or ""
		local gps_sync_enabled = m.uci:get(m.config, "@ntpclient[0]", "gps_sync") or ""

		if gps_sync_enabled == "1" and gps_service_enabled ~= "1" then
			m.uci:set("gps", "gpsd", "enabled", "1")
			m.uci:commit("gps")
			luci.sys.exec("/sbin/luci-reload")
		end
	end
end

return m, m2
