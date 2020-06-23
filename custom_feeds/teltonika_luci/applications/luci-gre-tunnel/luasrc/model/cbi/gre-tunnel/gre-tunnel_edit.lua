local http = require "luci.http"

local VPN_INST

if arg[1] then
	VPN_INST = arg[1]
else
	return nil
end



local interfaces = {
	{ifname="3g-ppp", genName="Mobile", type="3G"},
	{ifname="eth2", genName="Mobile", type="3G"},
	{ifname="usb0", genName="WiMAX", type="WiMAX"},
	{ifname="eth1", genName="Wired", type="vlan"},
	{ifname="wlan0", genName="WiFi", type="wifi"},
	{ifname="none", genName="Mobile bridged", type="3G"},
	{ifname="wwan0", genName="Mobile", type="3G"},
	{ifname="wm0", genName="WiMAX", type="WiMAX"},
	{ifname="wwan-usb0", genName="Mobile USB", type="3G"},
}

m = Map("network", translatef("GRE Tunnel Instance: %s", VPN_INST:gsub("^%l", string.upper)), "")

m.redirect = luci.dispatcher.build_url("admin/services/vpn/gre-tunnel/")

--~ GRE PROTOCOL INTERFACE SECTION
s = m:section( NamedSection, arg[1], "interface", translate("Main Settings"), "")

dis = s:option(Flag, "disabled", translate("Enabled"), translate("Enable GRE (Generic Routing Encapsulation) tunnel."))
	dis.rmempty = false
	dis.enabled="0"
	dis.disabled="1"
	dis.default = "0"

local local_ipaddr = s:option(Value, "ipaddr_tunlink", translate("Tunnel source"), translate("IP address of the local WAN interface."))
local_ipaddr.combobox_manual = "-- Enter IP address --"
m.uci:foreach("network", "interface", function(intf)
	for _, known_interface in ipairs(interfaces) do
		if known_interface.ifname == intf.ifname  then
			local modem = m.uci:get("system", "module", "name") or ""
			local ifname
			if modem:find("Quectel") then
				ifname = "ppp_4"
			else
				ifname = intf[".name"]
			end
			if (string.match(known_interface.genName, "Mobile") and not string.match(intf[".name"], "wan")) then
				local_ipaddr:value(ifname, translate(known_interface.genName.." ("..string.upper(intf[".name"])..")"))
			elseif not string.match(known_interface.genName, "Mobile") then
				local_ipaddr:value(intf[".name"], translate(known_interface.genName.." ("..string.upper(intf[".name"])..")"))
			end
		elseif string.match(intf[".name"], "lan_") then
			local_ipaddr:value(intf[".name"], translate("VLAN ("..string.upper(intf[".name"])..")"))
		elseif string.match(intf[".name"], "lan") then
			local_ipaddr:value(intf[".name"], translate("LAN"))
		end
	end
end)

function local_ipaddr.write(self, section, value)
	if string.match(value, "(%d+)%.(%d+)%.(%d+)%.(%d+)") then
		m.uci:set(self.config, section, "ipaddr", value)
		m.uci:delete(self.config, section, "tunlink")
	else
		m.uci:set(self.config, section, "tunlink", value)
		m.uci:delete(self.config, section, "ipaddr")
	end
end

function local_ipaddr.cfgvalue(self, section)
	return m.uci:get(self.config, section, "ipaddr") or m.uci:get(self.config, section, "tunlink")
end


o = s:option(Value, "peeraddr", translate("Remote endpoint IP address"), translate("IP address of the remote GRE tunnel device."))
	o.datatype = "ip4addr"

o = s:option(Value, "mtu", translate("MTU"),translate("MTU (Maximum Transmission Unit) for tunnel connection. Range [0-1500]."))
	o.datatype = "range(0,1500)"
	o.default = "1476"

o = s:option(Value, "ttl", translate("TTL"),translate("TTL of the encapsulating packets. Range [0-255]."))
	o.datatype = "range(0,255)"
	o.default = "255"
	o:depends("df", "1")

o = s:option(Value, "okey", translate("Outbound key"),translate("Key for outgoing packets. Range [0-65535]"))
	o.datatype = "and(range(0, 65535), lengthvalidation(0,5,'^[0-9]+$'))"

o = s:option(Value, "ikey", translate("Inbound key"),translate("Key for incomming packets. Range [0-65535]"))
	o.datatype = "and(range(0, 65535), lengthvalidation(0,5,'^[0-9]+$'))"

o = s:option(Flag, "df", translate("Don\'t fragment"), translate("If unchecked sets nopmtudisc option for tunnel. Can not be used with TTL option."))
	o.default = "1"

o = s:option(Flag, "keep_alive", translate("Keep alive"), translate("Enable Keep Alive."))

o = s:option(Value, "keep_alive_interval", translate("Keep alive interval"), translate("Time interval for Keep Alive in seconds. Range [0-255]."))
	o.datatype = "range(0,255)"
	o:depends("keep_alive", "1")

--~ STATIC GRE ADDRESS
s = m:section( NamedSection, VPN_INST.."_static", "interface", translate("Tunnel Settings"), "")

o = s:option(Value, "ipaddr", translate("Local GRE interface IP address"), translate("IP address of the local GRE tunnel device."))
	o.datatype = "ip4addr"

function o.write(self, section, value)
	self.map:set(section, self.option, value)
	self.map:set(section, "proto", "static")
	self.map:set(section, "ifname", "@"..arg[1])
end

o = s:option(Value, "netmask", translate("Local GRE interface netmask"), translate("Netmask of the local GRE tunnel device."))
	o.datatype = "ip4addr"

--~ ADDITIONAL ROUTES
s = m:section(TypedSection, "route", translate("Routing Settings"))
s.addremove = true
s.anonymous = true
s.template = "cbi/iptsection"
s:depends("dep", arg[1])
s.defaults.dep = arg[1]
s.defaults.interface = arg[1].."_static"
s.novaluetext = "There are no routes created yet"

o = s:option(Value, "target", translate("Remote subnet IP address"), translate("IP address of the remote subnet."))
	o.datatype = "ip4addr"

o = s:option(Value, "netmask", translate("Remote subnet netmask"), translate("Netmask of the remote subnet."))
	o.datatype = "ip4addr"

function m.on_commit()
	local gre_rule = m.uci:get("firewall", "ALLOW_GRE")
	local gre_disabled = false

	m.uci:foreach("network", "interface",
		function (section)
			if section["proto"] == "gre" and section["disabled"] == "1" then
				gre_disabled = true
			end
	end)

	if not gre_rule then
		m.uci:set("firewall", "ALLOW_GRE", "rule")
		m.uci:set("firewall", "ALLOW_GRE", "name", "Allow-GRE-Input")
		m.uci:set("firewall", "ALLOW_GRE", "src", "wan")
		m.uci:set("firewall", "ALLOW_GRE", "proto", "47")
		m.uci:set("firewall", "ALLOW_GRE", "target", "ACCEPT")
	end

	if gre_disabled then
			m.uci:set("firewall", "ALLOW_GRE", "enabled", "0")
			m.uci:commit("firewall")
	else
			m.uci:set("firewall", "ALLOW_GRE", "enabled", "1")
			m.uci:commit("firewall")
	end
end

return m
