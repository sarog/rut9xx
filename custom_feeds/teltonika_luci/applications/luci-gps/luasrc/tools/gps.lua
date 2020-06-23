--[[
LuCI - Lua Configuration Interface

Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: firewall.lua 8314 2012-02-19 20:22:17Z jow $
]]--

module("luci.tools.gps", package.seeall)

local ut = require "luci.util"
local ip = require "luci.ip"
local nx = require "nixio"

local translate, translatef = luci.i18n.translate, luci.i18n.translatef

local function tr(...)
	return tostring(translate(...))
end

function opt_enabled(s, t, ...)
	if t == luci.cbi.Button then
		local o = s:option(t, "__enabled")
		function o.render(self, section)
			if self.map:get(section, "enabled") == "1" then
				self.title      = tr("Rule is enabled")
				self.inputtitle = tr("Disable")
				self.inputstyle = "reset"
			else
				self.title      = tr("Rule is disabled")
				self.inputtitle = tr("Enable")
				self.inputstyle = "apply"
			end
			t.render(self, section)
		end
		function o.write(self, section, value)
			if self.map:get(section, "enabled") == "1" then
				self.map:set(section, "enabled", "0")
			else
				self.map:set(section, "enabled", "1")
			end
		end
		return o
	else
		local o = s:option(t, "enabled", ...)
		      o.rmempty = false
		      o.default = o.enabled
		return o
	end
end
