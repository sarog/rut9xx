local uci = require "luci.model.uci".cursor()
local ds = require "luci.dispatcher"
local wa = require "luci.tools.webadmin"

local slave_cfg
if arg[1] then
	slave_cfg = arg[1]
else
	luci.http.redirect(ds.build_url("admin/services/modbus/modbus_master"))
end

local register_list = "register_" .. slave_cfg
local alert_list = "alert_"  .. slave_cfg

m = Map("modbus_tcp_master", translate("Advanced device settings"), translate("Here you can add and configure request parameters and alarms for this TCP slave device"))
m.redirect = ds.build_url("admin/services/modbus/modbus_master")

local cfgName
m.uci:foreach("modbus_tcp_master", "tcp_slave", function(s)
	if s.section_id == slave_cfg then
		cfgName = s[".name"]
	end
end)

s = m:section(NamedSection, cfgName, "Slave device configuration", "Slave device configuration")

enabled = s:option(Flag, "enabled", translate("Enabled"), translate("Check to enable this Modbus TCP slave configuration"))

local function parseFunctionFactory(errmsg)
	return function(self, section, novld, ...)
		local e = enabled:formvalue(section)
		local v = self:formvalue(section)
		if e == "1" and (type(v) ~= "string" or v == "") then
			self:add_error(section, "invalid", errmsg)
		end
		Value.parse(self, section, novld, ...)
	end
end

name = s:option(Value, "name", translate("Name"), translate("Name of the slave device. Used for easier device management purposes only"))
name.datatype = "lengthvalidation(0, 32)"
name.parse = parseFunctionFactory("Name cannot be empty")

slave_id = s:option(Value, "slave_id", translate("Slave ID"), translate("TCP slave ID number"))
slave_id.datatype = "range(1,255)"
slave_id.parse = parseFunctionFactory("Slave ID cannot be empty")

dev_ipaddr = s:option(Value, "dev_ipaddr", translate("IP address"), translate("IP address of the slave device"))
dev_ipaddr.datatype = "ip4addr"
dev_ipaddr.parse = parseFunctionFactory("IP address cannot be empty")

port = s:option(Value, "port", translate("Port"), translate("Slave device Port"))
port.datatype = "port"
port.parse = parseFunctionFactory("Port cannot be empty")

period = s:option(Value, "period", translate("Period"), translate("Interval for sending requests to this device (in seconds, 1-86400)"))
period.datatype = "range(1,86400)"
period.default = "60"
period.parse = parseFunctionFactory("Period cannot be empty")

timeout = s:option(Value, "timeout", translate("Timeout"), translate("Time period for waiting of the TCP device response (in seconds, 1-30)"))
timeout.datatype = "range(1,30)"
timeout.default = "5"
timeout.parse = parseFunctionFactory("Timeout cannot be empty")

s = m:section(SimpleSection)
s.template = "modbus_master/testdiv2"

s = m:section(TypedSection, register_list, translate("Requests configuration"))
s.template = "cbi/tblsection"
s.template_addremove = "modbus_master/testdiv"
s.anonymous = true
s.addremove = true
s.novaluetext = translate("No requests configured for this slave device yet")
function s:filter(section)
	local is_alarm_request = self.map.uci:get(self.config, section, "alarm_request")
	return (is_alarm_request ~= "1")
end

name = s:option(Value, "name", translate("Name"), translate("Name of this request (only used for easier identification of the request or its meaning)"))
name.datatype = "lengthvalidation(0,32)"
name.default = "Unnamed"
name.maxWidth = "100px"

data_type = s:option(ListValue, "data_type", translate("Data type"), translate("Select data type that will be used for storing the response data"))
data_type:value("8bit_int", translate("8bit INT"))
data_type:value("8bit_uint", translate("8bit UINT"))
data_type:value("16bit_int_hi_first", translate("16bit INT, high byte first"))
data_type:value("16bit_int_low_first", translate("16bit INT, low byte first"))
data_type:value("16bit_uint_hi_first", translate("16bit UINT, high byte first"))
data_type:value("16bit_uint_low_first", translate("16bit UINT, low byte first"))
data_type:value("32bit_float1234", translate("32bit float, Byte order 1,2,3,4"))
data_type:value("32bit_float4321", translate("32bit float, Byte order 4,3,2,1"))
data_type:value("32bit_float2143", translate("32bit float, Byte order 2,1,4,3"))
data_type:value("32bit_float3412", translate("32bit float, Byte order 3,4,1,2"))
data_type:value("32bit_int1234", translate("32bit INT, Byte order 1,2,3,4"))
data_type:value("32bit_int4321", translate("32bit INT, Byte order 4,3,2,1"))
data_type:value("32bit_int2143", translate("32bit INT, Byte order 2,1,4,3"))
data_type:value("32bit_int3412", translate("32bit INT, Byte order 3,4,1,2"))
data_type:value("32bit_uint1234", translate("32bit UINT, Byte order 1,2,3,4"))
data_type:value("32bit_uint4321", translate("32bit UINT, Byte order 4,3,2,1"))
data_type:value("32bit_uint2143", translate("32bit UINT, Byte order 2,1,4,3"))
data_type:value("32bit_uint3412", translate("32bit UINT, Byte order 3,4,1,2"))
data_type.default = "16bit_int_hi_first"
data_type.maxWidth = "150px"

f_code = s:option(ListValue, "function", translate("Function"), translate("Select Modbus function code for the request"))
f_code:value("1", translate("Read coils (1)"))
f_code:value("2", translate("Read input coils (2)"))
f_code:value("3", translate("Read holding registers (3)"))
f_code:value("4", translate("Read input registers (4)"))
f_code:value("5", translate("Set single coil (5)"))
f_code:value("6", translate("Set single holding register (6)"))
f_code:value("15", translate("Set multiple coils (15)"))
f_code:value("16", translate("Set multiple holding registers (16)"))
f_code.default = "3"
f_code.maxWidth = "150px"

first_reg = s:option(Value, "first_reg", translate("First register"), translate("Start Register/Coil/Input (1-65536)"))
first_reg.datatype = "range(1,65536)"
first_reg.default = "1"
first_reg.rmempty = false
first_reg.maxWidth = "100px"

reg_count = s:option(Value, "reg_count", translate("Register count / Values"),translate("Number of Registers/Coils/Inputs or actual values to be written (Multiple values must be separated by space character)"))
reg_count.default = "1"
reg_count.rmempty = false
reg_count.maxWidth = "100px"

enabled = s:option(Flag, "enabled", translate("Enabled"), translate("Check to enable this request"))
enabled.maxWidth = "20px"

test = s:option(Button, "test")
test.inputtitle = "Test"
test.type="button"
test.template = "modbus_master/test"
test.maxWidth = "50px"
test.fn = "do_test"

return m
