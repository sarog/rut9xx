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
	luci.http.redirect(ds.build_url("admin/services/modbus/modbus_master_alarms"))
end

local register_list = alarm_cfg
m = Map("modbus_master_alarms", translate("Alarm Configuration"), translate(""))

s = m:section(NamedSection, alarm_cfg, "Alarm configuration", "Alarm settings")

local slave_cfg_name = m.uci:get("modbus_master_alarms", alarm_cfg, "device_config")
if slave_cfg_name then
	m.redirect = ds.build_url("admin/services/modbus/modbus_master_alarms/" .. slave_cfg_name)
else
	m.redirect = ds.build_url("admin/services/modbus/modbus_master_alarms")
end

local have_io = m.uci:get("hwinfo", "hwinfo", "in_out") or "0"
local have_4pin = m.uci:get("hwinfo", "hwinfo", "4pin_io") or "0"

enabled = s:option(Flag, "enabled", translate("Enabled"), translate("Check to enable this alarm"))

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
condition:value("more", "More than")
condition:value("less", "Less than")
condition:value("equal", "Equal to")
condition:value("not", "Not Equal to")
condition.default = "equal"

value = s:option(Value, "value", translate("Value"), translate("Value to be compared with values read from slave"))
value.datatype = "range(0,65535)"
value.parse = parseFunctionFactory(enabled, nil, "Error: empty test register value", nil)

action_frequency = s:option(ListValue, "actionfrequency", translate("Action frequency"), translate("How frequency every action is triggered"))
action_frequency:value("everytrigger", translate("Every trigger"))
action_frequency:value("firsttrigger", translate("First trigger"))

redundancy_protection = s:option(Flag, "redundancy_protection", translate("Redundancy protection"), translate("Protection against executing a configured action too often"))

redundancy_protection_period = s:option(Value, "redundancy_protection_period", translate("Redundancy protection period"), translate("Duration to activate redundancy protection for, in seconds (1-86400)"))
redundancy_protection_period:depends("redundancy_protection", "1")
redundancy_protection_period.datatype = "range(1,86400)"
redundancy_protection_period.parse = parseFunctionFactory(enabled, redundancy_protection, "Error: empty redundacy protection period", "1")

action = s:option(ListValue, "action", translate("Action"), translate("Action triggered by this alarm"))
action:value("sms", "SMS")
if have_io == "1" then
	action:value("io", "Trigger output")
end
action:value("modbus_tcp", "Modbus write request")
action.default = "sms"

msg = s:option(TextValue, "msg")
msg.template = "modbus_master/sms_textarea"
msg:depends("action", "sms")
msg.parse = parseFunctionFactory(enabled, action, "Error: empty SMS text", "sms")

telnum = s:option(Value, "telnum", translate("Phone number"), translate("Recipient's phone number"))
telnum:depends("action", "sms")
telnum.datatype = "lengthvalidation(3, 20, '^[0-9+]+$')"
telnum.parse = parseFunctionFactory(enabled, action, "Error: empty telephone number text", "sms")

output = s:option(ListValue, "output", translate("Output"), translate("Select output"))
output:depends("action", "io")
output:value("dout1", "Open collector output")
output:value("dout2", "Relay output")
if have_4pin == "1" then
	output:value("dout3", "4PIN output")
end
output:value("both", "all")

io_action = s:option(ListValue, "io_action", translate("I/O Action"), translate("Action to be performed with selected output"))
io_action:depends("action", "io")
io_action:value("on", "Turn On")
io_action:value("off", "Turn Off")
io_action:value("invert", "Invert")

modbus_ip_addr = s:option(Value, "modbus_ip_addr", translate("IP address"), translate("IP address of this slave device"))
modbus_ip_addr:depends("action", "modbus_tcp")
modbus_ip_addr.datatype = "ip4addr"
modbus_ip_addr.parse = parseFunctionFactory(enabled, action, "Error: no host", "modbus_tcp")

modbus_port = s:option(Value, "modbus_port", translate("Port"), translate("Slave device port number"))
modbus_port:depends("action", "modbus_tcp")
modbus_port.datatype = "port"
modbus_port.parse = parseFunctionFactory(enabled, action, "Error: no port", "modbus_tcp")

modbus_timeout = s:option(Value, "modbus_timeout", translate("Timeout"), translate("Time period for waiting of the tcp device response (in seconds, 1-30)"))
modbus_timeout:depends("action", "modbus_tcp")
modbus_timeout.datatype = "range(1,30)"
modbus_timeout.default = "5"
modbus_timeout.parse = parseFunctionFactory(enabled, action, "Error: no timeout", "modbus_tcp")

modbus_id = s:option(Value, "modbus_id", translate("ID"), translate("TCP slave ID number"))
modbus_id:depends("action", "modbus_tcp")
modbus_id.datatype = "range(1,255)"
modbus_id.parse = parseFunctionFactory(enabled, action, "Error: no MODBUS ID", "modbus_tcp")

modbus_function  = s:option(ListValue, "modbus_function", translate("Modbus function"), translate("Modbus function code to be used for this request"))
modbus_function:depends("action", "modbus_tcp")
modbus_function:value("5", translate("Set Single Coil (5)"))
modbus_function:value("6", translate("Set Single Register (6)"))
modbus_function:value("15", translate("Set Multiple Coils (15)"))
modbus_function:value("16", translate("Set Multiple Registers (16)"))
modbus_function.default = "5"

modbus_first_reg = s:option(Value, "modbus_first_reg", translate("First register"), translate("Start Register/Coil/Input number (1-65536)"))
modbus_first_reg:depends("action", "modbus_tcp")
modbus_first_reg.datatype = "range(1,65536)"
modbus_first_reg.parse = parseFunctionFactory(enabled, action, "Error: no first register", "modbus_tcp")

modbus_reg_count = s:option(Value, "modbus_reg_count", translate("Values"), translate("Register/Coil values to be written (multiple values must be separated by space character)"))
modbus_reg_count:depends("action", "modbus_tcp")
modbus_reg_count.parse = parseFunctionFactory(enabled, action, "Error: no register count info", "modbus_tcp")

function m.on_after_commit(self)
	local config_exists_already = 0
	local new_cfg

	if alarm_cfg then
		local device_config = m.uci:get("modbus_master_alarms", alarm_cfg, "device_config")
		if device_config then
			uci:foreach("modbus_tcp_master", "register_" .. device_config, function(s)
				alarm_request_cfg_name = s["alarm_cfg"]

				if alarm_request_cfg_name ==  alarm_cfg then
					new_cfg = s[".name"]
					config_exists_already = 1
				end
			end)

			if config_exists_already == 0 then
				new_cfg = uci:add("modbus_tcp_master", "register_" .. device_config)
			end

			if new_cfg then
				local f_code = m.uci:get("modbus_master_alarms", alarm_cfg, "f_code")
				local register = m.uci:get("modbus_master_alarms", alarm_cfg, "register")

				uci:set("modbus_tcp_master", new_cfg, "alarm_cfg", alarm_cfg)
				uci:set("modbus_tcp_master", new_cfg, "alarm_request", "1")
				uci:set("modbus_tcp_master", new_cfg, "enabled", "1")
				if register then
					uci:set("modbus_tcp_master", new_cfg, "first_reg", register)
				end
				uci:set("modbus_tcp_master", new_cfg, "reg_count", "1")
				if f_code then
					uci:set("modbus_tcp_master", new_cfg, "function", f_code)
				end
				uci:set("modbus_tcp_master", new_cfg, "data_type", "16bit_uint_hi_first")
				uci:set("modbus_tcp_master", new_cfg, "name", "alarm request")
				uci:save("modbus_tcp_master")
				uci:commit("modbus_tcp_master")
			end
		end
	end
end

s = m:section(SimpleSection)
s.template = "modbus_master/cbi_alarm_details"

return m
