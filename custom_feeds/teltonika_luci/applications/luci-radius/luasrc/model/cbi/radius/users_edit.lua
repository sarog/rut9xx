--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: coovachilli.lua 3442 2008-09-25 10:12:21Z jow $
]]--
local ds = require "luci.dispatcher"
local function debug(string)
	luci.sys.call("logger -s \"" .. string .. "\"")
end
arg[1] = arg[1] or ""

debug("user edit page")

m = Map( "radius",	translate( "Radius Server Configuration" ), translate( "" ))
	m.redirect = ds.build_url("admin/services/hotspot/radius/")

users = m:section(NamedSection, arg[1], "user", translate( "Users Configuration Settings"))

enb_user = users:option(Flag, "enabled", translate("Enable user"), translate("" ))
	enb_user.enabled = "1"
	enb_user.rmempty = false

name = users:option(Value, "username", translate("User name"), translate("" ))
	name.rmempty = false
	name.datatype = "fieldvalidation('^[a-zA-Z0-9_]+$',0)"

pass = users:option(Value, "pass", translate("User password"), translate("" ))
	pass.password = true
	pass.rmempty = false
	pass.datatype = "fieldvalidation('^[^ ]+$',0)"

message = users:option(Value, "message", translate("Reply message"), translate("" ))

return m
