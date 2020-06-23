local m, s, o
local ds = require "luci.dispatcher"
local uci = require "luci.model.uci".cursor()

m = Map("sms_utils", translate("SMS Utilities"),translate(""))

s = m:section(TypedSection, "rule", translate("SMS Rules"))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.sortable  = true
s.extedit   = ds.build_url("admin/services/sms/sms-utilities/%s")
s.template_addremove = "sms-utilities/cbi_addsms_rule"
s.novaluetext = translate("There are no SMS rules created yet")
s.validate = function(self, section)
	local action = self.map:get(section, "action") or ""
	local in_out = self.map.uci:get("hwinfo","hwinfo","in_out") or "0"
	local is_io = self.map.uci:get("hwinfo","hwinfo","4pin_io") or "0"
	local gps = self.map.uci:get("hwinfo","hwinfo","gps") or "0"

	if action == "get_configure" or action == "set_configure" then
		return
	elseif (in_out == "0" and is_io == "0") and (action == "iostatus" or action == "dout") then
		return
	elseif gps == "0" and (action == "gps_coordinates" or action == "gps") then
		return
	end

	return section
end

function is_checked()
	local checked = ""
	m.uci:foreach("sms_utils", "rule", function(s)
		if s.action ~= "get_configure" and s.action ~= "set_configure" then
			if s.enabled and s.enabled == "1" then
				checked = "checked"
			end
		end
	end)

	return checked
end

function s.create(self, section)
	local t = m:formvalue("_newinput.smstext") or ""
	local tr = m:formvalue("_newinput.tel") or ""
	local a = m:formvalue("_newinput.action")

	created = TypedSection.create(self, section)
	self.map:set(created, "smstext",   t)
	self.map:set(created, "tel", tr)
	self.map:set(created, "action", a)
end

function s.parse(self, ...)
	TypedSection.parse(self, ...)
	if created then
		m.uci:save("sms_utils")
		luci.http.redirect(ds.build_url("admin/services/sms/sms-utilities", created	))
	end
end

function s.cfgsections(self)
	local sections = {}
	self.map.uci:foreach(self.map.config, self.sectiontype,
		function (section)
			if (self:checkscope(section[".name"]) and section["action"] and section["action"] ~= "set_configure") or self:checkscope(section[".name"]) then
				table.insert(sections, section[".name"])
			end
		end)

	return sections
end

o = s:option(Flag, "enabled", translate("Enable"), translate("Make a rule active/inactive"))
o.width = "10%"
o.useCustomHTML = true
o.customHTML = "<div id=\"select_all_sms\"><input " .. is_checked() .. " type='checkbox' onclick=\"select_all(this, 'cbi-input-checkbox');\"></div>"

src = s:option(DummyValue, "action", translate("Action"), translate("The action to be performed when a rule is met"))
src.rawhtml = true
src.width   = "20%"
function src.cfgvalue(self, s)
	local z = self.map:get(s, "action")
	if z == "send_status" then
		return translate("Get status")
	elseif z == "iostatus" then
		return translate("Get I/O status")
	elseif z == "vpnstatus" then
		return translate("Get OpenVPN status")
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
	elseif z == "change_mobile_settings" then
		return translate("Change mobile data settings")
	elseif z == "list_of_profile" then
		return translate("Get list of profiles")
	elseif z == "change_profile" then
		return translate("Change profile")
	elseif z == "vpn" then
		return translate("Manage OpenVPN")
	elseif z == "dout" then
		local state = self.map:get(s, "value")
		if state == "off" then
			return translate("Switch output off")
		else
			return translate("Switch output on")
		end
	elseif z == "ssh_access" then
		return translate("SSH access Control")
	elseif z == "web_access" then
		return translate("Web access Control")
	elseif z == "firstboot" then
		return translate("Restore to default")
	elseif z == "switch_sim" then
		return translate("Force switch SIM")
	elseif z == "gps_coordinates" then
		return translate("GPS coordinates")
	elseif z == "fw_upgrade" then
		return translate("Force FW upgrade from server")
	-- elseif z == "config_update" then
	-- 	return translate("Force Config update from server")
	elseif z == "monitoring" then
		local state = self.map:get(s, "value")
		if state == "off" then
			return translate("Switch monitoring off")
		else
			return translate("Switch monitoring on")
		end
	elseif z == "gps" then
		local state = self.map:get(s, "value")
		if state == "off" then
			return translate("GPS off")
		else
			return translate("GPS on")
		end
	elseif z == "monitoring_status" then
		return translate("Monitoring status")
	elseif z == "uci" then
		return translate("UCI API")
	elseif z == "wol" then
		return translate("WOL")
	elseif z == "more" then
		return translate("MORE")
	else
		return translate("N/A")
	end
end

src = s:option(DummyValue, "smstext", translate("SMS Text"), translate("SMS text that is required to trigger the rule. If authorization is used, password or serial number should be in the beginning of the message and separated with a space character from the rest of the configured message text"))
src.rawhtml = true
src.width   = "20%"

--src = s:option(DummyValue, "tel", translate("Sender's phone number"), translate("Text of message which will be expected to do the action"))
--src.rawhtml = true
--src.width   = "20%"

o = s:option(ListValue, "authorisation", translate("Authorization method"), translate("What kind of authorization to use for SMS management"))
	o:value("no", translate("No authorization"))
	o:value("serial", translate("By serial"))
	o:value("password", translate("By router admin password"))
	o.width   = "20%"

local save = m:formvalue("cbi.apply")
if save then
	--Delete all usr_enable from sms_utils config
	m.uci:foreach("sms_utils", "rule", function(s)
		sms_inst = s[".name"] or ""
		smsEnable = m:formvalue("cbid.sms_utils." .. sms_inst .. ".enabled") or "0"
		sms_enable = s.enabled or "0"
		if smsEnable ~= sms_enable then
			m.uci:foreach("sms_utils", "rule", function(a)
				sms_inst2 = a[".name"] or ""
				local usr_enable = a.usr_enable or ""
				if usr_enable == "1" then
					m.uci:delete("sms_utils", sms_inst2, "usr_enable")
				end
			end)
		end
	end)
	m.uci:save("sms_utils")
	m.uci.commit("sms_utils")
end

return m
