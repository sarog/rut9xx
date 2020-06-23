
-- FIXME: This shit is rapidly descending into 104* territory, everything, including network.lua (the one that's +30Kb) have to
-- be reviewed to allow for better multi module support!!!!!

require("luci.tools.webadmin")

local nw = require "luci.model.network"
local fw = require "luci.model.firewall"
local ModuleVidPid = luci.util.trim(luci.sys.exec("uci get -q system.module.vid"))..":"..luci.util.trim(luci.sys.exec("uci get -q system.module.pid"))
local modulservice = "3G"
local moduleproto = luci.util.trim(luci.sys.exec("uci get -q network.ppp.proto"))
local DoubleRun	= false		-- prevent write function being called twice
if ModuleVidPid == "12D1:1573" or ModuleVidPid == "12D1:15C1" or ModuleVidPid == "12D1:15C3" or ModuleVidPid == "1BC7:1201" or ModuleVidPid == "1BC7:0036" then
	modulservice = "LTE"
end

m = Map("multiwan", translate("WAN Failover"))
m:chain("network")
m:chain("firewall")

nw.init(m.uci)
fw.init(m.uci)

local g3_wan_ifname = "usb0" 

local wanifname = nw:get_wan_ifname()

local module = nw:get_module()
local moduleIfname = nw:get_module_fname()

--[[if module == "3g_ppp" then
	g3_wan_ifname = "3g-ppp"
elseif module == "ndis" then
	g3_wan_ifname = "eth1"
end]]


function wifi_sta_static()
	--luci.sys.call("echo \"entering wifi_sta_static...\" >> /tmp/log.log")
	local staConfigPresent = true
	local uci = require("luci.model.uci").cursor()
	uci:foreach("wireless", "wifi-iface", function(s)
		if s.mode == "sta" then
			staConfigPresent = false
		end
	end)
	return staConfigPresent
end

s = m:section(NamedSection, "config", "multiwan", translate("Failover Link"))

if wanifname == moduleIfname or wanifname == "eth2" or wanifname == "wm0" then
	if wanifname == moduleIfname then
		e = s:option(Flag, "enabled", translate("Enable"), translate("Mobile selected as WAN (Wide Area Network) - cannot enable failover link"))
	else
		e = s:option(Flag, "enabled", translate("Enable"), translate("WiMAX selected as WAN (Wide Area Network) - cannot enable failover link"))
	end
	e.hardDisabled = true
else
	local hasssid = wifi_sta_static()
	local bridge_on = luci.util.trim(luci.sys.exec("uci get -q network.ppp.bridge"))
	if hasssid and wanifname == "wlan0" then
		e = s:option(Flag, "enabled", translate("Enable"), translate("Wireless SSID (Service Set Identifier) is not set - cannot enable failover link"))
		e.hardDisabled = true
	elseif bridge_on == "1" then
		e = s:option(Flag, "enabled", translate("Enable"), translate("Cannot enable failover link in bridge mode"))
		e.hardDisabled = true
	else
		e = s:option(Flag, "enabled", translate("Enable"), translate("Here you can setup WAN (Wide Area Network) failover. If your conventional WAN connection, such as wired Ethernet or Wireless, fails, the failover link will be enabled and take over to keep the router connected"))
	end
end

e.rmempty = false
e.default = e.enabled

function e.write(self, section, value)
	local wanifname = nw:get_wan_ifname()
	local PreviousState = m.uci:get("multiwan", "config", "enabled")
	local CurrentState = value
	local fail = false
	if DoubleRun == false then
		DoubleRun = true
	else
		return
	end	
	
	if CurrentState ~= PreviousState then
		if CurrentState == "1" and (wanifname == moduleIfname or wanifname == "eth2" or wanifname == "wm0") then
			if wanifname == moduleIfname then
				m.message = translate("wrn:%s selected as WAN - cannot enable failover link.", modulservice)
			else
				m.message = translate("wrn:WiMAX selected as WAN - cannot enable failover link.")
			end
			return
		end
		if CurrentState == "1" then
			if module == "3g_ppp" then
				m.uci:set("network", "ppp", "enabled", "1")
				m.uci:set("network", "ppp", "mwmode", "1")
				--m.uci:save("network")
				--m.uci:commit("network")
				local opts = { 
						proto	= "",
						ifname	= "" }
				-- Set proto none for 3G to prevent dhcpc from starting
				if moduleproto == "3g" then
					opts.proto 	= "none"
					opts.ifname 	= moduleIfname
				else
					opts.proto 	= "dhcp"
					opts.metric 	= "10"
					opts.ifname 	= moduleIfname
				end
			
				nw:add_network("wan2", opts)
				local zn = fw:get_zone("wan")
				zn:add_network("wan2")
				--os.execute("/etc/init.d/multiwan enable")
			else
				fail = true
			end
		else
			if module == "3g_ppp" then
				m.uci:set("network", "ppp", "enabled", "0")
				m.uci:set("network", "ppp", "mwmode", "0")
				--m.uci:save("network")
				--m.uci:commit("network")
				nw:del_network("wan2")
				--os.execute("/etc/init.d/multiwan stop 2>/dev/null 1>&2")
			else
				fail = true
			end
			
		end	
		
		if fail then
			m.message = translate("wrn:No mobile module detected!")
			return
		end
		nw:save("network")
		nw:commit("network")
		Flag.write(self, section, value)
	end
	
	--[[
	if wanifname == g3_wan_ifname or wanifname == "eth1" or wanifname == "wm0" then
		--Flag.write(self, section, "0")
		if wanifname == g3_wan_ifname then
			m.message = translate("wrn:Mobile selected as wan - cannot enable backup link.")
		else
			m.message = translate("wrn:WiMAX selected as wan - cannot enable backup link.")
		end
		--luci.http.redirect(luci.dispatcher.build_url("admin/network/wan"))
		return
	else
		if value == "0" then
			if module == "3g_ppp" then
				m.uci:set("network", "ppp", "enabled", "0")
				m.uci:save("network")
				m.uci:commit("network")
			end
			nw:del_network("wan2")
			os.execute("/etc/init.d/multiwan stop")
		else
			if module == "3g_ppp" then
				m.uci:set("network", "ppp", "enabled", "1")
				m.uci:save("network")
				m.uci:commit("network")
			end
			local opts = {
				proto	= "dhcp",
				ifname	= g3_wan_ifname
			}
			if module == "wimax" then
				opts.ifname = moduleStruct:get_iface()
			end
			nw:add_network("wan2", opts)
			local zn = fw:get_zone("wan")
			zn:add_network("wan2")
			--os.execute("/etc/init.d/multiwan enable")
		end
		nw:save("network")
		nw:commit("network")
		Flag.write(self, section, value)
	end]]
end

s = m:section(NamedSection, "wan", "interface", translate("Failover Configuration"),
	translate("If mobile is selected as WAN, you cannot enable failover link. \nTiming and other parameters will indicate how and when it will be determined that your conventional connection has gone down."))
s.addremove = false

--[[weight = s:option(ListValue, "weight", translate("Load Balancer Distribution"))
weight:value("10", "10")
weight:value("9", "9")
weight:value("8", "8")
weight:value("7", "7")
weight:value("6", "6")
weight:value("5", "5")
weight:value("4", "4")
weight:value("3", "3")
weight:value("2", "2")
weight:value("1", "1")
weight:value("disable", translate("None"))
weight.default = "10"
weight.optional = false
weight.rmempty = false]]

interval = s:option(ListValue, "health_interval", translate("Health monitor interval"), translate("Interval to check whether network connection is still up"))
interval:value("disable", translate("Disable"))
interval:value("5", translate("5 sec."))
interval:value("10", translate("10 sec."))
interval:value("20", translate("20 sec."))
interval:value("30", translate("30 sec."))
interval:value("60", translate("60 sec."))
interval:value("120", translate("120 sec."))
interval.default = "10"
interval.optional = false
interval.rmempty = false

icmp_hosts = s:option(Value, "icmp_hosts", translate("Health monitor ICMP host(s)"), translate("A remote host to ping (send an ICMP (Internet Control Message Protocol) packet to) and determine when connection goes down"))
icmp_hosts:value("disable", translate("Disable"))
icmp_hosts:value("dns", translate("DNS Server(s)"))
icmp_hosts:value("gateway", translate("WAN Gateway"))
icmp_hosts.default = "dns"
icmp_hosts.optional = false
icmp_hosts.rmempty = false

timeout = s:option(ListValue, "timeout", translate("Health monitor ICMP timeout"), translate("A timeout value for ICMP (Internet Control Message Protocol) packet"))
timeout:value("1", translate("1 sec."))
timeout:value("2", translate("2 sec."))
timeout:value("3", translate("3 sec."))
timeout:value("4", translate("4 sec."))
timeout:value("5", translate("5 sec."))
timeout:value("10", translate("10 sec."))
timeout.default = "3"
timeout.optional = false
timeout.rmempty = false

fail = s:option(ListValue, "health_fail_retries", translate("Attempts before WAN failover"), translate("Failed ping attempts\\' count before switching to failover WAN (Wide Area Network)"))
fail:value("1", "1")
fail:value("3", "3")
fail:value("5", "5")
fail:value("10", "10")
fail:value("15", "15")
fail:value("20", "20")
fail.default = "3"
fail.optional = false
fail.rmempty = false

recovery = s:option(ListValue, "health_recovery_retries", translate("Attempts before WAN recovery"), translate("Successful ping attempts\\' count before switching back to regular WAN (Wide Area Network)"))
recovery:value("1", "1")
recovery:value("3", "3")
recovery:value("5", "5")
recovery:value("10", "10")
recovery:value("15", "15")
recovery:value("20", "20")
recovery.default = "5"
recovery.optional = false
recovery.rmempty = false

--[[failover_to = s:option(ListValue, "failover_to", translate("Failover Traffic Destination"))
failover_to:value("disable", translate("None"))
luci.tools.webadmin.cbi_add_networks(failover_to)
failover_to:value("fastbalancer", translate("Load Balancer(Performance)"))
failover_to:value("balancer", translate("Load Balancer(Compatibility)"))
failover_to.default = "balancer"
failover_to.optional = false
failover_to.rmempty = false]]

--[[dns = s:option(Value, "dns", translate("DNS Server(s)"))
dns:value("auto", translate("Auto"))
dns.default = "auto"
dns.optional = false
dns.rmempty = true]]--

s = m:section(NamedSection, "wan2", "interface", translate("Failover Check"),
	translate("A remote host that will be used to test whether your failover link is alive."))
s.addremove = false

o = s:option(Value, "icmp_hosts", translate("ICMP host"), translate("A remote host to test failover link"))
o.optional = false
o.datatype = "ip4addr"

--[[s = m:section(TypedSection, "mwanfw", translate("Multi-WAN Traffic Rules"),
	translate("Configure rules for directing outbound traffic through specified WAN Uplinks."))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true

src = s:option(Value, "src", translate("Source Address"), translate("Specify source address to get error messages"))
src.rmempty = true
src:value("", translate("all"))
luci.tools.webadmin.cbi_add_knownips(src)

dst = s:option(Value, "dst", translate("Destination Address"), translate("Specify destination address to transmit data"))
dst.rmempty = true
dst:value("", translate("all"))
luci.tools.webadmin.cbi_add_knownips(dst)

proto = s:option(Value, "proto", translate("Protocol"), translate("Protocol for data transmission"))
proto:value("", translate("all"))
proto:value("tcp", "TCP")
proto:value("udp", "UDP")
proto:value("icmp", "ICMP")
proto.rmempty = true

ports = s:option(Value, "ports", translate("Ports"))
ports.rmempty = true
ports:value("", translate("all", translate("all")))

wanrule = s:option(ListValue, "wanrule", translate("WAN Uplink"), translate(""))
luci.tools.webadmin.cbi_add_networks(wanrule)
wanrule:value("fastbalancer", translate("Load Balancer(Performance)"))
wanrule:value("balancer", translate("Load Balancer(Compatibility)"))
wanrule.default = "fastbalancer"
wanrule.optional = false
wanrule.rmempty = false

s = m:section(NamedSection, "config", "", "")
s.addremove = false

default_route = s:option(ListValue, "default_route", translate("Default Route"), translate(""))
luci.tools.webadmin.cbi_add_networks(default_route)
default_route:value("fastbalancer", translate("Load Balancer(Performance)"))
default_route:value("balancer", translate("Load Balancer(Compatibility)"))
default_route.default = "balancer"
default_route.optional = false
default_route.rmempty = false]]

function m.on_parse(self)
	-- We will ettempt to push multiwan to the very end of the parse chain, hopefully making it run last in the init script sequence, hence fixing the problem that has been plagueing me for fucking ever
	--luci.sys.call("echo \"on_parse called\" >> /tmp/log.log")
	self.parsechain[1] = "network"
	self.parsechain[2] = "firewall"
	self.parsechain[3] = "multiwan"
	--[[for k, v in pairs(self.parsechain) do
		luci.sys.call("echo \"k: [" .. k .. "], v: [" .. v .. "]\" >> /tmp/log.log")
	end]]
end

return m
