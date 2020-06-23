--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008-2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: system.lua 8122 2011-12-20 17:35:50Z jow $
]]--

module("luci.controller.admin.user_scripts", package.seeall)

function index()
	entry({"admin", "system", "startup"}, form("admin_system/startup"), _("User Scripts"), 4)
end