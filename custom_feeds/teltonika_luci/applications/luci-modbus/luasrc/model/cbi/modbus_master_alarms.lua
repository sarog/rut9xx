local uci = require "luci.model.uci".cursor()
local ds = require "luci.dispatcher"

if arg[1] then
	alarm_cfg = arg[1]
else
	luci.http.redirect(ds.build_url("admin/services/modbus/modbus_master"))
end

m = Map("modbus_master_alarms", translate("Modbus Master Alarms"), translate(""))

m.redirect = ds.build_url("admin/services/modbus/modbus_master")

s = m:section(TypedSection, "alarm_" .. alarm_cfg, translate("Modbus Master Alarms"))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.sortable  = false
s.main_toggled = true
s.extedit   = ds.build_url("admin/services/modbus/modbus_master_alarm_details/%s")
s.novaluetext = translate("No Modbus Master alarms created yet")
s.defaults["device_config"] = alarm_cfg

function s.parse(self, ...)
	TypedSection.parse(self, ...)

	local i_i = m:formvalue("_newopen.f_code")
	local i_x = m:formvalue("_newopen.submit")

	local f_i = m:formvalue("_newfwd.f_code")
	local f_x = m:formvalue("_newfwd.submit")

	if i_x then
		created = TypedSection.create(self, section)
		self.map:set(created, "f_code",		i_i)

	elseif f_x then
		created = TypedSection.create(self, section)
		self.map:set(created, "f_code",		f_i)
	end

	if created then
		uci:set("modbus_master_alarms", created, "section_id", alarm_cfg)
		m.uci:save("modbus_master_alarms")
		m.uci:commit("modbus_master_alarms")
		luci.http.redirect(ds.build_url(
			"admin/services/modbus/modbus_master_alarm_details", created
		))
	end
end

function s.remove(self, section)
	--when deleting alarm also delete alarm requests

	if (section) then
		local slave_cfg_name = m.uci:get("modbus_master_alarms", section, "device_config")
		if (slave_cfg_name) then
			m.uci:foreach("modbus_tcp_master", "register_" .. slave_cfg_name, function(s)
				if s["alarm_cfg"] == section then
					m.uci:delete("modbus_tcp_master", s[".name"])
				end
			end)
		end
	end

	m.uci:delete("modbus_master_alarms", section)
	m.uci:save("modbus_master_alarms")
	m.uci:commit("modbus_master_alarms")

	m.uci:save("modbus_tcp_master")
	m.uci:commit("modbus_tcp_master")
	luci.http.redirect(ds.build_url("admin/services/modbus/modbus_master_alarms/" .. alarm_cfg))
end


f_code = s:option(DummyValue, "f_code", translate("Function"), translate("Modbus Function"))
f_code.rawhtml = true
f_code.width   = "15%"
function f_code.cfgvalue(self, s)
	local x = self.map:get(s, "f_code")
	local t = {"Read Coil Status (1)", "Read Input Status (2)", "Read Holding Registers (3)", "Read Input Registers (4)"}
	if x == "1" or x == "2" or x == "3" or x == "4" then
		return t[tonumber(x)]
	else
		return "N/A"
	end
end

register = s:option(DummyValue, "register", translate("Register"), translate("Register or coil number (or range) to be checked"))
register.rawhtml = true
register.width   = "10%"
function register.cfgvalue(self, s)
	return self.map:get(s, "register") or "N/A"
end

condition = s:option(DummyValue, "condition", translate("Condition"), translate("Condition for comparing values read with configured values"))
condition.rawhtml = true
condition.width   = "15%"
function condition.cfgvalue(self, s)
	local condition = self.map:get(s, "condition")
	if condition == "more" then
		return "More Than"
	elseif condition == "less" then
		return "Less Than"
	elseif condition == "equal" then
		return "Equal"
	elseif condition == "not" then
		return "Not Equal"
	else
		return "N/A"
	end
end

value = s:option(DummyValue, "value", translate("Value"), translate("Register or coil value used to check for the alarm conditions"))
value.rawhtml = true
value.width   = "15%"
function value.cfgvalue(self, s)
	return self.map:get(s, "value") or "N/A"
end

action = s:option(DummyValue, "action", translate("Action"), translate("Action triggered by this alarm"))
action.rawhtml = true
action.width   = "15%"
function action.cfgvalue(self, s)
	local action = self.map:get(s, "action")
	if action == "sms" then
		return "SMS"
	elseif action == "email" then
		return "Email"
	elseif action == "io" then
		return "Output"
	elseif action == "modbus_tcp" then
		return "Modbus write request"
	else
		return "N/A"
	end
end

enabled = s:option(Flag, "enabled", translate("Enabled"), translate("Check to enable this alarm"))
enabled.rawhtml = true
enabled.width   = "5%"

function enabled:parse(section, novld)
	local xenabled = self:formvalue(section)
	local xregister = uci:get("modbus_master_alarms", section, "register")
	local xvalue = uci:get("modbus_master_alarms", section, "value")
	local xaction = uci:get("modbus_master_alarms", section, "action")

	if xenabled ~= "1" then
		Value.parse(self, section, novld)
		return
	end

	if type(xregister) ~= "string" or xregister == "" then
		self:add_error(section, "invalid", "Empty test register number, unable to enable the alarm")
	end

	if xaction == "sms" then
		local xphonenumber = uci:get("modbus_master_alarms", section, "telnum")
		if type(xphonenumber) ~= "string" or xphonenumber == "" then
			self:add_error(section, "invalid", "Empty phone number, unable to enable the alarm")
		end
	elseif xaction == "modbus_tcp" then
		local xrequestip = uci:get("modbus_master_alarms", section, "modbus_ip_addr")
		local xrequestport = uci:get("modbus_master_alarms", section, "modbus_port")
		local xrequesttimeout = uci:get("modbus_master_alarms", section, "modbus_timeout")
		local xrequestid = uci:get("modbus_master_alarms", section, "modbus_id")
		local xrequestregister = uci:get("modbus_master_alarms", section, "modbus_first_reg")
		local xrequestvalues = uci:get("modbus_master_alarms", section, "modbus_reg_count")
		if type(xrequestip) ~= "string" or xrequestip == "" then
			self:add_error(section, "invalid", "Invalid MODBUS write request: no IP, unable to enable the alarm")
		end
		if type(xrequestport) ~= "string" or xrequestport == "" then
			self:add_error(section, "invalid", "Invalid MODBUS write request: no port, unable to enable the alarm")
		end
		if type(xrequesttimeout) ~= "string" or xrequesttimeout == "" then
			self:add_error(section, "invalid", "Invalid MODBUS write request: no timeout, unable to enable the alarm")
		end
		if type(xrequestid) ~= "string" or xrequestid == "" then
			self:add_error(section, "invalid", "Invalid MODBUS write request: no ID, unable to enable the alarm")
		end
		if type(xrequestregister) ~= "string" or xrequestregister == "" then
			self:add_error(section, "invalid", "Invalid MODBUS write request: no register number, unable to enable the alarm")
		end
		if type(xrequestvalues) ~= "string" or xrequestvalues == "" then
			self:add_error(section, "invalid", "Invalid MODBUS write request: no register values, unable to enable the alarm")
		end
	end

	Value.parse(self, section, novld)
end

return m
