--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: coovachilli.lua 3442 2008-09-25 10:12:21Z jow $
]]--

local function cecho(string)
	luci.sys.call("echo \"" .. string .. "\" >> /tmp/log.log")
end

local m, s, o

local utl = require "luci.util"
local sys = require "luci.sys"
local uci  = cursor or _uci_real or uci.cursor()
require("uci")
require("teltonika_lua_functions")
local x = uci.cursor()
local listofssids = {}
local list_length = 0


local url = uci:set("coovachilli", "url", "url")
uci:commit("coovachilli")

x:foreach("wireless", "wifi-iface", function(s)
		table.insert(listofssids, escapeHTML(s.ssid))
		list_length = list_length +1
end)

m = Map( "coovachilli", translate( "Wireless Hotspot URL parameters settings" ), translate( "" ) )

lan_ip = m.uci:get("network", "lan", "ipaddr")
hostname = m.uci:get("system", "system", "hostname")
fw_version = luci.util.exec("cat /etc/version") or "Unknown"

s = m:section( NamedSection, "url", "url", translate("Login URL parameters"), translate("Here you can set custom Captive Portal URL identification names."))

o = s:option( Value, "uamip", translate("UAM IP" ), translate("The IP Address of the Captive Portal gateway"))
	o.placeholder = "uamip"

o = s:option( Value, "uamport", translate("UAM port" ), translate("The port on which the Captive Portal will serve web content"))
	o.placeholder = "uamport"

o = s:option( Value, "called", translate("Called" ), translate("The MAC address of the IP Address of the Captive Portal gateway"))
	o.placeholder = "called"

o = s:option( Value, "mac", translate("MAC" ), translate("The MAC address of the client trying to gain Internet access"))
	o.placeholder = "mac"

o = s:option( Value, "ip", translate("IP" ), translate("The IP Address of the client trying to gain Internet access"))
	o.placeholder = "ip"

o = s:option( Value, "nasid", translate("NAS id" ), translate("An identification for the Captive Portal used in the RADIUS request"))
	o.placeholder = "nasid"

o = s:option( Value, "sessionid", translate("Session id" ), translate("The unique identifer for session"))
	o.placeholder = "sessionid"

o = s:option( Value, "userurl", translate("User url" ), translate("The URL which the user tried to access before he were redirected to the Captive Portal\\'s URL\\'s pages"))
	o.placeholder = "userurl"

o = s:option( Value, "challenge", translate("Challenge" ), translate("A challenge that should be used together with the user\\'s password to create an encrypted phrase used to log on"))
	o.placeholder = "challenge"

o = s:option( Value, "custom_name_nr1", translate("Custom No.1"), translate("Add custom name and custom value which will be displayed in url parameters"))
o.displayInline = true
o.placeholder = "Custom name"

o = s:option( Value, "custom_value_nr1", translate(""), translate(""))
o.displayInline = true
for i=list_length,1,-1
do
   o:value(listofssids[i], translate("SSID: "..listofssids[i]))
end
o:value(lan_ip, translate("LAN IP: "..lan_ip))
o:value(hostname, translate("Hostname: "..hostname))
o:value(fw_version, translate("FW versoin: "..fw_version))
o.default = listofssids[1]


o = s:option( Value, "custom_name_nr2", translate("Custom No.2"), translate("Add custom name and custom value which will be displayed in url parameters"))
o.displayInline = true
o.placeholder = "Custom name"

o = s:option( Value, "custom_value_nr2", translate(""), translate(""))
o.displayInline = true
for i=list_length,1,-1
do
   o:value(listofssids[i], translate("SSID: "..listofssids[i]))
end
o:value(lan_ip, translate("LAN IP: "..lan_ip))
o:value(hostname, translate("Hostname: "..hostname))
o:value(fw_version, translate("FW versoin: "..fw_version))
o.default = listofssids[1]

return m
