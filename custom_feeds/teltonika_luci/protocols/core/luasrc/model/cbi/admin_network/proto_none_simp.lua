--[[
LuCI - Lua Configuration Interface

Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0
]]--
local ut = require "luci.util"
local sys = require "luci.sys"
local map, section, interface, empty = ...
local ifc = interface
local ifname = (interface and interface:name()) or ""
local hostname, accept_ra, send_rs
local bcast, no_gw, no_dns, dns, metric, clientid, vendorclass, macaddr, mtu
local moduleproto = ut.trim(sys.exec("uci get -q network.ppp.proto"))

-- if luci.model.network:has_ipv6() then
-- 	local value=ut.trim(sys.exec("uci get -q system.ipv6.enable"))
-- 	if tonumber(value)==1 then
-- 		--accept_ra = s:taboption("general", Flag, "accept_ra", translate("Accept router advertisements"))
-- 		accept_ra = section:taboption("general", Flag, "accept_ra", translate("Accept router advertisements"), translate("Enable to let accept router advertisements to locate router and learn parameters for operating local network"))
-- 		accept_ra.default = accept_ra.enabled
-- 		--send_rs = s:taboption("general", Flag, "send_rs", translate("Send router solicitations"))
-- 		send_rs = section:taboption("general", Flag, "send_rs", translate("Send router solicitations"), translate("Enable to send router solicitations as response"))
-- 		send_rs.default = send_rs.disabled
-- 		send_rs:depends("accept_ra", "")
-- 	end
-- end

bcast = section:taboption("advanced", Flag, "broadcast", translate("Use broadcast flag"), translate("Required for certain ISPs, e.g. Charter with DOCSIS 3"))
bcast.default = bcast.disabled

no_gw = section:taboption("advanced", Flag, "gateway", translate("Use default gateway"), translate("If unchecked, no default route is configured"))
no_gw.default = no_gw.enabled

function no_gw.cfgvalue(...)
	return Flag.cfgvalue(...) == "0.0.0.0" and "0" or "1"
end

function no_gw.write(self, section, value)
	if value == "1" then
		map:set(section, "gateway", nil)
	else
		map:set(section, "gateway", "0.0.0.0")
	end
end

no_dns = section:taboption("advanced", Flag, "no_dns", translate("Use DNS servers advertised by peer"), translate("If unchecked, the advertised DNS (Domain Name System) server addresses are ignored"))
no_dns.default = no_dns.enabled

function no_dns.write(self, section, value)
	if value == "0" then
		map:set(section, "no_dns", value)
	else
		map:del(section, "no_dns")
	end
end

dns = section:taboption("advanced", DynamicList, "dns", translate("Use custom DNS servers"), translate("By entering custom DNS (Domain Name System) server the router will take care of host name resolution. You can enter multiple DNS servers"))
dns:depends("no_dns", "")
dns.datatype = "ipaddr"
dns.cast = "string"

metric = section:taboption("advanced", Value, "metric", translate("Use gateway metric"), translate("The WAN (Wide Area Network) configuration by default generates a routing table entry. With this field you can alter the metric of that entry"))
metric.placeholder = "0"
metric.datatype    = "uinteger"
metric:depends("gateway", "1")

-- clientid = section:taboption("advanced", Value, "clientid", translate("Client ID to send when requesting DHCP"), translate("Specify client ID which will be sent when requesting DHCP (Dynamic Host Configuration Protocol)"))

-- vendorclass = section:taboption("advanced", Value, "vendorid", translate("Vendor class to send when requesting DHCP"), translate("Specify vendor class which will be sent when requesting DHCP (Dynamic Host Configuration Protocol)"))

if moduleproto == "ndis" and ifname == "eth2" then
	--skip macoverride field
else
	macaddr = section:taboption("advanced", Value, "macaddr", translate("Override MAC address"), translate("Override MAC (Media Access Control) address of the WAN interface (Wide Area Network)"))
	macaddr.placeholder = ifc and ifc:mac() or "00:00:00:00:00:00"
	macaddr.datatype    = "macaddr"
end

mtu = section:taboption("advanced", Value, "mtu", translate("Override MTU"), translate("MTU (Maximum Transmission Unit) specifies the largest possible size of a data packet"))
mtu.placeholder = "1500"
mtu.datatype    = "max(1500)"

if empty then
	function bcast.cfgvalue(self, section)
		return self.default
	end
	if moduleproto ~= "qmi" and ifname ~= "wwan0" then
		function macaddr.cfgvalue(self, section)
			return ""
		end
	end
end
