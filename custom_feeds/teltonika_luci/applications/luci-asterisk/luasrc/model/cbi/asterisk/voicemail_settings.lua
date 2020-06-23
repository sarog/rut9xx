--[[
LuCI - Lua Configuration Interface

Copyright 2009 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: voicemail_settings.lua 4396 2009-03-30 17:01:05Z jow $
]]--

require "luci.sys.zoneinfo"


cbimap = Map("asterisk", translate("Voicemail - Common Settings"))

voicegeneral = cbimap:section(TypedSection, "voicegeneral", translate("General Voicemail Options", "Common settings for all mailboxes are defined here. Most of them are optional. The storage format should never be changed once set."))

voicegeneral.anonymous = true
voicegeneral.addremove = false

format = voicegeneral:option(MultiValue, translate("Used storage formats"))
format.widget = "checkbox"
format:value("wav49")
format:value("gsm")
format:value("wav")

voicegeneral:option(Flag, "sendvoicemail", translate("Enable sending of emails"))
voicegeneral:option(Flag, "attach", translate("Attach voice messages to emails"))
voicegeneral:option(Value, "serveremail", translate("Used email sender address"))
voicegeneral:option(Value, "emaildateformat", translate("Date format used in emails")).optional = true
voicegeneral:option(Value, "maxlogins", translate("Max. failed login attempts")).optional = true
voicegeneral:option(Value, "maxmsg", translate("Max. allowed messages per mailbox")).optional = true
voicegeneral:option(Value, "minmessage", translate("Min. number of seconds for voicemail")).optional = true
voicegeneral:option(Value, "maxmessage", translate("Max. number of seconds for voicemail")).optional = true
voicegeneral:option(Value, "maxsilence", translate("Seconds of silence until stop recording")).optional = true
voicegeneral:option(Value, "maxgreet", translate("Max. number of seconds for greetings")).optional = true
voicegeneral:option(Value, "skipms", translate("Milliseconds to skip for rew./ff.")).optional = true
voicegeneral:option(Value, "silencethreshold", translate("Threshold to detect silence")).optional = true


voicezone = cbimap:section(TypedSection, "voicezone", translate("Time Zones"),
	translate("Time zones define how dates and times are expressen when used in an voice mails. Refer to the asterisk manual for placeholder values."))

voicezone.addremove = true
voicezone.sectionhead = translate("Name")
voicezone.template = "cbi/tblsection"

tz = voicezone:option(ListValue, "zone", translate("Location"))
for _, z in ipairs(luci.sys.zoneinfo.TZ) do tz:value(z[1]) end

voicezone:option(Value, "message", translate("Date Format"))


return cbimap
