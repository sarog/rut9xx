--[[
LuCI - Lua Configuration Interface

Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: firewall.lua 8314 2012-02-19 20:22:17Z jow $
]]--

module("luci.tools.events_reporting", package.seeall)

local ut = require "luci.util"
local ip = require "luci.ip"
local nx = require "nixio"

local translate, translatef = luci.i18n.translate, luci.i18n.translatef

local function tr(...)
	return tostring(translate(...))
end

function fmt_neg(x)
	if type(x) == "string" then
		local v, neg = x:gsub("^ *! *", "")
		if neg > 0 then
			return v, "%s " % tr("not")
		else
			return x, ""
		end
	end
	return x, ""
end

function fmt_icmp_type(x)
	if x and #x > 0 then
		local t, v, n
		local l = { tr("type"), " " }
		for v in ut.imatch(x) do
			v, n = fmt_neg(v)
			l[#l+1] = "<var>%s%s</var>" %{ n, v }
			l[#l+1] = ", "
		end
		if #l > 1 then
			l[#l] = nil
			if #l > 3 then
				l[1] = tr("types")
			end
			return table.concat(l, "")
		end
	end
end

function opt_enabled(s, t, ...)
	if t == luci.cbi.Button then
		local o = s:option(t, "__enabled")
		function o.render(self, section)
			if self.map:get(section, "enable") == "1" then
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
			if self.map:get(section, "enable") == "1" then
				self.map:set(section, "enable", "0")
			else
				self.map:set(section, "enable", "1")
			end
		end
		return o
	else
		local o = s:option(t, "enable", ...)
		      o.rmempty = false
		      o.default = o.enabled
		return o
	end
end
