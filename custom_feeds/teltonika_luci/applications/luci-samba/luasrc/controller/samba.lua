--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: samba.lua 7362 2011-08-12 13:16:27Z jow $
]]--

module("luci.controller.samba", package.seeall)

--local inspect = require 'inspect'

function index()
	local sys = require "luci.sys"
	local utl = require "luci.util"
	local usb = utl.trim(luci.sys.exec("uci get hwinfo.hwinfo.usb"))
	local microsd = utl.trim(luci.sys.exec("uci get hwinfo.hwinfo.microsd"))
	if usb == "1" or microsd == "1" then
                entry({"admin", "services", "usb-tools","network_shares"},  alias("admin", "services", "usb-tools", "network_shares", "filesystem"), _("Network Shares"), 1)
                entry({"admin", "services", "usb-tools", "usb_to_serial"}, cbi("usb_serial"), _("USB to serial"), 3)
                    entry({"admin", "services", "usb-tools", "network_shares", "filesystem"}, cbi("filesystem"), _("Mounted file systems"), 1).leaf=true
--                     entry({"admin", "services", "samba", "network_shares", "edit"}, call("safe_remove"), nil).leaf = true
                    entry({"admin", "services", "usb-tools", "network_shares", "samba"}, cbi("samba"), _(translate("Samba")), 2).leaf=true
                    entry({"admin", "services", "usb-tools", "network_shares", "user"}, cbi("samba_user"), _(translate("Samba user")), 3).leaf=true
                    entry({"admin", "services", "usb-tools", "network_shares", "edit"}, call("safe_remove"), nil).leaf = true
	--else
		--entry({"admin", "services", "samba"}, alias("admin", "services", "samba", "samba"), _(translate("Network Shares"), 99))
	end	
	--entry({"admin", "services", "samba", "edit"}, call("safe_remove"), nil).leaf = true
	--entry({"admin", "services", "samba", "samba"}, cbi("samba"), _(translate("Samba")), 2).leaf=true
	--entry({"admin", "services", "samba", "user"}, cbi("samba_user"), _(translate("Samba user")), 3).leaf=true
end

function safe_remove(id)
	local mounts = luci.sys.mounts()
	local v = mounts[tonumber(id)]["fs"]
	luci.sys.call("/etc/init.d/samba stop; umount -l " .. v .. "; /etc/init.d/samba start")
	luci.http.redirect(luci.dispatcher.build_url("admin/services/usb-tools/network_shares"))
end
