--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: samba.lua 6984 2011-04-13 15:14:42Z soma $
]]--
require("luci.tools.webadmin")
local sys = require "luci.sys"
local utl = require "luci.util"

m = Map("samba", translate("Network Shares"))
--m2 = Map("fstab", translate("Mount Points"))
m.pageaction = true
m.refreshpage = "Refresh"

local mounts = luci.sys.mounts()

v = m:section(Table, mounts, translate("Mounted file systems"))

v.template = "cbi/tblsection"
--v.addremove = true
--vl = mounts[section].mountpoint

v.addbutton = luci.dispatcher.build_url("admin", "services", "usb-tools", "network_shares", "edit", "%s")
v.addbuttontittle = "Safely Remove Disk"


fs = v:option(DummyValue, "fs", translate("Filesystem"))

mp = v:option(DummyValue, "mountpoint", translate("Mount Point"))

avail = v:option(DummyValue, "avail", translate("Available"))
--local vl 
function avail.cfgvalue(self, section)
	return luci.tools.webadmin.byte_format(	( tonumber(mounts[section].available) or 0 ) * 1024
	) .. " / " .. luci.tools.webadmin.byte_format(
		( tonumber(mounts[section].blocks) or 0 ) * 1024
	)
end

used = v:option(DummyValue, "used", translate("Used"))
function used.cfgvalue(self, section)
       vl = mounts[section].mountpoint
      
	return ( mounts[section].percent or "0%" ) .. " (" ..
	luci.tools.webadmin.byte_format(
		( tonumber(mounts[section].used) or 0 ) * 1024
	) .. ")"
end



--luci.sys.call("echo \"vpn: " .. vl .. "\" >> /tmp/test.txt")
  --local pptpdEnable = m:formvalue("cbid.samba." .. mounts .. ".fs")
--conLog = v:option(Button, "_save" )
-- function conLog.cfgvalue(self, section)
-- 	return mounts[section].fs 
-- 	
-- end

-- conLog.title      = translate("Save remove")
-- conLog.inputstyle = "apply"
-- function conLog.cfgvalue(self, section)
-- 	return mounts[section].mountpoint
-- end
-- 
-- -----------------------Troubleshoot download-----------------
-- if m:formvalue("cbid.table.1._save") then
--   
-- 	local role = luci.http.formvalue("cbid.table." .. self.sectiontype .. ".mountpoint")
-- 	  
-- 	luci.sys.call("echo \"vpn: " .. vl .. "\" >> /tmp/test.txt")
-- 	luci.http.redirect(luci.dispatcher.build_url("admin/services/samba"))
-- end



return m
