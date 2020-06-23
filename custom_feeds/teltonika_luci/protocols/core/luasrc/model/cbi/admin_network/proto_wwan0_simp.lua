--[[
LuCI - Lua Configuration Interface

Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0
]]--
local ut = require "luci.util"
local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()
local map, section, interface, empty = ...

no_dns = section:taboption("advanced", Flag, "_no_dns", translate("Use DNS servers advertised by peer"), translate("If unchecked, the advertised DNS (Domain Name System) server addresses are ignored"))
no_dns.default = no_dns.enabled

function no_dns.cfgvalue(self, section)
	local addr
	for addr in luci.util.imatch(uci:get("network", "ppp", "dns")) do
		return self.disabled
	end
	return self.enabled
end

function no_dns.remove(self, section)
		uci:delete("network", "ppp", "dns")
		uci:commit("network")
	return map:del(section, "dns")
end

function no_dns.write() end

dns = section:taboption("advanced", DynamicList, "dns", translate("Use custom DNS servers"), translate("By entering custom DNS (Domain Name System) server the router will take care of host name resolution. You can enter multiple DNS server"))
dns:depends("_no_dns", "")
dns.datatype = "ipaddr"
dns.cast     = "string"

function dns.cfgvalue(self, section)
	return uci:get("network", "ppp", "dns")
end

function dns.write(self, section, value)
	newvalue = {}
	nindex = 1
	for index, field in ipairs(value) do
		val = luci.util.trim(value[index])
		newvalue[nindex] = val
		nindex = nindex + 1
	end
	uci:set("network", "ppp", "dns", newvalue)
	uci:commit("network")
end
