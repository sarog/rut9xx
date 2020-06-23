--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: routes.lua 6447 2010-11-16 19:06:51Z jow $
]]--
require("luci.tools.webadmin")

local interfacesa = {}
interfacesa["3g-ppp"]="Mobile"
interfacesa["eth2"]="Mobile"
interfacesa["eth1"]="Wired"
interfacesa["wlan0"]="WiFi"
interfacesa["none"]="Mobile bridged"
interfacesa["wwan0"]="Mobile"
interfacesa["usb0"]="WiMAX"
interfacesa["wm0"]="WiMAX"

m = Map("network",
	translate("Static Routes"),
	translate("Routes specify over which interface and gateway a certain host or network can be reached."))

m:chain("gre_tunnel")
m:chain("openvpn")
local sys = require "luci.sys"
local util = require "luci.util"
local routes6 = luci.sys.net.routes6()
local ipv6_enable=util.trim(sys.exec("uci get -q system.ipv6.enable"))
local bit = require "bit"
local uci = require "luci.model.uci".cursor()

m.uci:foreach("network", "route", function(x)
	configName = x[".name"] or ""
	if luci.http.formvalue("cbi.rts.network.".. configName) then
		interface = x["interface"] or ""
		target = x["target"] or ""
		netmask = x["netmask"] or ""
		gateway = x["gateway"] or ""
		metric = x["metric"] or "0"
		os.execute("route del -net " .. target .. " netmask " .. netmask .. " gw " .. gateway .. " metric " ..metric .. " dev " .. interface .. " 2>/dev/null")
	end
end)

if luci.http.formvalue("cbi.apply") then
        local delButtonFormString = "cbi.rts.network."
        local configName
        local changes = false
        m.uci:foreach("network", "route", function(x)
                configName = x[".name"] or ""
                local interface = luci.util.trim(luci.sys.exec("uci get -q network." .. configName .. ".interface")) or ""
                local target = luci.util.trim(luci.sys.exec("uci get -q network." .. configName .. ".target")) or ""
                local netmask = luci.util.trim(luci.sys.exec("uci get -q network." .. configName .. ".netmask")) or ""
                local gateway = luci.util.trim(luci.sys.exec("uci get -q network." .. configName .. ".gateway")) or ""
                local metric = luci.util.trim(luci.sys.exec("uci get -q network." .. configName .. ".metric")) or ""
                if interface ~= luci.http.formvalue("cbid.network." .. configName .. ".interface") then
                	changes = true
                end
		if target ~= luci.http.formvalue("cbid.network." .. configName .. ".target") then
			changes = true
		end
		if netmask ~= luci.http.formvalue("cbid.network." .. configName .. ".netmask") then
			changes = true
		end
                if gateway ~= luci.http.formvalue("cbid.network." .. configName .. ".gateway") then
			changes = true
		end
		if metric ~= luci.http.formvalue("cbid.network." .. configName .. ".metric") then
                	changes = true
		end
                if changes == true then
                        --delete static routes
                        if netmask ~= "" then
                                netmask = " netmask " .. netmask
                        end
                        if gateway ~= "" then
                                gateway = " gateway " .. gateway
                        end
                        if metric ~= "" then
                                metric = " metric " .. metric
                        end
                        os.execute("route del -net " .. target .. netmask .. gateway .. metric .. " dev " .. interface .. " 2>/dev/null")
                end
        end)

end


s = m:section(TypedSection, "route", translate("Static IP Routes"))
s.addremove = true
s.anonymous = true

s.template  = "cbi/tblsection"
s.novaluetext = translate("There are no static IP routes yet")

table = s:option(ListValue, "table", translate("Routing table"), translate("Defines the table to use for the route"))
	table:value("main", string.upper("main"))

	m.uci:foreach("network", "interface",
		function (section)
			if section[".name"]:match("wan") == "wan" then
				table:value(section[".name"], section[".name"]:upper())
			end
		end
	)

iface = s:option(ListValue, "interface", translate("Interface"), translate("The zone where target network resides"))
--luci.tools.webadmin.cbi_add_networks(iface, "routes")


uci:foreach("network", "interface", function(a)
	if a[".name"] ~= "loopback" and a[".name"] ~= "ppp" then
		name=interfacesa[a.ifname]
		if name then
			type=a[".name"]:upper()
			all=type.." ("..name..")"
		elseif a.proto == "l2tp" then
			type=a[".name"]
			all="l2tp_"..type
		elseif a.proto == "pptp" then
			type=a[".name"]
			all="pptp_"..type
		else
			type=a[".name"]:upper()
			all=type
		end
		iface:value(a[".name"], all)
	end
end)

uci:foreach("xl2tpd", "service", function(a)
	local name = m.uci:get("xl2tpd", "xl2tpd", "_name") or "xl2tpd"
	iface:value("xl2tpd-server", "xl2tpd_"..name)
end)

uci:foreach("gre_tunnel", "gre_tunnel", function(a)
	iface:value(a.ifname)
end)

uci:foreach("pptpd", "service", function(a)
	local name = m.uci:get("pptpd", "pptpd", "_name") or "pptp"
	iface:value("pptp-server", "pptp_"..name)
end)

uci:foreach("openvpn", "openvpn", function(b)
	if b.dev ~= "tun_rms" then
		iface:value(b.dev)
	end
end)

function s.filter(self, section)
	local ifname = self.map:get(section, "interface")
	local interface_found = 0
	if not ifname then
		interface_found = 1
	else
		uci:foreach("network", "interface", function(s)
			if s[".name"] == ifname then
				interface_found = 1
				return
			end
		end)
		uci:foreach("xl2tpd", "service", function(a)
			if "xl2tpd-server" == ifname then
				interface_found = 1
				return
			end
		end)
		uci:foreach("gre_tunnel", "gre_tunnel", function(a)
			if a.ifname == ifname then
				interface_found = 1
				return
			end
		end)
		uci:foreach("pptpd", "service", function(a)
			if "pptp-server" == ifname then
				interface_found = 1
				return
			end
		end)
		uci:foreach("openvpn", "openvpn", function(b)
			if b.dev == ifname then
				interface_found = 1
				return
			end
		end)
	end
	return (interface_found == 1)
end

t = s:option(Value, "target", translate("Destination address"), translate("The address of the destination network"))
t.datatype = "ip4addr"
t.rmempty = false

function t.validate(self, value, section)
	netmask = luci.http.formvalue("cbid." .. self.config .. "." .. section .. ".netmask")
	network = luci.util.trim(luci.sys.exec("ipcalc.sh "..value.." " ..netmask.." | grep 'NETWORK=' | awk -F '=' '{print $2}' "))
	if network == value then
		return value
	else
		function t.cfgvalue(self, sec)
			if section == sec then
				return network
			else
				return self.map:get(sec, "target")
			end
		end
		m.message = translatef("To match specified netmask, destination address was changed from %s to %s. Click save to apply changes.", value , network)
		--return nil, translate("ERROR")
		return nil
	end
end

n = s:option(Value, "netmask", translate("Netmask"), translate("Netmask that is applied to the destination IP address to determine if the routing rule applies"))
n.placeholder = "255.255.255.255"
n.datatype = "ip4addr"
n.rmempty = true

g = s:option(Value, "gateway", translate("Gateway"), translate("Next hop router for the specified routing rule"))
g.datatype = "ip4addr"
g.rmempty = true

metric = s:option(Value, "metric", translate("Metric"), translate("Used as a sorting measure. If a packet about to be routed fits two rules, the one with the lower metric is applied. Range (0-255)"))
metric.placeholder = 0
metric.datatype = "range(0,255)"
metric.rmempty = true
metric.maxWidth = "40px"

if routes6 and tonumber(ipv6_enable)==1 then
        s = m:section(TypedSection, "route6", translate("Static IPv6 Routes"))
        s.addremove = true
        s.anonymous = true

        s.template  = "cbi/tblsection"

        iface = s:option(ListValue, "interface", translate("Interface"))
        luci.tools.webadmin.cbi_add_networks(iface,"routes")

        t = s:option(Value, "target", translate("Target"), translate("IPv6-Address or Network (CIDR)"))
        t.datatype = "ip6addr"
        t.rmempty = false

        g = s:option(Value, "gateway", translate("IPv6-Gateway"))
        g.datatype = "ip6addr"
        g.rmempty = true

        metric = s:option(Value, "metric", translate("Metric"))
        metric.placeholder = 0
        metric.datatype = "range(0,65535)" -- XXX: not sure
        metric.rmempty = true

end

m2 = Map("static_arp", translate(""), translate(""))

s = m2:section(TypedSection, "rule", translate("Static ARP Entries"))
s.addremove = true
s.anonymous = true

s.template  = "cbi/tblsection"
s.novaluetext = translate("There are no static ARP entries yet")

o = s:option(Value, "ip", translate("IP address"), translate("IP address for static ARP entries"))
o.datatype = "ip4addr"
o.rmempty = false

o = s:option(Value, "mac", translate("MAC address"), translate("MAC address for static ARP entries"))
o.datatype = "macaddr"
o.rmempty = false

return m, m2
