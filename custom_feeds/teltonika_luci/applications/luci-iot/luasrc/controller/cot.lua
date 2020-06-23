--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0
]]--
module("luci.controller.cot", package.seeall)

function index()
	local fs = require "nixio.fs"
	entry({"admin", "services", "iot", "cot"},
		cbi("cot"), _("Cloud of Things"), 2).leaf = true

	if not fs.access("/usr/lib/opkg/info/tlt_custom_pkg_azure_iothub.control") and not fs.access("/usr/lib/opkg/info/tlt_custom_pkg_cmstreamapp.control") and not fs.access("/usr/lib/opkg/info/tlt_custom_pkg_twstreamapp.control") then
		entry({"admin", "services", "iot"},
			alias("admin", "services", "iot", "cot"), _("IoT Platforms"), 50)
	end
end
