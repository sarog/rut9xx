--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0
]]--

local m, s, enabled, server, port, thing, appkey

m = Map("iottw", translate("ThingWorx"))

s = m:section(NamedSection, "thingworx", "iottw", translate("Configuration"))
	s.addremove = false

enabled = s:option(Flag, "enabled", translate("Enable"), translate("Enable ThingWorx Application"))
	enabled.default = "0"

server = s:option(Value, "server", translate("Server Address"), translate("ThingWorx Server IP"))

port = s:option(Value, "port", translate("Server Port"), translate("ThingWorx Server Port"))
	port.datatype = "port"

thing = s:option(Value, "thing", translate("Thing Name"), translate("Thing name defined in ThingWorx CP"))

	function thing.validate(self, value)
		if value:match("%W") then
			return nil, "Thing Name: \'" .. (value and value or "") .. "\' is invalid. Only alphanumeric"
				.. " characters are allowed."
		end
		return value
	end

appkey = s:option(Value, "appkey", translate("Application Key"), translate("Application key generated in ThingWorx CP"))

	function appkey.validate(self, value)
		if value:gsub("-", ""):match("%W") then
			return nil, "Application Key: \'" .. (value and value or "") .. "\' is invalid. Only alphanumeric"
				.. " characters and '-' symbol are allowed."
		end
		return value
	end

return m
