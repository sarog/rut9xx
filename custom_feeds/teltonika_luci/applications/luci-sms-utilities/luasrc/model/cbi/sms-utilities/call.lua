local ds = require "luci.dispatcher"
local uci = require "luci.model.uci".cursor()

local lte = uci:get("network", "ppp", "service") or "auto"
local m, s, o


m = Map("call_utils", translate("Call Utilities"),translate(""))
if lte == "lte-only" then
	m.message = translatef("err: Call utilities will not work as 4G-only service mode is on")
end
s = m:section(TypedSection, "rule", translate("Call Rules"))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.sortable  = true
s.extedit   = ds.build_url("admin/services/sms/call-utilities/%s")
s.template_addremove = "sms-utilities/cbi_addcall_rule"
s.novaluetext = translate("There are no Call rules created yet")
s.validate = function(self, section)
	local action = self.map:get(section, "action") or ""
	local in_out = self.map.uci:get("hwinfo","hwinfo","in_out") or "0"
	local is_io = self.map.uci:get("hwinfo","hwinfo","4pin_io") or "0"
	local gps = self.map.uci:get("hwinfo","hwinfo","gps") or "0"

	if (in_out == "0" and is_io == "0") and (action == "iostatus" or action == "dout") then
		return
	elseif gps == "0" and (action == "gps_coordinates" or action == "gps") then
		return
	end

	return section
end

function s.create(self, section)
	local a = m:formvalue("_newinput.action")
	created = TypedSection.create(self, section)
	self.map:set(created, "action", a)
end

function s.parse(self, ...)
	TypedSection.parse(self, ...)
	if created then
		m.uci:save("call_utils")
		luci.http.redirect(ds.build_url("admin/services/sms/call-utilities", created ))
	end
end

src = s:option(DummyValue, "action", translate("Action"), translate("The action to be performed when a rule is met"))
src.rawhtml = true
src.width   = "65%"
function src.cfgvalue(self, s)
	local z = self.map:get(s, "action")
	if z == "send_status" then
		return translate("Get status")
	elseif z == "reboot" then
		return translate("Reboot")
	elseif z == "wifi" then
		local state = self.map:get(s, "value")
		if state == "off" then
			return translate("Switch WiFi off")
		else
			return translate("Switch WiFi on")
		end
	elseif z == "mobile" then
		local state = self.map:get(s, "value")
		if state == "off" then
			return translate("Switch mobile data off")
		else
			return translate("Switch mobile data on")
		end
	elseif z == "dout" then
		local state = self.map:get(s, "value")
		if state == "off" then
			return translate("Switch output off")
		else
			return translate("Switch output on")
		end
	elseif z == "firstboot" then
			return translate("Restore to default")
	else
		return translate("N/A")
	end
end

o = s:option(Flag, "enabled", translate("Enable"), translate("Make a rule active/inactive"))

s1 = m:section(TypedSection, "call", translate("Incoming Calls"))
s1.addremove = false
o2 = s1:option(Flag, "reject_incoming_calls", translate("Reject unrecognized incoming calls"), translate("If a call is made from number that is not in the active rule list, it can be rejected with this option"))
o2.rmempty = false

o3 = s1:option(Value, "line_close_time", translate("Answer and hangup after time period (s)"), translate("Time interval (seconds) until line will be closed after answering the call."))
o3.datatype = "string_not_empty"
o3:depends("reject_incoming_calls", "")
function o3.validate(self, value)
	val = tonumber(value)
	if not val then return nil end
	if val < 0 then return nil end
	if val > 100 then return nil end
	return value
end

return m
