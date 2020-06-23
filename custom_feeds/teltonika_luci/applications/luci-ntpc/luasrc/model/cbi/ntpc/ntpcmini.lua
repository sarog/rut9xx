--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: ntpcmini.lua 6065 2010-04-14 11:36:13Z ben $
]]--
require("luci.tools.webadmin")
m = Map("ntpclient", translate("Time Synchronisation"), translate("Synchronizes the system time"))

s = m:section(TypedSection, "ntpclient", translate("General"))
s.anonymous = true
s.addremove = false

s:option(DummyValue, "_time", translate("Current system time")).value = os.date("%c")

s:option(Value, "interval", translate("Update interval (in seconds)")).rmempty = true


s3 = m:section(TypedSection, "ntpserver", translate("Time Server"))
s3.anonymous = true
s3.addremove = true
s3.template = "cbi/tblsection"

s3:option(Value, "hostname", translate("Hostname"))
s3:option(Value, "port", translate("Port")).rmempty = true

return m
