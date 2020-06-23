--[[

LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: forwards.lua 8117 2011-12-20 03:14:54Z jow $
]]--

local ds = require "luci.dispatcher"

m = Map("sms_gateway", translate("Scheduled Messages"),	translate("Configure time and text for scheduled messages."))

--
-- Port Forwards
--

s = m:section(TypedSection, "msg", translate("Messages To Send"))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.sortable  = true
s.extedit   = ds.build_url("admin/services/sms_gateway/scheduled_messages/%s")
s.template_addremove = "sms_gateway/addin_scheduled_messages"
s.novaluetext = translate("There are no scheduled messages created yet")

function s.create(self, section)
	local e = m:formvalue("_newinput.event")
	local t = m:formvalue("_newinput.type")
	
	created = TypedSection.create(self, section)
	self.map:set(created, "phonenumber", e)
	self.map:set(created, "repeats", t)
end

function s.parse(self, ...)
	TypedSection.parse(self, ...)
	if created then
		m.uci:save("sms_gateway")
		luci.http.redirect(ds.build_url("admin/services/sms_gateway/scheduled_messages", created	))
	end
end
src = s:option(DummyValue, "phonenumber", translate("Recipients number"), translate("Phone number to which the messages are going to be sent"))
src.rawhtml = true
src.width = "23%"
function src.cfgvalue(self, s)
	local z = self.map:get(s, "phonenumber")
	if z ~= nil then 
		return translatef(z)
	else
	    return translatef("NA")
	end
end

src = s:option(DummyValue, "repeats", translate("Sending Interval"), translate("Message sending interval"))
src.rawhtml = true
src.width = "23%"
function src.cfgvalue(self, s)
	local z = self.map:get(s, "repeats")
	if z ~= nil then 
		return translatef(z)
	else
	    return translatef("NA")
	end
end

en = s:option(Flag, "enable", translate("Enable"), translate("Switch to enable/disable message sending"))
en.width = "23%"

function m.on_after_save()
	
	require("uci")
	local x = uci.cursor()
	local scriptsh = "scheduled_sms_sender.sh"
	luci.sys.call('sed -i /' .. scriptsh .. '/d /etc/crontabs/root')
    luci.sys.call('sed -i /last_day_of_month.s/d /etc/crontabs/root')
	local crontab = io.open("/etc/crontabs/root", "a")
	x:foreach("sms_gateway", "msg", function(s)

			if s.enable == "1" then
                if s.monthday == "28-31" then
                    local datstring = string.format("%s %s %s * * %s %s %s", s.minute, s.hour, s.monthday, "/sbin/last_day_of_month.sh", s.phonenumber, "\n")
					crontab:write(datstring)
				elseif s.repeats == "day" then
					local datstring = string.format("%s %s * * * %s %s %s", s.minute, s.hour, "/sbin/scheduled_sms_sender.sh", s.phonenumber, "\n")
					crontab:write(datstring)
				elseif s.repeats == "week" then
					local datstring = string.format("%s %s * * %s %s %s %s", s.minute, s.hour, s.weekday, "/sbin/scheduled_sms_sender.sh", s.phonenumber, "\n")
					crontab:write(datstring)
				elseif s.repeats == "month" then
					local datstring = string.format("%s %s %s * * %s %s %s", s.minute, s.hour, s.monthday, "/sbin/scheduled_sms_sender.sh", s.phonenumber, "\n")
					crontab:write(datstring)
				elseif s.repeats == "year" then
					local datstring = string.format("%s %s %s %s * %s %s %s", s.minute, s.hour, s.monthday, s.month, "/sbin/scheduled_sms_sender.sh", s.phonenumber, "\n")
					crontab:write(datstring)
					
				end
			end
			
	end)
	crontab:close()
end

return m
