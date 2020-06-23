m = Map("sms_utils", translate("Receive Configuration"),translate(""))

s1 = m:section(TypedSection, "rule", translate("Receive Configuration"))
-- s1.template  = "sms-utilities/tsection"
-- s1.show_rule= {"action", "get_configure"}

function TypedSection.cfgsections(self)
	local sections = {}
	self.map.uci:foreach(self.map.config, self.sectiontype,
		function (section)
			if self:checkscope(section[".name"]) then
				if section.action == "get_configure" then
					table.insert(sections, section[".name"])
				end
			end
		end)

	return sections
end

enb = s1:option(Flag, "enabled", translate("Enable"))

o = s1:option(ListValue, "authorisation", translate("Authorization method"), translate("What kind of authorization to use for SMS management"))
	o:value("no", translate("No authorization"))
	o:value("serial", translate("By serial"))
	o:value("password", translate("By router admin password"))

o = s1:option(ListValue, "allowed_phone", translate("Allowed users"), translate("Whitelist of allowed users"))
	o:value("all", translate("From all numbers"))
	o:value("group", translate("From group"))
	o:value("single", translate("From single number"))

src = s1:option(Value, "tel", translate("Sender's phone number"), translate("A whitelisted phone number. Allowable characters (0-9#*+)"))
	src.datatype = "fieldvalidation('^[0-9#*+]+$',0)"
	src:depends("allowed_phone", "single")

o = s1:option(ListValue, "group", translate("Group"), translate("A whitelisted users group"))
	local values = false
	m.uci:foreach("sms_utils", "group", function(s)
		o:value(s.name, s.name)
		values = true
	end)
	if not values then
		o:value("none", "-")
	end
	o:depends("allowed_phone", "group")
	
return m
