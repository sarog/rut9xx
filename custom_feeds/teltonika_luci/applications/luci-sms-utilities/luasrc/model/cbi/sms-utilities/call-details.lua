--[[
LuCI - Lua Configuration Interface

Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: forward-details.lua 8117 2011-12-20 03:14:54Z jow $
]]--

local sys = require "luci.sys"
local dsp = require "luci.dispatcher"
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()
local in_out = uci:get("hwinfo","hwinfo","in_out") or "0"
local is_io = uci:get("hwinfo","hwinfo","4pin_io") or "0"
local m, s, o

arg[1] = arg[1] or ""

m = Map("call_utils", translate("Call Configuration"))

m.redirect = dsp.build_url("admin/services/sms/call-utilities/")
if m.uci:get("call_utils", arg[1]) ~= "rule" then
	luci.http.redirect(dsp.build_url("admin/services/sms/call-utilities"))
	return
else
	--local name = m:get(arg[1], "name") or m:get(arg[1], "_name")
	--if not name or #name == 0 then
	--	name = translate("(Unnamed Entry)")
	--end
	--m.title = "%s - %s" %{ translate("Firewall - Port Forwards"), name }
end

s = m:section(NamedSection, arg[1], "rule", translate("Modify Call Rule"))
s.anonymous = true
s.addremove = false

o = s:option(Flag, "enabled", translate("Enable"), translate("Enable this rule"))

o = s:option(ListValue, "action", translate("Action"), translate("The action to be performed when this rule is met"))
o:value("reboot", translate("Reboot"))
o:value("send_status", translate("Send status"))
o:value("wifi", translate("Switch WiFi"))
o:value("mobile", translate("Switch mobile data"))
if in_out == "1" or is_io == "1" then
	o:value("dout", translate("Switch output"))
end

function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	return v
end

o = s:option(ListValue, "value", translate(" "), translate(""))
o:value("on", translate("On"))
o:value("off", translate("Off"))
o:depends("action", "wifi")
o:depends("action", "mobile")
o:depends("action", "dout")

o = s:option(Flag, "timeout", translate("Active timeout"), translate("Rule active for a specified time"))
o:depends("action", "dout")
o.default = false
o.rmempty = false

o = s:option(Value, "seconds", translate("Seconds"), translate("Rule active for a specified time, format seconds"))
o:depends("timeout", "1")
o.datatype = "range(1,999999)"
o.default = "5"

o = s:option(ListValue, "allowed_phone", translate("Allowed users"), translate("Whitelist of allowed users"))
o:value("all", translate("From all numbers"))
o:value("group", translate("From group"))
o:value("single", translate("From single number"))

src = s:option(Value, "tel", translate("Caller's phone number"), translate("A whitelisted phone number. Allowable characters: (0-9#*+)"))
src.datatype = "fieldvalidation('^[0-9#*+]+$',0)"
src:depends("allowed_phone", "single")

o = s:option(ListValue, "group", translate("Group"), translate("A whitelisted users group"))
m.uci:foreach("sms_utils", "group", function(s)
	o:value(s.name, s.name)
end)
o:depends("allowed_phone", "group")

stat_e = s:option(Flag, "status_sms", translate("Get status via SMS after reboot"), translate("Receive router status information via SMS after reboot"))
stat_e:depends("action", "reboot")
wifiw = s:option(Flag, "write_wifi", translate("Write to config"), translate("Permanently save wireless network state to configuration"))
wifiw:depends("action", "wifi")
wifiw.rmempty = false

w3g = s:option(Flag, "write_mobile", translate("Write to config"), translate("Permanently save mobile network state to configuration"))
w3g:depends("action", "mobile")
w3g.rmempty = false

send = s:option(DummyValue, "getinfo", translate("Get information:"), translate("Which status information should be included in SMS"))
send:depends("action", "send_status")
send:depends("status_sms", "1")
--[[
send = s:option(Flag, "date_state",translate("Data state"), translate("Include the mobile data connection state in status SMS"))
send:depends("action", "send_status")
send:depends("status_sms", "1")

send = s:option(Flag, "operator", translate("Operator"), translate("Include the operator ID of the mobile network in status SMS"))
send:depends("action", "send_status")
send:depends("status_sms", "1")

send = s:option(Flag, "connection_type", translate("Connection type"), translate("Include the access technology type in status SMS"))
send:depends("action", "send_status")
send:depends("status_sms", "1")

send = s:option(Flag, "signal_strength", translate("Signal strength"), translate("Include signal strength measured in dBm in status SMS"))
send:depends("action", "send_status")
send:depends("status_sms", "1")

send = s:option(Flag, "network_state", translate("Network state"), translate("Include the mobile data connection state in status SMS"))
send:depends("action", "send_status")
send:depends("status_sms", "1")

send = s:option(Flag, "ip", translate("WAN IP"), translate("Include the IP address that the router uses to connect to the internet"))
send:depends("action", "send_status")
send:depends("status_sms", "1")
]]
message = s:option(Value, "message", translate("Message text"), translate("Message to send. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
message:depends("action", "send_status")
message:depends("status_sms", "1")
message.template = "sms-utilities/status_textbox"
message.default = "Router name - %rn; WAN IP - %wi; Data Connection state - %cs; Connection type - %ct; Signal Strength - %ss; New FW available - %fs;"
message.rows = "6"
message.indicator = arg[1]

outputtype = s:option(ListValue, "outputnb", translate("Output type"), translate("Type of output which will be activated"))
outputtype:depends("action", "dout")

if in_out == "1"  then
	outputtype:value("DOUT1", translate("Digital OC output"))
	outputtype:value("DOUT2", translate("Relay output"))
end
if is_io == "1" then
	outputtype:value("DOUT3", translate("4PIN output"))
end

return m
