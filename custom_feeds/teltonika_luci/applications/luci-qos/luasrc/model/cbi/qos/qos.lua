--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: qos.lua 7238 2011-06-25 23:17:10Z jow $
]]--

local wa = require "luci.tools.webadmin"
local fs = require "nixio.fs"
local uci = require("uci").cursor()


m = Map("qos", translate("Quality of Service"),
	translate("With QoS you can prioritize network traffic selected by addresses, ports or services."))

s = m:section(TypedSection, "interface", translate("Interfaces"))
s.addremove = false
s.anonymous = false
s.template = "cbi/tblsection"
s.sectionhead = "Interface"

e = s:option(Flag, "enabled", translate("Enable"), translate("Check to enable settings/Uncheck to disable settings"))
e.rmempty = false

function e.write(self, section, value)
	local error = 0
	if value and value == "1" then
		local download = luci.http.formvalue("cbid.qos."..section..".download")
		local upload = luci.http.formvalue("cbid.qos."..section..".upload")
		if not download or #download == 0 then
			m.message = translate("err: "..section.." Download speed field is empty!")
			error = 1
		end
		if not upload or #upload == 0 then
			m.message = translate("err: "..section.." Upload speed field is empty!")
			error = 1
		end
	end

	if error == 0 then
		m.uci:set("qos", section, "enabled", value)
	else
		m.uci:set("qos", section, "enabled", "0")
	end
end

s:option(Flag, "overhead", translate("Calculate overhead"), translate("Check to decrease upload and download ratio to prevent link saturation"))

down = s:option(Value, "download", translate("Download speed (kbit/s)"), translate("Specify maximal download speed"))
down.datatype = "uinteger"

up = s:option(Value, "upload", translate("Upload speed (kbit/s)"), translate("Specify maximal upload speed"))
up.datatype = "uinteger"

s = m:section(TypedSection, "classify", translate("Classification Rules"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true
s.sortable  = true

t = s:option(ListValue, "target", translate("Target"), translate("Select target for which rule will be applied"))
t:value("Priority", translate("Priority"))
t:value("Express", translate("Express"))
t:value("Normal", translate("Normal"))
t:value("Bulk", translate("Low"))
t.default = "Normal"


srch = s:option(Value, "srchost", translate("Source host"), translate("Packets matching this source host(s) (single IP or in CIDR notation) belong to the bucket defined in target "))
srch.rmempty = true
srch:value("", translate("All"))
wa.cbi_add_knownips(srch)
srch.maxWidth = "100px"
srch.datatype = "ip4addr"

dsth = s:option(Value, "dsthost", translate("Destination host"), translate("Packets matching this destination host(s) (single IP or in CIDR notation) belong to the bucket defined in target"))
dsth.rmempty = true
dsth:value("", translate("All"))
wa.cbi_add_knownips(dsth)
dsth.maxWidth = "100px"
dsth.datatype = "ip4addr"

p = s:option(Value, "proto", translate("Protocol"), translate("Select data transmission protocol"))
p:value("", translate("All"))
p:value("tcp", translate("TCP"))
p:value("udp", translate("UDP"))
p:value("icmp", translate("ICMP"))
p.datatype = "lengthvalidation(0,16)"
p.rmempty = true
p.maxWidth = "100px"

ports = s:option(Value, "ports", translate("Ports"), translate("Select which ports will be used for transmission"))
ports.template = "cbi/value_combobox"
ports.rmempty = true
ports:value("", translate("All"))
ports.maxWidth = "100px"
ports.datatype = "number_list"

bytes = s:option(Value, "connbytes", translate("Number of bytes"), translate("Specify the maximal number of bytes for connection"))
bytes.maxWidth = "100px"
bytes.datatype = "uinteger"

-- Previous firmware versions configured QoS in a substancially different way
-- therefore we need to fix the configuration to match the current version
function fix_user_configuration(required_interfaces)
	uci:foreach("qos", "interface", function(s)
		local current_interface = s[".name"]
		for key, value in pairs(required_interfaces) do
			if current_interface == "lan" and key == "LAN" then
				uci:rename("qos", current_interface, "LAN")
				required_interfaces[key][2] = 1
			elseif (current_interface == "wan" or current_interface == "eth1") and key == "Wired" then
				uci:rename("qos", current_interface, "Wired")
				required_interfaces[key][2] = 1
			elseif (current_interface == "ppp" or current_interface == "wwan0") and key == "Mobile" then
				uci:rename("qos", current_interface, "Mobile")
				required_interfaces[key][2] = 1
			elseif string.upper(current_interface) == string.upper(required_interfaces[key][1]) then
				uci:rename("qos", current_interface, key)
			end
			if current_interface == key then
				uci:set("qos", current_interface, "device", required_interfaces[key][1])
				required_interfaces[key][2] = 1
			end
		end
	end)
end

-- Users used to add QoS interfaces themselves, while the new configuration
-- dispalys them by default. This function will add missing interfaces after
-- firmware update with keep settings
function add_missing_interfaces(required_interfaces)
	for key, value in pairs(required_interfaces) do
		if required_interfaces[key][2] == 0 then
			uci:set("qos", key, "interface")
			uci:set("qos", key, "device", required_interfaces[key][1])
			uci:set("qos", key, "enabled", 0)
		end
	end
end

m.on_init = function(self)
	local required_interfaces = {["Wired"] = {"eth1", 0}, ["Mobile"] = {"wwan0", 0}, ["WiFi_WAN"] = {"wlan0", 0}, ["LAN"] = {"br-lan", 0}}
	fix_user_configuration(required_interfaces)
	add_missing_interfaces(required_interfaces)
	uci:commit("qos")
end

local save = m:formvalue("cbi.apply")
if save then
	--Delete all usr_enable from qos config
	m.uci:foreach("qos", "interface", function(s)
		qos_inst = s[".name"] or ""
		qosEnable = m:formvalue("cbid.qos." .. qos_inst .. ".enabled") or "0"
		qos_enable = s.enabled or "0"
		if qosEnable ~= qos_enable then
			m.uci:foreach("qos", "interface", function(a)
				qos_inst2 = a[".name"] or ""
				local usr_enable = a.usr_enable or ""
				if usr_enable == "1" then
					m.uci:delete("qos", qos_inst2, "usr_enable")
				end
			end)
		end
	end)
	m.uci:save("qos")
	m.uci.commit("qos")
end

return m
