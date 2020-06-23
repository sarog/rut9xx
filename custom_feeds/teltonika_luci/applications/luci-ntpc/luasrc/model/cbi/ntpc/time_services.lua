--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: ntpc.lua 6065 2010-04-14 11:36:13Z ben $
]]--
require("luci.sys")
require("luci.sys.zoneinfo")
require("luci.tools.webadmin")
require("luci.fs")
require("luci.config")

local port

local function cecho(string)
	luci.sys.call("echo \"" .. string .. "\" >> /tmp/log.log")
end

m = Map("ntpclient", translate("Time Synchronisation"), translate(""))

----- Time Servers


s3 = m:section(TypedSection, "ntpserver", translate("Time Servers"))
s3.anonymous = true
s3.addremove = true
s3.template = "cbi/tblsection"

s3:option(Value, "hostname", translate("Hostname"), translate("NTP (Network Time Protocol) server\\'s hostname"))

-- port = s3:option(Value, "port", translate("Port"), translate("NTP (Network Time Protocol) server\\'s port number"))
-- port.rmempty = false
-- port.datatype = "port"
-- port.default = "123"

function m.on_after_commit(self)
	luci.sys.call("export ACTION=ifdown; sh /etc/hotplug.d/iface/20-ntpclient")
	luci.sys.call("export ACTION=; sh /etc/hotplug.d/iface/20-ntpclient &")
end

return m
