--[[
LuCI - Lua Configuration Interface

Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: forward-details.lua 8117 2011-12-20 03:14:54Z jow $
]]--

local m, s, o

m = Map("sms_gateway", translate("POP3 Email To SMS Configuration"))
m.email_to_sms = true

s = m:section(NamedSection, "pop3", "pop3", translate("Email To SMS Settings"))
s.anonymous = true
s.addremove = false

o = s:option(Flag, "enabled", translate("Enable"), translate("Enable"))

host = s:option(Value, "host", translate("POP3 server"), translate("POP3 server"))

port = s:option(Value, "port", translate("Server port"), translate("Server port"))

user = s:option(Value, "username", translate("User name"), translate("User name used for authorization"))
user.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',5)"

pass = s:option(Value, "password", translate("Password"), translate("Password used for authorization (5 characters minimum). Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:; []"))
pass.datatype = "and(password(1), string_length(5))"
pass.password = true

o = s:option(Flag, "ssl", translate("Secure connection (SSL)"), translate("Secure connection (SSL)"))

limit = s:option(Value, "limit", translate("Max. email symbol count"), translate("Maximum number of allowed symbols contained in email"))
limit.datatype = "min(1)"
limit.placeholder = '160'

o = s:option(ListValue, "min", translate("Check email every"), translate(""))
	o:depends({time = "min"})
	o.displayInline = true
	a={1,2,5,10,15,20,30}
	for i=1,7 do
		o:value(a[i], a[i])
	end

o = s:option(ListValue, "hour", translate("Check email every"), translate(""))
	o.displayInline = true
	o:depends({time = "hour"})
	a={1,2,4,6,8,12}
	for i=1,6 do
		o:value(a[i], a[i])
	end

o = s:option(ListValue, "day", translate("Check email every"), translate(""))
	o.displayInline = true
	o:depends({time = "day"})
	a={1,2,3,5,10,15}
	for i=1,6 do
		o:value(a[i], a[i])
	end

o = s:option(ListValue, "time", translate(""), translate(""))
	o.displayInline = true
	o:value("min", translate("Minutes"))
	o:value("hour", translate("Hours"))
	o:value("day", translate("Days"))

return m
