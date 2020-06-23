--[[
LuCI - Lua Configuration Interface

Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0
]]--

local map, section, net = ...

local server, username, password
local buffering, defaultroute, metric, peerdns, dns


server = section:taboption("general", Value, "server", translate("VPN Server"))
server.datatype = "host"


username = section:taboption("general", Value, "username", translate("PAP/CHAP username"))


password = section:taboption("general", Value, "password", translate("PAP/CHAP password"))
password.password = true


buffering = section:taboption("advanced", Flag, "buffering", translate("Enable buffering"))
buffering.default = buffering.enabled


defaultroute = section:taboption("advanced", Flag, "defaultroute",
	translate("Use default gateway"),
	translate("If unchecked, no default route is configured"))

defaultroute.default = defaultroute.enabled


metric = section:taboption("advanced", Value, "metric",
	translate("Use gateway metric"),
	translate("The WAN configuration by default generates a routing table entry. With this field you can alter the metric of that entry"))

metric.placeholder = "0"
metric.datatype    = "uinteger"
metric:depends("defaultroute", defaultroute.enabled)


peerdns = section:taboption("advanced", Flag, "peerdns",
	translate("Use DNS servers advertised by peer"),
	translate("If unchecked, the advertised DNS server addresses are ignored"))

peerdns.default = peerdns.enabled


dns = section:taboption("advanced", DynamicList, "dns",
	translate("Use custom DNS servers"),
	translate("By entering custom DNS server the router will take care of host name resolution. You can enter multiple DNS server"))

dns:depends("peerdns", "")
dns.datatype = "ipaddr"
dns.cast     = "string"
