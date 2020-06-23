if arg[1] then
	section_id = arg[1]
else
	luci.http.redirect(ds.build_url("admin/services/modbus/modbus_serial_master/rs485/"))
end

local slave_name
local cfg_name
local request_list = "request_" .. section_id
local m = Map("modbus_serial_master", translate("Advanced device settings"), translate("Here you can add and configure requests for this serial slave device"))
m.redirect = luci.dispatcher.build_url("admin/services/modbus/modbus_serial_master/rs485/")

m.uci:foreach("modbus_serial_master", "rs485_slave", function(s)
	if s.section_id == section_id then
		cfgName = s[".name"]
		slave_name = s["name"]
	end
end)

if not slave_name or slave_name == "" then
	slave_name = "Nameless device"
end

s = m:section(NamedSection, cfgName, "Slave device configuration", translate("Slave device configuration"))

enabled = s:option(Flag, "enabled", translate("Enabled"), translate("Check to enable this Modbus serial slave configuration"))

name = s:option(Value, "name", translate("Name"), translate("Name of the slave device. Used for easier device management purposes only"))
name.datatype = "lengthvalidation(0, 32)"

slave_id = s:option(Value, "slave_id", translate("Slave ID"), translate("Modbus slave ID number"))
slave_id.datatype = "range(1,255)"

period = s:option(Value, "period", translate("Period"), translate("Interval for sending requests to this device (in seconds, 1-86400)"))
period.datatype = "range(1,86400)"
period.default = "60"

timeout = s:option(Value, "timeout", translate("Timeout"), translate("Time period for waiting of the serial device response (in seconds, 1-30)"))
timeout.datatype = "range(1,30)"
timeout.default = "5"

s = m:section(SimpleSection)
s.template = "modbus_master/testdiv2"

s = m:section(TypedSection, request_list, translate("Requests configuration"))
s.template = "cbi/tblsection"
s.template_addremove = "modbus_master/testdiv"
s.anonymous = true
s.addremove = true
s.novaluetext = translate("No requests configured for this slave device yet")

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
f_code:value("4", translate("Read input registers(4)"))
f_code:value("5", translate("Set single coil (5)"))
f_code:value("6", translate("Set single register (6)"))
f_code:value("15", translate("Set multiple coils (15)"))
f_code:value("16", translate("Set multiple registers (16)"))
f_code.default = "3"
f_code.maxWidth = "150px"

first_reg = s:option(Value, "first_reg", translate("First Register"), translate("Start Register/Coil/Input (1-65536)"))
first_reg.maxWidth = "60px"
first_reg.datatype = "range(1,65536)"
first_reg.default = "1"
first_reg.maxWidth = "100px"

reg_count = s:option(Value,"reg_count",translate("Register count / Values"),translate("Number of Registers/Coils/Inputs or actual values to be written (multiple values must be separated by space character)"))
reg_count.maxWidth = "140px"
reg_count.default = "1"
reg_count.maxWidth = "100px"

enabled = s:option(Flag, "enabled", translate("Enabled"), translate("Check to enable this request"))
enabled.maxWidth = "20px"

test = s:option(Button, "test")
test.inputtitle = "Test"
test.type="button"
test.template = "modbus_master/test"
test.fn = "dotest"

return m
