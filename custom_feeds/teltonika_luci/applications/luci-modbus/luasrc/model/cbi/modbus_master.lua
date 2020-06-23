local uci = require "luci.model.uci".cursor()
local ds = require "luci.dispatcher"
local sys = require "luci.sys"

m = Map("modbus_tcp_master", translate("Modbus TCP Master"), translate("Modbus TCP master periodically sends modbus requests to modbus slave devices. Data collected from TCP slaves is stored in a local database residing in RAM (lost after reboot)."))

s = m:section(TypedSection, "tcp_slave", translate("Modbus TCP slave devices"))
s.addremove = true
s.anonymous = true
s.sortable = false
s.novaluetext = translate("No Modbus TCP slaves added yet")
s.template = "cbi/tblsection"

function s.extedit(self, section)
	local section_id = self.map.uci:get("modbus_tcp_master", section, "section_id")
	section_id = (type(section_id) == "string" and section_id or "")
	return ds.build_url("admin", "services", "modbus", "modbus_master", section_id)
end

function s.create(self, section, ...)
	local section_id = sys.exec("head -c 32 /dev/urandom | md5sum | awk '{ print $1 }' | tr -d '\n'")
	local created = TypedSection.create(self, section, ...)
	self.map:set(created, "section_id", section_id)
end

function s.remove(self, section)
	--when deleting slave device, also delete all configured registers
	local register_cfg_name
	if (section) then
		local section_id = self.map.uci:get(self.config, section, "section_id")
		if (section_id) then
			m.uci:foreach("modbus_tcp_master", "register_" .. section_id, function(s)
				register_cfg_name = s[".name"] or ""
				m.uci:delete("modbus_tcp_master", register_cfg_name)
			end)
		end
	end

	--Also delete all the alarms for this device
	if (section) then
		local section_id = self.map.uci:get(self.config, section, "section_id")
		if (section_id) then
			m.uci:foreach("modbus_master_alarms", "alarm_" .. section_id, function(z)
				alarm_cfg_name = z[".name"] or ""
				m.uci:delete("modbus_master_alarms", alarm_cfg_name)
			end)
		end
	end


	m.uci:delete("modbus_tcp_master", section)
	m.uci:save("modbus_tcp_master")
	m.uci:commit("modbus_tcp_master")

	m.uci:save("modbus_master_alarms")
	m.uci:commit("modbus_master_alarms")

	luci.http.redirect(ds.build_url("admin/services/modbus/modbus_master"))
end

name = s:option(DummyValue, "name", translate("Name"), translate("Name of the slave device. Used for easier device management purposes only"))
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

slave_id = s:option(DummyValue, "slave_id", translate("ID"), translate("TCP slave ID number"))
function slave_id.cfgvalue(self, s)
	return self.map:get(s, "slave_id") or "N/A"
end

dev_ipaddr = s:option(DummyValue, "dev_ipaddr", translate("IP address"), translate("IP address of the slave device"))
function dev_ipaddr.cfgvalue(self, s)
	local set_ip = self.map:get(s, "dev_ipaddr")
	if set_ip then
		if #set_ip < 24 then
			return set_ip
		else
			return string.sub(set_ip,1,24) .. "..."
		end
	else
		return "N/A"
	end
end

period = s:option(DummyValue, "period", translate("Period"), translate("Interval for sending requests to this device (in seconds, 1-86400)"))
function period.cfgvalue(self, s)
	return self.map:get(s, "period") or "N/A"
end

timeout = s:option(DummyValue, "timeout", translate("Timeout"), translate("Time period for waiting of the TCP device response (in seconds, 1-30)"))
function timeout.cfgvalue(self, s)
	return self.map:get(s, "timeout") or "N/A"
end

enabled = s:option(Flag, "enabled")

function enabled:parse(section, novld, ...)
	local xenabled = self:formvalue(section)

	if xenabled ~= "1" then
		Value.parse(self, section, novld, ...)
		return
	end

	local xname = self.map.uci:get(self.config, section, "name")
	local xslave = self.map.uci:get(self.config, section, "slave_id")
	local xip = self.map.uci:get(self.config, section, "dev_ipaddr")
	local xport = self.map.uci:get(self.config, section, "port")
	local xperiod = self.map.uci:get(self.config, section, "period")
	local xtimeout = self.map.uci:get(self.config, section, "timeout")

	if type(xname) ~= "string" or #xname > 32 then
		self:add_error(section, "invalid", "Cannot enable a slave: invalid name")
	end

	if type(xslave) ~= "string" or #xslave > 3 or xslave:match("^[0-9]+$") ~= xslave or tonumber(xslave) < 1 or tonumber(xslave) > 255 then
		self:add_error(section, "invalid", "Cannot enable a slave: invalid ID")
	end

	if type(xip) ~= "string" or #xip > 15 or xip:match("^[0-9]+[.][0-9]+[.][0-9]+[.][0-9]+$") ~= xip then
		self:add_error(section, "invalid", "Cannot enable a slave: invalid IP")
	end

	if type(xport) ~= "string" or #xport > 5 or xport:match("^[0-9]+$") ~= xport or tonumber(xport) > 65535 then
		self:add_error(section, "invalid", "Cannot enable a slave: invalid port")
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
	local section_id = self.map.uci:get("modbus_tcp_master", section, "section_id")
	section_id = (type(section_id) == "string" and section_id or "")
	return ds.build_url("admin", "services", "modbus", "modbus_master_alarms", section_id)
end

return m
