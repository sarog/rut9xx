--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: ntpc.lua 7362 2011-08-12 13:16:27Z jow $
]]--

module("luci.controller.ntpc", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/ntpclient") then
		return
	end
	
	local page
	entry({"admin", "services", "ntpc"}, alias("admin", "services", "ntpc", "general"), _("NTP"), 50)

	entry({"admin", "services", "ntpc","general"}, cbi("ntpc/ntpc"), _("General"), 1).leaf = true
	entry({"admin", "services", "ntpc","time_servers"}, cbi("ntpc/time_services"), _("Time Servers"), 2).leaf = true

	
	page = entry({"mini", "services", "ntpc"}, cbi("ntpc/ntpcmini", {autoapply=true}), _("NTP"), 50)
	page.i18n = "ntpc"
	page.dependent = true
end
