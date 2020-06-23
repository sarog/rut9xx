local sys = require "luci.sys"
local dsp = require "luci.dispatcher"
local ft = require "luci.tools.input-output"
local utl = require "luci.util"
local m, s, o

arg[1] = arg[1] or ""

m = Map("ioman",
	translate("Output Configuration"))

is_4pin = m.uci:get("hwinfo", "hwinfo", "4pin_io") or "0"
is_io = m.uci:get("hwinfo", "hwinfo", "in_out") or "0"

s = m:section(TypedSection, "ioman", translate("Default output state configuration"))
if is_io == "1" then
	state = s:option(ListValue, "default_DOUT1_status", translate("Open collector output"), translate("Default open collector output state after reboot"))
	state:value("0", translate("Low level"))
	state:value("1", translate("High level"))

	state = s:option(ListValue, "default_DOUT2_status", translate("Relay output"), translate("Default relay output state after reboot"))
	state:value("0", translate("Contacts open"))
	state:value("1", translate("Contacts closed"))
end

if is_4pin == "1" then
	state = s:option(ListValue, "default_DOUT3_status", translate("Digital output 4PIN"), translate("Default digital output state after reboot"))
	state:value("0", translate("Low level"))
	state:value("1", translate("High level"))
end

return m
