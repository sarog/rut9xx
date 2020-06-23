local uci = require "luci.model.uci".cursor()
local ds = require "luci.dispatcher"
local wa = require "luci.tools.webadmin"

local function parseFunctionFactory(enabledO, actionO, errmsg, action)
	return function(self, section, novld, ...)
		local alarmon = enabledO:formvalue(section)
		local actionv = actionO and actionO:formvalue(section) or ""
		local value = self:formvalue(section)
		if action ~= nil then
			if alarmon == "1" and actionv == action and (value == nil or value == "") then
				self:add_error(section, "invalid", errmsg)
			end
		else
			if alarmon == "1" and (value == nil or value == "") then
				self:add_error(section, "invalid", errmsg)
			end
		end
		Value.parse(self, section, novld, ...)
	end
end

local alarm_cfg
if arg[1] then
	alarm_cfg = arg[1]
else
	luci.http.redirect(ds.build_url("admin/services/modbus/modbus_serial_master_alarms"))
end

m = Map("modbus_master_alarms", translate("Alarm Configuration"),translate(""))
local slave_cfg_name = m.uci:get("modbus_master_alarms", alarm_cfg, "device_config")
if slave_cfg_name then
	m.redirect = ds.build_url("admin/services/modbus/modbus_serial_master_alarms/" .. slave_cfg_name)
else
	m.redirect = ds.build_url("admin/services/modbus/modbus_serial_master_alarms")
end

s = m:section(NamedSection, alarm_cfg, "Alarm configuration", "Alarm settings")

enabled = s:option(Flag, "enabled", translate("Enabled"), translate("Check to enable this alarm"))
enabled.rmempty = false

f_code = s:option(ListValue, "f_code", translate("Function Code"), translate("Modbus function code used to get the values"))
f_code:value("1", "Read Coil Status (1)")
f_code:value("2", "Read Input Status (2)")
f_code:value("3", "Read Holding Registers (3)")
f_code:value("4", "Read Input Registers (4)")
f_code.default = "1"

register = s:option(Value, "register", translate("Register"), translate("Modbus register or coil (1-65536)"))
register.datatype = "range(1,65536)"
register.parse = parseFunctionFactory(enabled, nil, "Error: first register field is empty", nil)

condition = s:option(ListValue, "condition", translate("Condition"), translate("Condition for comparing values read with configured values"))
condition:value("0", "More than")
condition:value("1", "Less than")
condition:value("2", "Equal to")
condition:value("3", "Not Equal to")
condition.default = "equal"

value = s:option(Value, "value", translate("Value"), translate("Value to be compared with values read from slave"))
value.datatype = "range(0,65535)"
value.parse = parseFunctionFactory(enabled, nil, "Error: empty test register value", nil)

local have_io = m.uci:get("hwinfo", "hwinfo", "in_out") or "0"
local have_4pin = m.uci:get("hwinfo", "hwinfo", "4pin_io") or "0"

action = s:option(ListValue, "action", translate("Action"), translate("Action triggered by this alarm"))
action:value("0", "SMS")
if have_io == "1" then
	action:value("1", "Trigger output")
end
action:value("2", "Modbus Request")
action.default = "sms"

msg = s:option(TextValue, "msg")
msg.template = "modbus_master/sms_textarea"
msg:depends("action", "0")

telnum = s:option(Value, "telnum", translate("Phone number"), translate("Recipient's phone number"))
telnum:depends("action", "0")
telnum.datatype = "lengthvalidation(3, 20, '^[0-9+]+$')"
telnum.parse = parseFunctionFactory(enabled, action, "Error: empty telephone number text", "0")

--- Not in order because of backwards compatibility
output = s:option(ListValue, "output", translate("Output"), translate("Select output"))
output:depends("action", "1")
output:value("0", "Open collector output")
output:value("1", "Relay output")
if have_4pin == "1" then
	output:value("3", "4PIN output")
end
output:value("2", "All")

io_action = s:option(ListValue, "io_action", translate("I/O Action"), translate("Action to be performed with selected output"))
io_action:depends("action", "1")
io_action:value("1", "Turn On")
io_action:value("0", "Turn Off")
io_action:value("2", "Invert")

modbus_timeout = s:option(Value, "modbus_timeout", translate("Timeout"), translate("Time period for waiting of the tcp device response (in seconds, 1-30)"))
modbus_timeout:depends("action", "2")
modbus_timeout.datatype = "range(1,30)"
modbus_timeout.default = "5"
modbus_timeout.parse = parseFunctionFactory(enabled, action, "Error: no timeout", "2")

modbus_id = s:option(Value, "modbus_id", translate("ID"), translate("TCP slave ID number"))
modbus_id:depends("action", "2")
modbus_id.datatype = "range(1,255)"
modbus_id.parse = parseFunctionFactory(enabled, action, "Error: no ID", "2")

modbus_function  = s:option(ListValue, "modbus_function", translate("Modbus function"), translate("Modbus function code to be used for this request"))
modbus_function:depends("action", "2")
modbus_function:value("5", translate("Set Single Coil (5)"))
modbus_function:value("6", translate("Set Single Register (6)"))
modbus_function:value("15", translate("Set Multiple Coils (15)"))
modbus_function:value("16", translate("Set Multiple Registers (16)"))
modbus_function.default = "5"

modbus_first_reg = s:option(Value, "modbus_first_reg", translate("First register"), translate("Start Register/Coil/Input number (0-65535)"))
modbus_first_reg:depends("action", "2")
modbus_first_reg.datatype = "range(1,65536)"
modbus_first_reg.parse = parseFunctionFactory(enabled, action, "Error: no first register", "2")

modbus_reg_count = s:option(Value, "modbus_reg_count", translate("Values"), translate("Values to be written (multiple values must be separated by space character)"))
modbus_reg_count:depends("action", "2")
modbus_reg_count.parse = parseFunctionFactory(enabled, action, "Error: no register count", "2")

s = m:section(SimpleSection)
s.template = "modbus_master/cbi_alarm_details"

return m
