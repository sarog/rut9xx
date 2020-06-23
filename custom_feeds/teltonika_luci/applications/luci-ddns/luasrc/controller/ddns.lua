--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: ddns.lua 7362 2011-08-12 13:16:27Z jow $
]]--

module("luci.controller.ddns", package.seeall)

luci_helper = "/usr/lib/ddns/dynamic_dns_lucihelper.sh"

function index()
	if not nixio.fs.access("/etc/config/ddns") then
		return
	end

	entry({"admin", "services", "ddns"}, cbi("ddns/ddns_first"), _("Dynamic DNS"), 60)
	entry({"admin", "services", "ddns_edit"}, cbi("ddns/ddns"), nil).leaf = true

end
