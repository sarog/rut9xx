--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

$Id: profiles.lua 7362 2011-08-12 13:16:27Z jow $
]]--

module("luci.controller.webfilter", package.seeall)

function index()
	entry({"admin", "services", "webfilter"}, alias("admin", "services", "webfilter", "site"), _("Web Filter"), 10)
	entry({"admin", "services", "webfilter", "site"}, cbi("webfilter/web_filter"), _("Site Blocking"), 1).leaf=true
	entry({"admin", "services", "webfilter", "proxy"}, cbi("webfilter/web_filter_proxy"), _("Proxy Based Content Blocker"), 2).leaf=true
end
