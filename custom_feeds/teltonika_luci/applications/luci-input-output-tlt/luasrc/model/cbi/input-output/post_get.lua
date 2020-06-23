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

m = Map("output_control", translate("Post/Get Configuration"))

s = m:section(NamedSection, "post_get", "post_get", translate("Output Post/Get Settings"))
s.anonymous = true
s.addremove = false

o = s:option(Flag, "enabled", translate("Enable"), translate("To enable post/get output functionality"))

user = s:option(Value, "username", translate("Username"), translate("Specify username for authentication (5 characters minimum). Allowed characters (^[a-zA-Z0-9!@#$%%^&*+=?_.-]+$ )"))
user.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%^&*+=?_.-]+$',5)"
user.default = "user1"

pass = s:option(Value, "password", translate("Password"), translate("Specify password of a username. Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:;[]"))
pass.datatype = "password"
pass.password = true

return m
