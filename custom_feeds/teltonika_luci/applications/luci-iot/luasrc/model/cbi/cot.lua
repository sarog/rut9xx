--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0
]]--

require "teltonika_lua_functions"
local sys = require "luci.sys"

m = Map("cot", translate("Cloud of Things"))

s = m:section(NamedSection, "cumulocity", "iot", translate("Configuration"))
s.addremove = false

enabled = s:option(Flag, "enabled", translate("Enable"), 
	translate("Enable Cloud of Things Application"))
enabled.default= "0"
enabled.rmempty = false

server = s:option(Value, "server", translate("Server Address"), 
	translate("Cloud of Things Server Address"))
server.datatype = "string_not_empty"
server.rmempty = false

reset = s:option(Button, "reset_auth", translate("Reset Auth"), 
	translate("Reset authentication data so that device could be "..
			"re-registered on Cloud of Things Device Management"))

function reset.write()
	m.message = "Successfully cleared authentication data. "..
	"Now you can re-register device on Cloud of Things Device Management."
	m.uci:delete("cot", "cumulocity", "tenant")
	m.uci:delete("cot", "cumulocity", "username")
	m.uci.delete("cot", "cumulocity", "password")
	m.uci:commit("cot")
	sys.exec("rm /tmp/cotauth")
	sys.exec("/etc/init.d/cot restart")
end

return m
