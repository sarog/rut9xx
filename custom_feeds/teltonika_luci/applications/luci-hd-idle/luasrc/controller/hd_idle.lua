--[[

LuCI hd-idle
(c) 2008 Yanira <forum-2008@email.de>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

$Id: hd_idle.lua 7362 2011-08-12 13:16:27Z jow $

]]--

module("luci.controller.hd_idle", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/hd-idle") then
		return
	end

	local page

	page = entry({"admin", "services", "hd_idle"}, cbi("hd_idle"), _("hd-idle"), 60)
	page.i18n = "hd_idle"
	page.dependent = true
end
