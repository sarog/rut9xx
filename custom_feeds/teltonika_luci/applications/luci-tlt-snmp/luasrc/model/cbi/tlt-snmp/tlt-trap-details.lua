local sys = require "luci.sys"
local dsp = require "luci.dispatcher"
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()
local in_out = uci:get("hwinfo","hwinfo","in_out") or "0"
local m, s, o
local kam = 0

arg[1] = arg[1] or ""

m = Map("snmpd", translate("Trap Configuration"))

m.redirect = dsp.build_url("admin/services/snmp/trap-settings/")
if m.uci:get("snmpd", arg[1]) ~= "rule" then
	luci.http.redirect(dsp.build_url("admin/services/snmp/trap-settings"))
  	return
end

s = m:section(NamedSection, arg[1], "rule", translate("Modify Trap Rule"))
s.anonymous = true
s.addremove = false

o = s:option(Flag, "enabled", translate("Enable"), translate("Enable this rule"))
o.rmempty = false
o.default = "0"

function check_exist(name)
	existname = 0
	are_section = 0
	uci:foreach("snmpd", "rule", function(x)
		actionName = x.action or ""
		if actionName == name then
			existname = 1
		end
	end)
	return existname
end
kam = luci.util.trim(luci.sys.exec("uci get snmpd.\"".. arg[1] .."\".action"))
-- luci.sys.call("echo \"arg|".. kam .."|\" >>/tmp/aaa")
o = s:option(ListValue, "action", translate("Action"), translate("The action to be performed when this rule is met"))

bam = check_exist("sigEnb")
if bam ~= 1 or kam == "sigEnb" then
	o:value("sigEnb", translate("Signal strength trap"))
end

bam = check_exist("conEnb")
if bam ~= 1 or kam == "conEnb" then
	o:value("conEnb", translate("Connection type trap"))
end

local has_4pins = uci:get("hwinfo", "hwinfo", "4pin_io")

if in_out == "1" then
	bam = check_exist("digIn")
	if bam ~= 1 or kam == "digIn" then
		o:value("digIn", translate("Digital input trap"))
	end

	bam = check_exist("digOCIn")
	if bam ~= 1  or kam == "digOCIn" then
		o:value("digOCIn", translate("Digital isolated input trap"))
	end
end

if has_4pins == "1" then 
	bam = check_exist("dig4PinIn")
	if bam ~= 1 or kam == "dig4PinIn" then
		o:value("dig4PinIn", translate("Digital 4PIN input trap"))
	end
end

if in_out == "1" then
	bam = check_exist("analog")
	if bam ~= 1  or kam == "analog" then
		o:value("analog", translate("Analog input trap"))
	end

	bam = check_exist("digOut")
	if bam ~= 1  or kam == "digOut" then
		o:value("digOut", translate("Digital output trap"))
	end

	bam = check_exist("digRelayOut")
	if bam ~= 1 or kam == "digRelayOut" then
		o:value("digRelayOut", translate("Digital relay output trap"))
	end
end

if has_4pins == "1" then 
	bam = check_exist("dig4PinOut")
	if bam ~= 1 or kam == "dig4PinOut" then
		o:value("dig4PinOut", translate("Digital 4PIN output trap"))
	end
end

function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	return v
end

sigstr = s:option(Value, "signal", translate("Signal strength"), translate("GSM signal\\'s strength value in dBm, e.g. -85"))
sigstr:depends("action", "sigEnb")
sigstr.datatype = "range(-130,0)"

o = s:option(ListValue, "state", translate("State change"), translate("Chose state to trigger TRAP"))
o:value("active", translate("Active"))
o:value("inactive", translate("Inactive"))
o:value("both", translate("Both"))
o:depends("action", "digIn")
o:depends("action", "digOCIn")
o:depends("action", "digOut")
o:depends("action", "digRelayOut")
o:depends("action", "dig4PinIn")
o:depends("action", "dig4PinOut")

o = s:option(ListValue, "analog_state", translate("State change"), translate("Chose state to trigger TRAP"))
o:value("higher", translate("Higher"))
o:value("lower", translate("Lower"))
o:value("both", translate("Both"))
o:depends("action", "analog")

an = s:option(Value, "volts", translate("Voltage"), translate("Voltage value in V, e.g. 12.21"))
an:depends("action", "analog")
an.datatype = "range(0,30)"


return m
