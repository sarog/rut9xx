local sys = require "luci.sys"
local dsp = require "luci.dispatcher"
local utl = require "luci.util"
local fs = require "nixio.fs"

local m, s, o

arg[1] = arg[1] or ""

m = Map("gps", translate("AVL Input Rule Data Configuration"))

m.redirect = dsp.build_url("admin/services/gps/input")
if m.uci:get("gps", arg[1]) ~= "input" then
        luci.http.redirect(dsp.build_url("admin/services/gps/input"))
        return
end

s = m:section(NamedSection, arg[1], "input", "")
s.anonymous = true
s.addremove = false

--ft.opt_enabled(s, Button)
o = s:option(Flag, "enabled", translate("Enable"), translate("To enable input configuration"))
o.rmempty = false

o = s:option(ListValue, "input", translate("Input type"), translate("Select type on your own intended configuration"))
o:value("digital1", translate("Digital"))
o:value("digital2", translate("Digital isolated"))

--[[
-- Only for facelift
-- ]]
if fs.access("/sys/bus/i2c/devices/0-0074/gpio") == nil then
	o:value("digital3", translate("4PIN Digital"))
end

o:value("analog", translate("Analog"))

-- function o.write(self, section, value)
--      if value == "analog" then
--              luci.sys.call('uci set gps.'..section..'.input="false"')
--      end
--              m.uci:set("gps", section, "type", value)
--              m.uci:save("gps")
--              m.uci:commit("gps")
-- end

minval = s:option(Value, "min", translate("Min [V]"), translate("Specify minimum voltage range"))
minval:depends("input", "analog")
function minval:validate(Values)
        Values = string.gsub(Values,",",".")
        if tonumber(Values) and tonumber(Values)>= 0 and tonumber(Values)<= 24 then
                return Values
        else
                return nil
        end
end
maxval = s:option(Value, "max", translate("Max [V]"), translate("Specify maximum voltage range"))
maxval:depends("input", "analog")
function maxval:validate(Values)
        Values = string.gsub(Values,",",".")
        if tonumber(Values) and tonumber(Values)>= 0 and tonumber(Values)<= 24 then
                return Values
        else
                return nil
        end
end

o = s:option(ListValue, "event", translate("Trigger"), translate("Select trigger event for your own intended configuration"))
o:value("no", translate("Input open"))
o:value("nc", translate("Input shorted"))
o:value("both", translate("Both"))
o:depends("input", "digital1")
o:depends("input", "digital3")
--o:depends("input", "digital2")

o = s:option(ListValue, "event2", translate("Trigger"), translate("Select trigger event for your own intended configuration"))
o:value("no", translate("Low logic level"))
o:value("nc", translate("High logic level"))
o:value("both", translate("Both"))
o:depends("input", "digital2")

function o.cfgvalue(self, section)
        local v = m.uci:get("gps", section, "event")
        return v
end

function o.write(self, section, value)
        local typ = luci.http.formvalue("cbid.gps."..arg[1]..".input")
        if typ == "digital2" then
                m.uci:set("gps", section, "event", value)
                m.uci:save("gps")
                m.uci:commit("gps")
        end
end

o = s:option(ListValue, "event3", translate("Trigger"), translate("Inside range - Input voltage falls in the specified region, Outside range - Input voltage drops out of the specified region"))
o:value("in", translate("Inside range"))
o:value("out", translate("Outside range"))
o:depends("input", "analog")

function o.cfgvalue(self, section)
        local v = m.uci:get("gps", section, "event")
        return v
end

function o.write(self, section, value)
        local typ = luci.http.formvalue("cbid.gps."..arg[1]..".input")
        if typ == "analog" then
        m.uci:set("gps", section, "event", value)
        m.uci:save("gps")
        m.uci:commit("gps")
        end
        if typ == "digital2" then
                m.uci:set("gps", section, "event", value)
                m.uci:save("gps")
                m.uci:commit("gps")
        end
end

o = s:option(ListValue, "priority", translate("Priority"), translate("Select priority"))
o:value("low", translate("Low"))
o:value("high", translate("High"))
o:value("panic", translate("Panic"))

local gps_enable = m.uci:get("gps",  arg[1], ".enabled") or "0"
function m.on_commit()
        --Delete all usr_enable from gps config
        local gpsEnable = m:formvalue("cbid.gps." .. arg[1] .. ".enabled") or "0"
        if gpsEnable ~= gps_enable then
                m.uci:foreach("gps", "input", function(s)
                        local usr_enable = s.usr_enable or ""
                        gps_inst2 = s[".name"] or ""
                        if usr_enable == "1" then
                                m.uci:delete("gps", gps_inst2 , "usr_enable")
                        end
                end)
        end
        m.uci:save("gps")
        m.uci.commit("gps")
end

return m
