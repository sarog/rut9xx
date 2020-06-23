--[[
LuCI - Lua Configuration Interface

Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0
]]--

local map, section, net, thisIsAWizard, isLan = ...
local ifc = net:get_interface()

local ipaddr, netmask, gateway, broadcast, dns, accept_ra, send_rs, ip6addr, ip6gw
local macaddr, mtu, metric
local ut = require "luci.util"
local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()
local ds = require "luci.dispatcher"
local ipv6_enable=ut.trim(sys.exec("uci get -q system.ipv6.enable"))

if thisIsAWizard == true then
	ipaddr = section:option(Value, "ipaddr", translate("IPv4 address"))
	netmask = section:option(Value, "netmask", translate("IPv4 netmask"))
	if not isLan then
		gateway = section:option(Value, "gateway", translate("IPv4 gateway"))
	end
	broadcast = section:option(Value, "broadcast", translate("IPv4 broadcast"))
	dns = section:option(DynamicList, "dns", translate("Use custom DNS servers"))
	dns.datatype = "ipaddr"
	dns.cast     = "string"
else
	ipaddr = section:taboption("general", Value, "ipaddr", translate("IP address"), translate("Address that the router uses on the LAN (Local Area Network)"))
	netmask = section:taboption("general", Value, "netmask", translate("IP netmask"), translate("A mask used to define how large the LAN (Local Area Network) is"))
	if not isLan then
		gateway = section:taboption("general", Value, "gateway", translate("IP gateway"), translate("Specify the default gateway, the address where traffic destinated for the Internet is routed to"))
	end
	broadcast = section:taboption("general", Value, "broadcast", translate("IP broadcast"), translate("IP broadcasts are used by BOOTP (The Bootstrap Protocol) and DHCP (Dynamic Host Configuration Protocol) clients to find and send requests to their respective servers"))

	if tonumber(ipv6_enable)==1 then
		ipaliases_ip6 = section:taboption("general", Value, "ip6addr", translate("IPv6 Address"), translate("Address that the router uses on the LAN (Local Area Network)."))
		ipaliases_ip6.optional = true
		ipaliases_ip6.datatype = "ip6addr"
		ipaliases_ip6.default = "2001:db80::2"
		function ipaliases_ip6.cfgvalue(self, section)
			local value = ut.trim(sys.exec("uci get -q network.lan.ip6addr")) or ""
			local val = value:split("/")
			if val[1] == nil or #val[1] == 0 then
				val = "2001:db80::2"
			else
				val = val[1]
			end
			return val
		end
		ipaliases_pr6 = section:taboption("general", Value, "_ip6pr", translate("IPv6 Prefix"), translate("Prefix"))
		ipaliases_pr6.optional = true
		ipaliases_pr6.datatype = "range(1,128)"
		ipaliases_pr6.default = "64"
		function ipaliases_pr6.cfgvalue(self, section)
			local value = ut.trim(sys.exec("uci get -q network.lan.ip6addr")) or ""
			local val = value:split("/")
			if val[2] == nil or #val[2] == 0 then
				val = "64"
			else
				val = val[2]
			end
			return val
		end
		function ipaliases_pr6.write(self, section, value)
			return value
		end
	end

	if isLan ~= true then
		send_rs = s:taboption("advanced", Flag, "send_rs", translate("Send router solicitations"))
		send_rs.default = send_rs.enabled
		send_rs:depends("accept_ra", "")
		dns = section:taboption("advanced", DynamicList, "dns", translate("Use custom DNS servers"), translate("Multiple DNS (Domain Name System) servers can be entered by clicking new entry button near a text input field"))
		dns.datatype = "ipaddr"
		dns.cast     = "string"
	end
end

--ipaddr = section:taboption("general", Value, "ipaddr", translate("IPv4 address"))
ipaddr.datatype = "ip4lan"
ipaddr.rmempty = false

--netmask = section:taboption("general", Value, "netmask", translate("IPv4 netmask"))

netmask.datatype = "ip4addr"
netmask:value("255.255.255.0")
netmask:value("255.255.0.0")
netmask:value("255.0.0.0")


--gateway = section:taboption("general", Value, "gateway", translate("IPv4 gateway"))
if not isLan then
	gateway.datatype = "ip4addr"
end


--broadcast = section:taboption("general", Value, "broadcast", translate("IPv4 broadcast"))
broadcast.datatype = "ip4addr"


--dns = section:taboption("general", DynamicList, "dns", translate("Use custom DNS servers"))



if luci.model.network:has_ipv6() then

	if thisIsAWizard == true then
		--accept_ra = s:option(Flag, "accept_ra", translate("Accept router advertisements"))
		accept_ra = section:option(Flag, "accept_ra", translate("Accept router advertisements"))

		--send_rs = s:option(Flag, "send_rs", translate("Send router solicitations"))
		send_rs = section:option(Flag, "send_rs", translate("Send router solicitations"))
		ip6addr = section:option(Value, "ip6addr", translate("IPv6 address"))
		ip6gw = section:option(Value, "ip6gw", translate("IPv6 gateway"))
	else
		accept_ra = s:taboption("advanced", Flag, "accept_ra", translate("Accept router advertisements"))
		accept_ra.default = accept_ra.disabled
		--[[
		ip6addr = section:taboption("general", Value, "ip6addr", translate("IPv6 address"))
 		ip6gw = section:taboption("general", Value, "ip6gw", translate("IPv6 gateway"))]]
	end

	--accept_ra = s:taboption("general", Flag, "accept_ra", translate("Accept router advertisements"))



	--send_rs = s:taboption("general", Flag, "send_rs", translate("Send router solicitations"))


--[[
	--ip6addr = section:taboption("general", Value, "ip6addr", translate("IPv6 address"))
	ip6addr.datatype = "ip6addr"
	ip6addr:depends("accept_ra", "")


	--ip6gw = section:taboption("general", Value, "ip6gw", translate("IPv6 gateway"))
	ip6gw.datatype = "ip6addr"
	ip6gw:depends("accept_ra", "")]]

end

if thisIsAWizard ~= true then

	if isLan ~= true then
		macaddr = section:taboption("advanced", Value, "macaddr", translate("Override MAC address"), translate("Override MAC (Media Access Control) address of the LAN (Local Area Network) interface"))
		macaddr.placeholder = ifc and ifc:mac() or "00:00:00:00:00:00"
		macaddr.datatype    = "macaddr"
	end


	mtu = section:taboption("advanced", Value, "mtu", translate("Override MTU"), translate("MTU (Maximum Transmission Unit) specifies the largest possible size of a data packet"))
	mtu.placeholder = "1500"
	mtu.datatype    = "max(1500)"

	metric = section:taboption("advanced", Value, "metric",
		translate("Use gateway metric"), translate("The LAN (Local Area Network) configuration generates a routing table entry by default. With this field you can alter the metric of that entry"))

	metric.placeholder = "0"
	metric.datatype    = "uinteger"
	arg[1] = arg[1] or "LAN"

	wan_lan = section:taboption("advanced", Flag, "_ifname", translate("Use WAN port as LAN"), translate("Use all Ethernet port as LAN"))

	uci:foreach("network", "interface", function(s)
		if s["ifname"] == "eth1" then
			if s["disabled"] ~= "1" then
				wan_lan.hardDisabled = true
				wan_lan.info = "Wired WAN can not be selected as main or secondary WAN"
				wan_lan.url = ds.build_url("/admin/network/wan")
			end
		end
	end)

	wan_lan.forcewrite = true
	wan_lan.rmempty = false

	function remove_eth1(str,section)
		if string.match(str, "eth1") and not string.match(str, "eth1%.") then
			i, j = string.find(str, 'eth1')
		elseif string.match(str, "eth1%.") then
			i, j = string.find(str, 'eth1%.[0-9]+')
		else
			return;
		end
		if i and j then
			ifn1=""
			ifn2=""
			if i > 2 then
				ifn1 = string.sub(str, 1, i-2)
			end
			if string.len(str)~= j then
				ifn2 = string.sub(str, j+1)
			end
			uci:set("network",section,"ifname",ifn1.."".. ifn2)
			uci:save("network")
		end
	end

	function wan_lan.write(self, section, val)
		local tmp = ""
		local changed = false
		switch_vlan_num = ""
		a = uci:get("network", section, "ifname")
		if val and tonumber(val) == 1 then
			if not string.match(a, "eth1") then
				uci:set("network", section, "ifname", a.." eth1")

				uci:foreach("network", "switch_vlan", function(sw_vlan)
					if not changed then
						switch_vlan_num = do_work_with_unchanged(sw_vlan, a, true)
					end
				end	)
				changed = false

				uci:save("network")
			end
			uci:foreach("network", "interface", function(l)
				type = string.sub(l[".name"], 1, 3)
				if type == "lan" then
					if l[".name"] ~= section then
						remove_eth1(l.ifname,l[".name"])

						uci:foreach("network", "switch_vlan", function(sw_vlan)
							if not changed and switch_vlan_num ~= sw_vlan.vid and switch_vlan_num ~= "" then
								do_work_with_unchanged(sw_vlan, l.ifname, false)
							end
						end	)
						changed = false
						tmp = ""
					end
				end
			end	)
		else
			remove_eth1(a, section)

			uci:foreach("network", "switch_vlan", function(sw_vlan)
				if not changed then
					do_work_with_unchanged(sw_vlan, a, false)
				end
			end	)
			changed = false
			tmp = ""
		end
		uci:save("network")
	end

	function wan_lan.cfgvalue(self, section)
		a=uci:get("network",section,"ifname")
		if string.match(a, "eth1") then
			val="1"
		else
			val="0"
		end

		return val
	end

	function get_length(array)
		local ret_value = 1

		if array == nil then
			return 0
		end

		for k in array do
			ret_value = ret_value + 1
		end
		return ret_value
	end

	function iterate_and_form_string_with_one(str_iter, str_len)
		local tmp = ""
		local i = 1

		for k in string.gmatch(str_iter, '%w+') do
			if k == "0" or k == "0t" then
				if i == str_len then
					tmp = tmp..""..k.." 1"
				else
					tmp = tmp..""..k.." 1 "
				end
			elseif k ~= "1" and k ~= "1t" then
				if i == str_len then
					tmp = tmp..""..k
				else
					tmp = tmp..""..k.." "
				end
			end
			i = i + 1
		end

		return tmp
	end

	function iterate_and_form_string_without_one(str_iter, str_len)
		local tmp = ""
		local i = 1

		for k in string.gmatch(str_iter, '%w+') do
			if k ~= "1" and k ~= "1t" then
				if i == str_len then
					tmp = tmp..""..k
				else
					tmp = tmp..""..k.." "
				end
			end
			i = i + 1
		end

		return tmp
	end

	function do_work_with_unchanged(switch_vlan, ifname, with_one)
		local tmp = ""
		local length_of_ports = get_length(string.gmatch(switch_vlan.ports, '%w+')) - 1
		local num_of_vlan = ""

		if string.match(ifname, "eth0") and not string.match(ifname, "eth0%.") then
			if switch_vlan.vid == "0" then
				if with_one then
					tmp = iterate_and_form_string_with_one(switch_vlan.ports, length_of_ports)
					num_of_vlan = switch_vlan.vid
				else
					tmp = iterate_and_form_string_without_one(switch_vlan.ports, length_of_ports)
				end

				changed = true
			end
		elseif string.match(ifname, "eth0%."..switch_vlan.vid) then
			if with_one then
				tmp = iterate_and_form_string_with_one(switch_vlan.ports, length_of_ports)
				num_of_vlan = switch_vlan.vid
			else
				tmp = iterate_and_form_string_without_one(switch_vlan.ports, length_of_ports)
			end

			changed = true
		end
		if tmp ~= "" then
			uci:set("network", switch_vlan[".name"], "ports", tmp)
		end
	end
end
