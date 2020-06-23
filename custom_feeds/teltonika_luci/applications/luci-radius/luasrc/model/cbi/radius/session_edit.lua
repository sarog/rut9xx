--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: coovachilli.lua 3442 2008-09-25 10:12:21Z jow $
]]--
local ds = require "luci.dispatcher"
local function debug(string)
	luci.sys.call("logger \"" .. string .. "\"")
end

debug(arg[1])

m = Map( "radius",	translate( "Radius Server Configuration" ), translate( "" ))
	m.redirect = ds.build_url("admin/services/hotspot/radius/")

ses = m:section(NamedSection, arg[1], "session", translate( "Session Configuration Settings"))

idle = ses:option(Value, "defidletimeout", translate("Idle timeout" ), translate("Max idle time in sec. (0, meaning unlimited)"))
	idle.datatype = "integer"

timeout = ses:option(Value, "defsessiontimeout", translate("Session timeout" ), translate("Max session time in sec. (0, meaning unlimited)"))
	timeout.datatype = "integer"

download_band = ses:option(Value, "downloadbandwidth", translate("Download bandwidth"), translate("The max allowed download speed, in megabits." ))
	download_band.datatype = "integer"
	download_band.template = "chilli/value"
	download_band.unit_field_option = "d_bandwidth_unit"

	function download_band.write(self, section, value)
		local unit_value = m:formvalue(string.format("cbid.%s.%s.%s", self.config, section, self.unit_field_option)) or "kb"
		local multiplier = unit_value == "kb" and 1000 or 1000000
		local value = tonumber(value) * multiplier

		m.uci:set(self.config, section, self.option, value)
		m.uci:set(self.config, section, self.unit_field_option, unit_value)
	end

	function download_band.cfgvalue(self, section)
		local unit_value = m.uci:get(self.config, section, self.unit_field_option) or "kb"
		local multiplier = unit_value == "kb" and 1000 or 1000000
		local value = m.uci:get(self.config, section, self.option)

		if value then
			value = tonumber(value) / multiplier
		else
			value = nil
		end
		return value
	end

	function download_band.cfgunits(self, section)
		local value = m.uci:get(self.config, section, self.unit_field_option) or ""
		return value
	end

upload_band = ses:option(Value, "uploadbandwidth", translate("Upload bandwidth"), translate("The max allowed upload speed, in megabits." ))
	upload_band.datatype = "integer"
	upload_band.template = "chilli/value"
	upload_band.unit_field_option = "u_bandwidth_unit"

	function upload_band.write(self, section, value)
		local unit_value = m:formvalue(string.format("cbid.%s.%s.%s", self.config, section, self.unit_field_option)) or "kb"
		local multiplier = unit_value == "kb" and 1000 or 1000000
		local value = tonumber(value) * multiplier

		m.uci:set(self.config, section, self.option, value)
		m.uci:set(self.config, section, self.unit_field_option, unit_value)
	end

	function upload_band.cfgvalue(self, section)
		local unit_value = m.uci:get(self.config, section, self.unit_field_option) or "kb"
		local multiplier = unit_value == "kb" and 1000 or 1000000
		local value = m.uci:get(self.config, section, self.option)

		if value then
			value = tonumber(value) / multiplier
		else
			value = nil
		end
		return value
	end

	function upload_band.cfgunits(self, section)
		local value = m.uci:get(self.config, section, self.unit_field_option) or ""
		return value
	end

downloadlimit = ses:option(Value, "downloadlimit", translate("Download limit"), translate("Disable hotspot user after download limit value in MB is reached"))
	downloadlimit.datatype = "integer"

	function downloadlimit.write(self, section, value)
		value = tonumber(value) * 1048576
		m.uci:set(self.config, section, self.option, value)
	end

	function downloadlimit.cfgvalue(self, section)
		local value = m.uci:get(self.config, section, self.option)
		if value then
			value = tonumber(value) / 1048576
		else
			value = nil
		end
		return value
	end

uploadlimit = ses:option(Value, "uploadlimit", translate("Upload limit"), translate("Disable hotspot user after upload limit value in MB is reached"))
	uploadlimit.datatype = "integer"

	function uploadlimit.write(self, section, value)
		value = tonumber(value) * 1048576
		m.uci:set(self.config, section, self.option, value)
	end

	function uploadlimit.cfgvalue(self, section)
		local value = m.uci:get(self.config, section, self.option)
		if value then
			value = tonumber(value) / 1048576
		else
			value = nil
		end
		return value
	end


period = ses:option(ListValue, "period", translate("Period"), translate("Period for which hotspot data limiting should apply"))
	period:value("3", translate("Month"))
	period:value("2", translate("Week"))
	period:value("1", translate("Day"))

day = ses:option(ListValue, "day", translate("Start day"), translate("A starting time for hotspot data limiting period"))
	day:depends("period", "3")

	for i=1,28 do
		day:value(i, i)
	end

hour = ses:option(ListValue, "hour", translate("Start hour"), translate("A starting time for hotspot data limiting period"))
	hour:depends("period", "1")

	for i=1,23 do
		hour:value(i, i)
	end
	hour:value("0", "24")


weekday = ses:option(ListValue, "weekday", translate("Start day"), translate("A starting time for hotspot data limiting period"))
	weekday:value("1", translate("Monday"))
	weekday:value("2", translate("Tuesday"))
	weekday:value("3", translate("Wednesday"))
	weekday:value("4", translate("Thursday"))
	weekday:value("5", translate("Friday"))
	weekday:value("6", translate("Saturday"))
	weekday:value("0", translate("Sunday"))
	weekday:depends("period", "2")

return m
