local sys = require("luci.sys")
local uci = require("luci.model.uci").cursor()
local utl = require "luci.util"
local dsp = require "luci.dispatcher"

local VPN_INST

local function cecho(string)
	sys.call("echo \"vpn: " .. string .. "\" >> /tmp/log.log")
end

if arg[1] then
	VPN_INST = arg[1]
	if string.sub(VPN_INST, 1, 2) == "c_" then
		mode="CLIENT"
		VPN_INST = string.sub(VPN_INST, 3)
	elseif string.sub(VPN_INST, 1, 2) == "s_" then
		mode="SERVER"
		VPN_INST = string.sub(VPN_INST, 3)
	else
		return nil
	end
else
	return nil
end

local o

if mode == "CLIENT" then

	local m = Map("network", translatef("PPTP Client Instance: %s", VPN_INST:gsub("^%l", string.upper)), "")
	local s = m:section( NamedSection, VPN_INST, "interface", translate("Main Settings"), "")
	m.redirect = dsp.build_url("admin/services/vpn/pptp/")
	o = s:option( Flag, "enabled", translate("Enable"), translate("Enable current configuration") )
		o.forcewrite = true
		o.rmempty = false

	o = s:option( Flag, "defaultroute", translate("Use as default gateway"), translate("Use this PPTP instance as default gateway") )
		o.rmempty = false
		
	o = s:option( Flag, "client_to_client", translate("Client to client"), translate("Add route to make other PPTP clients accessible") )
		o.rmempty = false

	o = s:option( Value, "server", translate("Server"), translate("The server IP address or hostname") )
	o.datatype = "host"

	o = s:option( Value, "username", translate("User name"), translate("The user name for authorization with the server") )

	o = s:option( Value, "password", translate("Password"), translate("The password for authorization with the server. Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:;[]") )
		o.password = true
		o.datatype = "password"

	function m.on_commit(map)
		local pptpEnable = m:formvalue("cbid.network." .. VPN_INST .. ".enabled")
		if pptpEnable then
			sys.call("ifup " .. VPN_INST .. " > /dev/null")
		else
			sys.call("ifdown " .. VPN_INST .. " > /dev/null")
		end

		--~ m.uci:foreach("firewall", "zone", function(s)
			--~ if s.name == "vpn" then
				--~ if pptpEnable == "1" then
					--~ m.uci:set("firewall", s[".name"], "device", "tun+ gre+ pptp+")
				--~ else
					--~ m.uci:set("firewall", s[".name"], "device", "tun+ gre+")
				--~ end
			--~ end
		--~ end)

		m.uci:save("firewall")
		m.uci:commit("firewall")
	end
	return m

elseif mode == "SERVER" then

	local name = utl.trim(sys.exec("uci get -q pptpd." .. VPN_INST .. "._name"))
	local m = Map("pptpd", translatef("PPTP Server Instance: %s", name:gsub("^%l", string.upper)), "")
		m:chain("firewall")
	m.redirect = dsp.build_url("admin/services/vpn/pptp/")
	local s = m:section( NamedSection, "pptpd", "service", translate("Main Settings"), "")

	o = s:option( Flag, "enabled", translate("Enable"), translate("Enable current configuration") )
		o.forcewrite = true
		o.rmempty = false

	o = s:option( Value, "localip", translate("Local IP"), translate("Server IP address, e.g. 192.168.0.1") )
		o.datatype = "ip4addr"
		

	start = s:option( Value, "start", translate("Remote IP range start"), translate("IP address leases begin, e.g. 192.168.0.20"))
		start.datatype = "ip4addr"
		start.default = "192.168.0.20"

	limit = s:option( Value, "limit", translate("Remote IP range end"), translate("IP address leases end, e.g. 192.168.0.30, but < 256"))
		limit.datatype = "ip4addr"
		limit.default = "192.168.0.30"

		function limit.validate(self, value, section)
			begin = luci.http.formvalue("cbid.pptpd.pptpd.start")
			theend = luci.http.formvalue("cbid.pptpd.pptpd.limit")
			o1,o2,o3,o4 = begin:match("(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)" )
			p1,p2,p3,p4 = theend:match("(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)" )
			o4 = tonumber(o4)
			p4 = tonumber(p4)
			if o4 < p4 and p4 < 256 then
				return value
			else
				return nil, translate("The value of remote IP range end is invalid.")
			end
		end


	sc1 = m:section(TypedSection, "login")
		sc1.addremove = true
		sc1.anonymous = true
		sc1.template  = "cbi/tblsection"
	
	
	usr = sc1:option(Value, "username", translate("User name"), translate("The user name for authorization with the server"))

	pass = sc1:option(Value, "password", translate("Password"), translate("The password for authorization with the server. Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:;[]"))
		pass.password = true
		pass.datatype = "password"
	
	ipaddr = sc1:option(Value, "remoteip", translate("PPTP Client's IP"), translate("This virtual IP will be given to PPTP client. For auto assignment leave empty"))
		ipaddr.datatype = "ip4addr"

	function m.on_commit(map)
		local form_value = m:formvalue("cbid.pptpd.pptpd.enabled") or "0"

		local changes = false
		m.uci:foreach("firewall", "rule", function(x)
			if x._name == "pptpd" then
				local section = x[".name"]
				local current_value = m.uci:get("firewall", section, "enabled") or "1"
				if current_value ~= form_value then
					m.uci:set("firewall", section, "enabled", form_value)
					changes = true
				end
			end
		end)

		--~ Unused because overrides pptp zone.
		
		--~ m.uci:foreach("firewall", "zone", function(s)
			--~ if s.name == "vpn" then
				--~ if form_value == "1" then
					--~ m.uci:set("firewall", s[".name"], "device", "tun+ gre+ pptp+")
				--~ else
					--~ m.uci:set("firewall", s[".name"], "device", "tun+ gre+")
				--~ end
			--~ end
		--~ end)

		m.uci:save("firewall")
		m.uci:commit("firewall")
		
		

		begin = luci.http.formvalue("cbid.pptpd.pptpd.start")
		theend = luci.http.formvalue("cbid.pptpd.pptpd.limit")
		o1,o2,o3,o4 = begin:match("(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)" )
		p1,p2,p3,p4 = theend:match("(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)%.(%d%d?%d?)" )
		
		remoteip = begin .."-".. p4
		sys.call("uci set pptpd.pptpd.remoteip="..remoteip.."; uci commit;")
		sys.call("uci set pptpd.pptpd.limit=".. o1 ..".".. o2 ..".".. o3 ..".".. p4 .."; uci commit;")


	end
	
	return m
end
