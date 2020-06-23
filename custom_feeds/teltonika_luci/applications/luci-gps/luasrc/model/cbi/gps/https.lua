local ds = require "luci.dispatcher"
local ft = require "luci.tools.gps"

map = Map("gps")

https_settings = map:section(NamedSection, "https", nil, "HTTPS/HTTP Server Settings")

enabled = https_settings:option(Flag, "enabled",
                              translate("Enabled"),
                              translate("Enable NMEA forwarding to remote server in HTTPS protocol"))
enabled.rmempty = false

hostname = https_settings:option(Value, "hostname",
                              translate("URL"),
                              translate("URL of the remote server (ex. example.com/xxxx)"))
hostname.rmempty = false
hostname.datatype = "fieldvalidation('^[a-zA-Z0-9!@#:%$%%&%*%+%-/=%?%^_`{|}~%. ]+$',0)"

--[[
--  TAVL settings section
--]]

local cfg_in_out = map.uci:get("hwinfo", "hwinfo", "in_out") or "0"

tavl = map:section(TypedSection, "https_tavl", translate("TAVL Settings"))
o = tavl:option(Flag, "send_gsm_signal", "Send GSM signal", translate("Check to include GSM signal strength information in GPS data package to be sent to server"))
if cfg_in_out == "1" then
	o = tavl:option(Flag, "send_analog_input", "Send analog input", translate("Check to include analog input state in GPS data package to be sent to server"))
	o = tavl:option(Flag, "send_digital_input1", "Send digital input (1)", translate("Check to include digital input #1 state in GPS data package to be sent to server"))
	o = tavl:option(Flag, "send_digital_input2", "Send digital input (2)", translate("Check to include digital input #2 state in GPS data package to be sent to server"))
end
return map
