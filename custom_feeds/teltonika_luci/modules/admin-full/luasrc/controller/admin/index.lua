--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: index.lua 7789 2011-10-26 03:04:18Z jow $
]]--

module("luci.controller.admin.index", package.seeall)

function index()
	local root = node()
	if not root.target then
		root.target = alias("admin")
		root.index = true
	end

	local page   = node("admin")

	x = uci.cursor()
	firstLogin = x:get("teltonika", "sys", "first_login")

	if firstLogin == "1" then
		page.target = call("redirect_to_pwd")
	else
		page.target  = call("redirect_to_overview")
	end

	--page.target  = firstchild()
	page.title   = _("Administration")
	page.order   = 10
	page.sysauth = "root"
	page.sysauth_authenticator = "htmlauth"
	page.ucidata = true
	page.index = true

	-- Empty services menu to be populated by addons
	entry({"admin", "services"}, call("redirect_to_overview"), _("Services"), 40).index = true

	entry({"admin", "logout"}, call("action_logout"), _("Logout"), 90)
end

function redirect_to_pwd()
	luci.http.redirect(luci.dispatcher.build_url("admin", "system", "wizard", "step-pwd"))
end

function redirect_to_overview()
	luci.http.redirect(luci.dispatcher.build_url("admin", "status", "overview"))
end

function action_logout()
	local dsp = require "luci.dispatcher"
	local sauth = require "luci.sauth"
	if dsp.context.authsession then
		sauth.kill(dsp.context.authsession)
		dsp.context.urltoken.stok = nil
	end

	luci.http.header("Set-Cookie", "sysauth=; path=" .. dsp.build_url())
	luci.http.redirect(luci.dispatcher.build_url())
end
