
--[[
LuCI - Lua Configuration Interface

Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: custom.lua 8108 2011-12-19 21:16:31Z jow $
]]--

local fs = require "nixio.fs"
local http = require("luci.http")
local m = SimpleForm("firewall",
	translate("Firewall - Custom Rules"),
	translate("Custom rules allow you to execute arbritary iptables commands which are not otherwise covered by the firewall framework. The commands are executed after each firewall restart, right after the default ruleset has been loaded."))

local o = m:field(Value, "_custom")
o.template = "cbi/tvalue"
o.rows = 20

function o.cfgvalue(self, section)
	return fs.readfile("/etc/firewall.user")
end

--[[function o.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	fs.writefile("/etc/firewall.user", value)
end]]--

--[[function o.parse(self, section)
	local cbeid = "cbid." .. self.config .. "." .. section .. "." .. self.option
	local value = http.formvalue(cbeid)
	value = value:gsub("\r\n?", "\n")
	fs.writefile("/etc/firewall.user", value)
end]]--

function m.handle(self, state, data)
	if state == FORM_VALID then
		local value2=m:formvalue("cbid.firewall.1._custom")
		if value2~=nil and value2~="" then
			local value = value2:gsub("\r\n?", "\n")
			fs.writefile("/etc/firewall.user", value)
		else
			luci.sys.call("echo '' >/etc/firewall.user")
		end
		luci.util.exec("/etc/init.d/firewall restart")
	end
end

return m
