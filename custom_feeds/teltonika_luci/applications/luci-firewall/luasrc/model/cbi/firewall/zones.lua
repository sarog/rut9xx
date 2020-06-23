--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: zones.lua 8108 2011-12-19 21:16:31Z jow $
]]--

local ds = require "luci.dispatcher"
local fw = require "luci.model.firewall"
local ut = require "luci.util"
local sys = require "luci.sys"

local m, s, o, p, i, v

m = Map("firewall",
	translate("Firewall"),
	translate("General settings allows you to set up default firewall policy."))

fw.init(m.uci)

s = m:section(TypedSection, "defaults", translate("General Settings"))
s.anonymous = true
s.addremove = false

-- s:option(Flag, "syn_flood", translate("Enable SYN flood protection"), translate("Makes router more resistant to SYN flood attacks"))

o = s:option(Flag, "drop_invalid", translate("Drop invalid packets"), translate("A Drop action is performed on a packet that is determined to be invalid"))
o.default = o.disabled

p = {
	s:option(ListValue, "input", translate("Input"), translate("DEFAULT* action that is to be performed for packets that pass through the Input chain")),
	s:option(ListValue, "output", translate("Output"), translate("DEFAULT* action that is to be performed for packets that pass through the Output chain")),
	s:option(ListValue, "forward", translate("Forward"), translate("DEFAULT* action that is to be performed for packets that pass through the Forward chain"))
}

for i, v in ipairs(p) do
	v:value("REJECT", translate("Reject"))
	v:value("DROP", translate("Drop"))
	v:value("ACCEPT", translate("Accept"))
end

s = m:section(NamedSection, "DMZ", "dmz", translate("DMZ Configuration"))

dmz_en = s:option(Flag, "enabled", translate("Enable"), translate("By enabling DMZ for a specific internet host (e.g. your computer), you will expose that host and its services to the router\\'s WAN network"))
dmz_en.rmempty = false

--[[function dzm_en.cfgvalue(self, section)
	local rtnVal
	if self.map:get(section, "enabled") == "0" then
		rtnVal = "0"
	else
		rtnVal = "1"
	end
	return rtnVal
end]]

--[[function dzm_en.write(self, section, value)
	if value ~= "0" then
		value = nil
	end
	return Flag.write(self, section, value)
end]]

o = s:option(Value, "dest_ip", translate("DMZ host IP address"), translate("Internal (i.e. LAN) host\\'s IP address"))
o.datatype = "ip4addr"


s = m:section(TypedSection, "zone", translate("Zone Forwarding"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = false
s.extedit   = ds.build_url("admin", "network", "firewall", "zones", "%s")

function s.create(self)
	local z = fw:new_zone()
	if z then
		luci.http.redirect(
			ds.build_url("admin", "network", "firewall", "zones", z.sid)
		)
	end
end

function s.remove(self, section)
	return fw:del_zone(section)
end

o = s:option(DummyValue, "_info", translate("Source zone"))
o.template = "cbi/firewall_zoneforwards"
o.cfgvalue = function(self, section)
	return self.map:get(section, "name")
end
o = s:option(DummyValue, "_info2", translate("Destination zones"))
o.template = "cbi/firewall_zoneforwardsdst"
o.cfgvalue = function(self, section)
	return self.map:get(section, "name")
end

p = {
	--~ s:option(ListValue, "input", translate("Input")),
	s:option(ListValue, "forward", translate("Default forwarding action"))
}

for i, v in ipairs(p) do
	v:value("REJECT", translate("reject"))
	v:value("DROP", translate("drop"))
	v:value("ACCEPT", translate("accept"))
end

s:option(Flag, "masq", translate("Masquerading"))
--~ s:option(Flag, "mtu_fix", translate("MSS clamping"))


function m.on_before_save(self)

	local http_enabled = sys.exec("uci get uhttpd.main._httpWanAccess")
	if http_enabled then
		http_enabled = ut.trim(http_enabled)
	else
		http_enabled = "0"
	end
	
	local https_enabled = sys.exec("uci get uhttpd.main._httpsWanAccess")
	if https_enabled then
		https_enabled = ut.trim(https_enabled)
	else
		https_enabled = "0"
	end
	
	local dmz_enabled = luci.http.formvalue("cbid.firewall.DMZ.enabled")
	if dmz_enabled then
		dmz_enabled = ut.trim(dmz_enabled)
	else
		dmz_enabled = "0"
	end
	
	local http_port = sys.exec("uci get firewall.E_HTTP_W_P.src_dport")
	if http_port then
		http_port = ut.trim(http_port)
	else
		http_port = "80"
	end

	local https_port = sys.exec("uci get firewall.E_HTTPS_W_P.src_dport")
	if https_port then
		https_port = ut.trim(https_port)
	else
		https_port = "443"
	end

	local lan_ip = sys.exec("uci get network.lan.ipaddr")
	if lan_ip then
		lan_ip = ut.trim(lan_ip)
	else
		lan_ip = "192.168.1.1"
	end

	m.uci:foreach("firewall", "redirect", function(s)
		if s["name"] == "tlt_allow_remote_http_through_DMZ" or s["name"] == "tlt_allow_remote_https_through_DMZ" then
			sys.exec("uci delete firewall.".. s[".name"])
		end
	end)
	
	if dmz_enabled == "1" and http_enabled == "1" then
		local config_value = ut.trim(sys.exec("uci add firewall redirect"))

		sys.exec("uci set firewall." ..config_value.. ".target='DNAT'")
		sys.exec("uci set firewall." ..config_value.. ".src='wan'")
		sys.exec("uci set firewall." ..config_value.. ".dest='lan'")
		sys.exec("uci set firewall." ..config_value.. ".proto='tcp'")
		sys.exec("uci set firewall." ..config_value.. ".src_dport='" ..http_port.. "'")
		sys.exec("uci set firewall." ..config_value.. ".dest_ip='".. lan_ip .."'")
		sys.exec("uci set firewall." ..config_value.. ".dest_port='"..http_port.."'")
		sys.exec("uci set firewall." ..config_value.. ".name='tlt_allow_remote_http_through_DMZ'")
		sys.exec("uci set firewall." ..config_value.. ".enabled='1'")
	end
	
	if dmz_enabled == "1" and https_enabled == "1" then
		local config_value = ut.trim(sys.exec("uci add firewall redirect"))
		
		sys.exec("uci set firewall." ..config_value.. ".target='DNAT'")
		sys.exec("uci set firewall." ..config_value.. ".src='wan'")
		sys.exec("uci set firewall." ..config_value.. ".dest='lan'")
		sys.exec("uci set firewall." ..config_value.. ".proto='tcp'")
		sys.exec("uci set firewall." ..config_value.. ".src_dport='" ..https_port.. "'")
		sys.exec("uci set firewall." ..config_value.. ".dest_ip='".. lan_ip .."'")
		sys.exec("uci set firewall." ..config_value.. ".dest_port='"..https_port.."'")
		sys.exec("uci set firewall." ..config_value.. ".name='tlt_allow_remote_https_through_DMZ'")
		sys.exec("uci set firewall." ..config_value.. ".enabled='1'")
	end
end


return m
