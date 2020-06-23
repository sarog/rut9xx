
-- some external libs
local ut = require "luci.util"
local fs = require "nixio.fs"
local nw = require "luci.model.network"
local fw = require "luci.model.firewall"
local sys = require "luci.sys"
local dsp = require "luci.dispatcher"

-- some defines
local __define__config_file_name = "network"
local __define__network = arg[1]
local __define__page_entry_path = "admin/network/wan/edit/" .. __define__network

-- Variables
-- Tables
local net, ifc
-- Basic variable set
local m, s, o
-- Page fields
local op_mode,			-- Operation Mode
      proto_sel,		-- Protocol selection (drop down list)
      proto_switch,		-- Protocol switch button
      no_nat			-- NAT disabling flag

-- Global variables/constants/defines etc...
local protoSwitchBtnName = "_switch"
local interfaceName
local networkProtocol
local has_udprelay = fs.access("/usr/sbin/udp-broadcast-relay")
-- HTTP GET params
local httpProto = luci.http.formvalue("prot")

--local moduletype = ut.trim(sys.exec("uci get system.module.type"))
local moduleproto = ut.trim(sys.exec("uci get network.ppp.proto"))
local ipv6_enable=ut.trim(sys.exec("uci get -q system.ipv6.enable"))

-- Helper Functions
-- custom Echo DIAGSTRING
local function debug(string, ...)
	luci.sys.call(string.format("logger -t \"Webui\" \"%s\"", string.format(string, ...)))
end

------------------------
-- Map initialisation --
------------------------
m = Map(__define__config_file_name, "WAN", translate("Your WAN configuration determines how the router will be connecting to the internet."))
	m:chain("wireless")
	m:chain("firewall")
	m:chain("multiwan")
	m.redirect = dsp.build_url("admin/network/wan")

nw.init(m.uci)
fw.init(m.uci)
net = nw:get_network(__define__network)
ifc = net:get_interface()

--------------------------------------------------------------------------------
interfaceName = m.uci:get(__define__config_file_name, __define__network, "ifname") or ifc:name()
interface_info = nw:get_interface(interfaceName)
networkProtocol = httpProto or m:formvalue("cbid." .. __define__config_file_name .. "." .. __define__network .. ".proto") or net:proto()
-------------------------------------
-- Protocol switch button handling --
-------------------------------------

if m:formvalue("cbid." .. __define__config_file_name .. "." .. __define__network .. "." .. protoSwitchBtnName) then
	local ptype = m:formvalue("cbid." .. __define__config_file_name .. "." .. __define__network .. ".proto")  or "dhcp"
	luci.http.redirect(luci.dispatcher.build_url(__define__page_entry_path) .. "?prot=" .. ptype)
	return
end

if interfaceName ~= "usb0" then

s = m:section(NamedSection, __define__network, "interface", translate("Common Configuration"))
	s:tab("general",  translate("General Setup"))
	s:tab("advanced", translate("Advanced Settings"))


-- Disable/Enable NAT
no_nat = s:taboption("advanced", Flag, "masquerade", translate("Disable NAT"), translate("If checked, router will not perform NAT (masquerade) on this interface"))
	no_nat.default = no_nat.disabled
	no_nat.rmempty = false

	function no_nat.cfgvalue(self, section)
		local cval = "0"
		m.uci:foreach("firewall", "zone", function(s)
			if s.name == "wan" then
				cval = s.masq
				if cval == "1" then	--//masq=1 means Disable NAT=0
					cval = "0"
				else
					cval = "1"
				end
				return cval
			end
		end)
		return cval
	end

	function no_nat.write(self, section, value)
		local fwzone = "nil"
		m.uci:foreach("firewall", "zone", function(s)
			if s.name == "wan" then
				fwzone = s[".name"]
				if value == "1" then	--//Disable NAT=1 means masq=0
					m.uci:set("firewall", fwzone, "masq", "0")
					m.uci:set("firewall", fwzone, "conntrack", "1")
				else
					m.uci:set("firewall", fwzone, "masq", "1")
					m.uci:delete("firewall", fwzone, "conntrack")
				end
				m.uci:save("firewall")
			end
		end)
	end

----------------------
-- Protocol prelude --
----------------------
-- Protocol selection: drop down list
if interfaceName ~= "3g-ppp" and interfaceName ~= "none"  then
	if interfaceName ~= "wwan0" and interfaceName ~= "wwan-usb0" then
		proto_sel = s:taboption("general", ListValue, "proto", translate("Protocol"), translate("Depending on Operation Mode you can switch between the Static, DHCP or PPPoE protocols by selecting the protocol that you want to use and then clicking Switch Protocol button"))
		proto_sel.default = net:proto()
		proto_sel:value("static", "Static")
		if interfaceName == "eth1" or interfaceName == "wlan0" or interfaceName == "eth2" then
			proto_sel:value("dhcp", "DHCP")
			if interfaceName == "eth1" or interfaceName == "wlan0" then
				proto_sel:value("static", "Static")
				if interfaceName == "eth1" then
					proto_sel:value("pppoe", "PPPoE")
				end
			end
		end

		function proto_sel.cfgvalue(self, section)
			return httpProto or self.map:get(section, self.option)
		end

		function proto_sel.write(self, section, val)
			-- Bad workaround for ndis mode
			wan_ifname = self.map:get(section, "ifname")
			if val == "none" and moduleproto == "ndis" and wan_ifname ~= "none" then
				val = "dhcp"
			end
			if val == "dhcp" then
				m.uci:delete("network", section, "ipaddr")
				m.uci:delete("network", section, "netmask")
				m.uci:delete("network", section, "gateway")
				m.uci:delete("network", section, "broadcast")
			end
			self.map:set(section, self.option, val)
		end

		-- Protocol switch button
		proto_switch = s:taboption("general", Button, "_switch")
			proto_switch.title      = translate("Really switch protocol?")
			proto_switch.inputtitle = translate("Switch protocol")
			proto_switch.inputstyle = "apply"
			proto_switch:depends("proto", "none")

		if interfaceName == "eth1" or interfaceName == "wlan0" then
			for _, i in ipairs({"static", "dhcp"}) do
				if networkProtocol ~= i then
					proto_switch:depends("proto", i)
				end
			end
			if interfaceName == "eth1" and networkProtocol ~= "pppoe" then
				proto_switch:depends("proto", "pppoe")
			end
		end
	end

	local form, ferr
	if networkProtocol == "none" and interfaceName == "wwan0" then
		form, ferr = loadfile(ut.libpath() .. "/model/cbi/admin_network/proto_wwan0_simp.lua")
	else
		-- Selected protocol inclusion
		form, ferr = loadfile(
			ut.libpath() .. "/model/cbi/admin_network/proto_%s_simp.lua" % networkProtocol
		)
	end

	if not form then
		s:option(DummyValue, "_error",
			translate("Missing protocol extension for proto %q" % networkProtocol)
		).value = ferr
	else
		if networkProtocol ~= net:proto() then
			setfenv(form, getfenv(1))(m, s, interface_info, true)
		else
			setfenv(form, getfenv(1))(m, s, interface_info)
		end
	end

		local _, field
		for _, field in ipairs(s.children) do
			if field ~= op_mode and field ~= proto_sel and field ~= proto_switch and field ~= no_nat then
				if networkProtocol ~= "none" and interfaceName ~= "wwan0" then
					if next(field.deps) then
						local _, dep
						for _, dep in ipairs(field.deps) do
							dep.deps.proto = networkProtocol
						end
					else
						field:depends("proto", networkProtocol)
					end
				end
			end
		end
	end

----------------------
-- Map finalisation --
----------------------
function m.on_parse(self)
	local section = "wan6"
	if tonumber(ipv6_enable)==1 then
		if networkProtocol=="dhcp" then
			m.uci:set(self.config, section, "interface")
			m.uci:set(self.config, section, "ifname" , "@wan")
			m.uci:set(self.config, section, "proto" , "dhcpv6")
		else
			m.uci:delete(self.config, section)
		end
		m.uci:commit(self.config)
	end
end
end
----------------
-- IP Aliases --
----------------

local ipaliases_s2, ipaliases_ip, ipaliases_nm, ipaliases_gw, ipaliases_ip6, ipaliases_gw6, ipaliases_bcast, ipaliases_dns

ipaliases_s2 = m:section(TypedSection, "alias", translate("IP Aliases"), translate(" "))
	ipaliases_s2.addremove = true
	ipaliases_s2.anonymous = true
	ipaliases_s2.template = "cbi/iptsection"
	ipaliases_s2.novaluetext = "There are no IP aliases created yet"
	ipaliases_s2:depends("interface", __define__network)
	ipaliases_s2.defaults.interface = __define__network

ipaliases_s2:tab("general", translate("General Setup"))
	ipaliases_s2.defaults.proto = "static"

ipaliases_ip = ipaliases_s2:taboption("general", Value, "ipaddr", translate("IP Address"), translate("Address that the router uses on the WAN network"))
	ipaliases_ip.optional = true
	ipaliases_ip.datatype = "ip4addr"

ipaliases_nm = ipaliases_s2:taboption("general", Value, "netmask", translate("Netmask"), translate("A mask used to specify the network available hosts"))
	ipaliases_nm.optional = true
	ipaliases_nm.datatype = "ip4addr"
	ipaliases_nm:value("255.255.255.0")
	ipaliases_nm:value("255.255.0.0")
	ipaliases_nm:value("255.0.0.0")
	ipaliases_nm.default = "255.255.255.0"

ipaliases_gw = ipaliases_s2:taboption("general", Value, "gateway", translate("Gateway"), translate("A route used then IP address does not mach any other route"))
	ipaliases_gw.optional = true
	ipaliases_gw.datatype = "ip4addr"

if has_ipv6 then
	ipaliases_s2:tab("ipv6", translate("IPv6 Setup"))

	ipaliases_ip6 = ipaliases_s2:taboption("ipv6", Value, "ip6addr", translate("IPv6-Address"), translate("Address that the router uses on the WAN network. CIDR-Notation: address/prefix"))
		ipaliases_ip6.optional = true
		ipaliases_ip6.datatype = "ip6addr"

	ipaliases_gw6 = ipaliases_s2:taboption("ipv6", Value, "ip6gw", translate("IPv6-Gateway"), translate("A route used then IP address does not mach any other route"))
		ipaliases_gw6.optional = true
		ipaliases_gw6.datatype = "ip6addr"
end

ipaliases_s2:tab("advanced", translate("Advanced Settings"))

ipaliases_bcast = ipaliases_s2:taboption("advanced", Value, "bcast", translate("IP Broadcast"), translate("A logical address at which all devices are enabled to receive datagrams"))
	ipaliases_bcast.optional = true
	ipaliases_bcast.datatype = "ip4addr"

ipaliases_dns = ipaliases_s2:taboption("advanced", Value, "dns", translate("DNS Server"), translate("DNS (Domain Name System) server address"))
	ipaliases_dns.optional = true
	ipaliases_dns.datatype = "ip4addr"

function m.on_commit()
	local staticip = m:formvalue("cbid." .. __define__config_file_name .. "." .. __define__network .. ".ipaddr")
-- 	local staticmask = m:formvalue("cbid." .. __define__config_file_name .. "." .. __define__network .. ".netmask")
	if staticip and staticip ~= "" --[[and staticmask and staticmask ~= ""]] then
		luci.sys.exec("/usr/sbin/wan_rule_man.sh edit ".. staticip .." ".. __define__network .."")
	end
end

function m.on_after_commit(self)
	local wifi_section
	local sifname = m.uci:get(self.config, __define__network, "ifname")

	m.uci:foreach(self.config, "wifi-iface", function(ws)
		if ws.network == "wan" then
			wifi_section = ws[".name"]
		end
	end)

	if sifname == "wlan0" and wifi_section then
		-- Wirless set MAC
		local wirless_config_mac = m.uci:get(self.config, __define__network, "macaddr")
		local wmac = m:formvalue(string.format("cbid.network.%s.macaddr", __define__network))
		--luci.sys.call("/etc/init.d/fix_sta_ap start")
		if wmac and wmac ~= "" then
			m.uci:set("wireless", wifi_section, "macaddr", wmac)
			m.uci:commit("wireless")
		elseif wirless_config_mac then
			m.uci:delete("wireless", wifi_section, "macaddr", wmac)
			m.uci:commit("wireless")
		end

		-- Wireless set MTU
		local configmtu = m.uci:get(self.config, __define__network, "mtu")
		if not configmtu then
			local wlanmntu = ut.trim(luci.sys.exec("ifconfig wlan0 | grep -wc 'MTU:1500'"))
			if wlanmntu ~= "1" then
				luci.sys.call("ifconfig wlan0 down mtu 1500 up 2>/dev/null")
			end
		else
			local wirlessmtu = ut.trim(luci.sys.exec("ifconfig wlan0 | grep -wc 'MTU:" .. configmtu .. "'"))
			if wirlessmtu ~= "1" then
				luci.sys.call("ifconfig wlan0 down mtu " .. configmtu .. " up 2>/dev/null ")
			end
		end
-- 	else
-- 		fix_sta_ap = ut.trim(luci.sys.exec("ps | grep fix_sta_ap.sh | grep -v grep"))
-- 		if fix_sta_ap ~= "" then
-- 			luci.sys.call("killall fix_sta_ap.sh")
-- 		end
	end

-- 	if sifname == "eth1" then
		--Panasu kad sitas workaround nebereikalingas
		-- Wired Mac address set new/default/leave old
-- 		local ethconfig = m.uci:get(self.config, __define__network, "macaddr")
-- 		if not ethconfig then
-- 			local ethdefault = ut.trim(luci.sys.exec("/sbin/mnf_info maceth"))
-- 			local lowercasemac = ut.trim(luci.sys.exec("ifconfig eth1 | grep HWaddr | grep eth1 | awk '{print $5}' | awk -F ':' '{ print toupper($1$2$3$4$5$6) }' "))
-- 			if ethdefault ~= lowercasemac then
-- 				luci.sys.call("ifconfig eth1 down hw ether " .. ethdefault .. " up 2>/dev/null")
-- 			end
-- 		else
-- 			local wiwanmac = ut.trim(luci.sys.exec("ifconfig eth1 | grep -wc 'HWaddr " .. ethconfig .. "'"))
-- 			if ethmacaddress ~= "1" then
-- 				luci.sys.call("ifdown wan 2>/dev/null")
-- 				luci.sys.call("ifup wan 2>/dev/null")
-- 			end
-- 		end

		-- Wired MTU set new/default/leave old
-- 		local eth_config_mtu = m.uci:get(self.config, __define__network, "mtu")
-- 		if not eth_config_mtu then
-- 			local defaultmntu = ut.trim(luci.sys.exec("ifconfig eth1 | grep -wc 'MTU:1500'"))
-- 			if defaultmntu ~= "1" then
-- 				luci.sys.call("ifdown wan 2>/dev/null")
-- 				luci.sys.call("ifup wan 2>/dev/null")
-- 			end
-- 		else
-- 			local wmtu = ut.trim(luci.sys.exec("ifconfig eth1 | grep -wc 'MTU:" .. eth_config_mtu .. "'"))
-- 			if wmtu ~= "1" then
-- 				luci.sys.call("ifdown wan 2>/dev/null")
-- 				luci.sys.call("ifup wan 2>/dev/null")
-- 			end
-- 		end
-- 	end
	-- Mobile MTU set new/default/leave olfd
-- 	if sifname == "eth2" then
-- 		local networkmtu = m.uci:get(self.config, __define__network, "mtu")
-- 		if not networkmtu then
-- 			local defaultmntu = ut.trim(luci.sys.exec("ifconfig eth2 | grep -wc 'MTU:1500'"))
-- 			if defaultmntu ~= "1" then
-- 				luci.sys.call("ifconfig eth2 down mtu 1500 up 2>/dev/null")
-- 			end
-- 		else
-- 			local mobilemtu = ut.trim(luci.sys.exec("ifconfig eth2 | grep -wc 'MTU:" .. networkmtu .. "'"))
-- 			if mobilemtu ~= "1" then
-- 				luci.sys.call("ifdown wan 2>/dev/null")
-- 				luci.sys.call("ifup wan 2>/dev/null")
-- 			end
-- 		end
-- 	end
end

m2 = Map("multiwan")

ms = m2:section(NamedSection, __define__network, "interface", translate("Failover Configuration"),
	translate("Timing and other parameters will indicate how and when it will be determined that your conventional connection has gone down."))

interval = ms:option(ListValue, "health_interval", translate("Health monitor interval"), translate("Interval to check whether network connection is still up"))
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

icmp_hosts = ms:option(Value, "icmp_hosts", translate("Health monitor ICMP host(s)"), translate("A remote host to ping (send an ICMP (Internet Control Message Protocol) packet to) and determine when connection goes down"))
	icmp_hosts:value("disable", translate("Disable"))
	icmp_hosts:value("dns", translate("DNS Server(s)"))
	icmp_hosts:value("gateway", translate("WAN Gateway"))
	icmp_hosts.default = "dns"
	icmp_hosts.optional = false
	icmp_hosts.rmempty = false

timeout = ms:option(ListValue, "timeout", translate("Health monitor ICMP timeout"), translate("A timeout value for ICMP (Internet Control Message Protocol) packet"))
	timeout:value("1", translate("1 sec."))
	timeout:value("2", translate("2 sec."))
	timeout:value("3", translate("3 sec."))
	timeout:value("4", translate("4 sec."))
	timeout:value("5", translate("5 sec."))
	timeout:value("10", translate("10 sec."))
	timeout.default = "3"
	timeout.optional = false
	timeout.rmempty = false

fail = ms:option(ListValue, "health_fail_retries", translate("Attempts before failover"), translate("Failed ping attempts\\' count before switching to failover connection"))
	fail:value("1", "1")
	fail:value("3", "3")
	fail:value("5", "5")
	fail:value("10", "10")
	fail:value("15", "15")
	fail:value("20", "20")
	fail.default = "3"
	fail.optional = false
	fail.rmempty = false

recovery = ms:option(ListValue, "health_recovery_retries", translate("Attempts before recovery"), translate("Successful ping attempts\\' count before switching back to regular connection"))
	recovery:value("1", "1")
	recovery:value("3", "3")
	recovery:value("5", "5")
	recovery:value("10", "10")
	recovery:value("15", "15")
	recovery:value("20", "20")
	recovery.default = "5"
	recovery.optional = false
	recovery.rmempty = false

execute = ms:option(Flag, "execute", translate("Execute command"), translate("Execute command after switching to this interface"))
execute.default = 0
execute.optional = false
execute.rmempty = false

command = ms:option(Value, "command", translate("Command"), translate("Specify a command to execute after switching to this interface"))
command:depends("execute", "1")

if has_udprelay then
	local interface_name = interfaceName
	local section_name = interface_name:gsub("-", "_")

	m3 = Map("udprelay", translate(""), translate(""))

	s = m3:section(TypedSection, "general", translate("UDP Broadcast Relay"), translate(""))
	s.addremove = false
	s:depends("interface_mark", interface_name)
	s.defaults = { interface_mark = interface_name }

	function s.cfgsections()
		local section = TypedSection.cfgsections(s)
		if section and #section == 0 then
			return { section_name }
		end
		return section
	end

	function s.parse(self, novld)
		local section = TypedSection.cfgsections(self)
		local enabled = m3:formvalue(string.format("cbid.%s.%s.%s"
			% { self.config, section_name, "enabled" })) or "0"
		if section and #section == 0 and enabled == "1" then
			TypedSection.create(self, section_name)
		end
		TypedSection.parse(self, novld)
	end

	o = s:option(Flag, "enabled", translate("Enable"), translate("Enable UDP broadcast relay"))

	o = s:option(Value, "port", translate("Port"), translate("Specify a port which the UDP broadcast relay will listen on for incoming packets to relay"))
	o.datatype = "port"
	o:depends({ enabled = "1" })

	o = s:option(Value, "interfaces", translate("Interfaces"), translate("UDP broadcast relay destination interfaces. Open port 137 in firewall so LAN could be reachable from WAN"))
	o.template = "cbi/network_netlist"
	o.novirtual = true
	o.nocreate = true
	o.cast = "string"
	o.widget = "checkbox"
	o.exclude = net:name()
	o:depends({ enabled = "1" })

	function o.cfgvalue(self, section)
		local section_names = { }
		local interfaces = self.map:get(section, self.option) or { }
		for _, v in ipairs(interfaces) do
			self.map.uci:foreach(m.config, "interface", function(s)
				if s.ifname ~= v then
					return true -- continue
				end
				if s[".name"] then
					table.insert(section_names, s[".name"])
				end
			end)
		end
		return section_names
	end

	function o.write(self, section, value)
		value = type(value) == "string" and { value } or value
		local interfaces = { }
		for _, v in ipairs(value) do
			self.map.uci:foreach(m.config, "interface", function(s)
				if s[".name"] ~= v then
					return true -- continue
				end
				if s.ifname then
					table.insert(interfaces, s.ifname)
				end
			end)
		end
		return ListValue.write(self, section, interfaces)
	end

end

return unpack({ m, m2, m3 })
