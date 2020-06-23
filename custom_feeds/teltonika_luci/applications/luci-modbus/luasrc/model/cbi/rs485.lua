local sys = require "luci.sys"
local ds = require "luci.dispatcher"

local m = Map("modbus_serial_master", translate("RS485"), translate("Modbus RS485 master periodically sends modbus requests to slave devices. Data collected from slaves is stored in a local database residing in RAM (lost after reboot)."))

s = m:section(NamedSection, "rs485", translate("RS485 configuration"),  translate("RS485 configuration"))

if (m.uci:get("rs", "rs485", "enabled") ~= "1" or m.uci:get("modbus_serial_master", "rs485", "enabled") == "1") then
	o = s:option(Flag, "enabled", translate("Enabled"), translate("Check to enable Modbus Serial Master over RS485"))
	o.rmempty = false
else
	o = s:option(DummyValue, "", "")
	o.default = "<center>Modbus Serial Master over RS485 cannot be used if <b>Services -> RS232/RS485 -> RS485 -> Enabled</b> is on.</center>"
	o.rawhtml = true
end

o = s:option(ListValue, "baudrate", translate("Baud rate"))
o:value("300", "300")
o:value("1200", "1200")
o:value("2400", "2400")
o:value("4800", "4800")
o:value("9600", "9600")
o:value("19200", "19200")
o:value("38400", "38400")
o:value("57600", "57600")
o:value("115200", "115200")
o.default = "115200"
o.rmempty = false

o = s:option(ListValue, "parity", translate("Parity"))
o:value("none", translate("None"))
o:value("even", translate("Even"))
o:value("odd", translate("Odd"))
o.default = "none"
o.rmempty = false

o = s:option(ListValue, "stopbits", translate("Stop bits"))
o:value("1", "1")
o:value("2", "2")
o.default = "1"
o.rmempty = false

o = s:option(ListValue, "flowctrl", translate("Flow control"))
o:value("none", translate("None"))
o:value("XonXoff", translate("Xon / Xoff"))
o.default = "none"
o.rmempty = false

s = m:section(TypedSection, "rs485_slave", translate("RS485 slave device's list"))
s.add_toggled = true
s.addremove = true
s.anonymous = true
s.nosectionname = true
s.template = "cbi/tblsection"

function s.extedit(self, section)
	local section_id = self.map.uci:get("modbus_serial_master", section, "section_id")
	section_id = (type(section_id) == "string" and section_id or "")
	return ds.build_url("admin", "services", "modbus", "modbus_serial_master", "rs485", section_id)
end

function s.create(self, section, ...)
	local section_id = sys.exec("head -c 32 /dev/urandom | md5sum | awk '{ print $1 }' | tr -d '\n'")
	local created = TypedSection.create(self, section, ...)
	self.map:set(created, "section_id", section_id)
end

function s.remove(self, section)
	local section_id = 	m.uci:get(self.config, section, "section_id")

	if section_id then
		m.uci:foreach(self.config, "request_" .. section_id, function(x)
			m.uci:delete(self.config, x[".name"])
		end)

		m.uci:foreach("modbus_master_alarms", "serial_alarm_" .. section_id, function(x)
			m.uci:delete("modbus_master_alarms", x[".name"])
		end)
	end

	m.uci:delete(self.config, section)
	m.uci:save(self.config)
	m.uci:commit(self.config)

	m.uci:save("modbus_master_alarms")
	m.uci:commit("modbus_master_alarms")

	local url = luci.dispatcher.build_url("admin", "services", "modbus", "modbus_serial_master", "rs485")
	luci.http.redirect(url)
end

name = s:option(DummyValue, "_name", translate("Name"), "")
function name.cfgvalue(self, s)
	local set_name = self.map:get(s, "name")
	if set_name then
		if #set_name < 24 then
			return set_name
		else
			return string.sub(set_name,1,24) .. "..."
		end
	else
		return "Nameless device"
	end
end

slave_id = s:option(DummyValue, "slave_id", translate("ID"), translate("Serial slave ID number"))
function slave_id.cfgvalue(self, s)
	return self.map:get(s, "slave_id") or "N/A"
end

period = s:option(DummyValue, "period", translate("Period"), translate("Interval for sending requests to this device (in seconds, 1-86400"))
function period.cfgvalue(self, s)
	return self.map:get(s, "period") or "N/A"
end

timeout = s:option(DummyValue, "timeout", translate("Timeout"), translate("Time period for waiting of the modbus device response (in seconds, 1-30)"))
function timeout.cfgvalue(self, s)
	return self.map:get(s, "timeout") or "N/A"
end

enabled = s:option(Flag, "enabled", translate("Enabled"), translate("Check to enable this serial slave"))

function enabled:parse(section, novld, ...)
	local xenabled = self:formvalue(section)

	if xenabled ~= "1" then
		Value.parse(self, section, novld, ...)
		return
	end

	local xname = self.map.uci:get(self.config, section, "name")
	local xslave = self.map.uci:get(self.config, section, "slave_id")
	local xperiod = self.map.uci:get(self.config, section, "period")
	local xtimeout = self.map.uci:get(self.config, section, "timeout")

	if type(xname) ~= "string" or #xname > 32 then
		self:add_error(section, "invalid", "Cannot enable a slave: invalid name")
	end

	if type(xslave) ~= "string" or #xslave > 3 or xslave:match("^[0-9]+$") ~= xslave or tonumber(xslave) < 1 or tonumber(xslave) > 255 then
		self:add_error(section, "invalid", "Cannot enable a slave: invalid ID")
	end

	if type(xperiod) ~= "string" or #xperiod > 5 or xperiod:match("^[0-9]+$") ~= xperiod or tonumber(xperiod) < 1 or tonumber(xperiod) > 86400 then
		self:add_error(section, "invalid", "Cannot enable a slave: invalid period")
	end

	if type(xtimeout) ~= "string" or #xtimeout > 5 or xtimeout:match("^[0-9]+$") ~= xtimeout or tonumber(xtimeout) < 1 or tonumber(xtimeout) > 30 then
		self:add_error(section, "invalid", "Cannot enable a slave: invalid timeout")
	end

	Value.parse(self, section, novld, ...)
end

alarms = s:option(Button, "alarms")
alarms.inputtitle = "Alarms"
alarms.template = "modbus_master/alarmbutt"
alarms.type="button"
function alarms.extedit(self, section)
	local section_id = self.map.uci:get("modbus_serial_master", section, "section_id")
	section_id = (type(section_id) == "string" and section_id or "")
	return ds.build_url("admin", "services", "modbus", "modbus_serial_master_alarms", section_id)
end

return m
