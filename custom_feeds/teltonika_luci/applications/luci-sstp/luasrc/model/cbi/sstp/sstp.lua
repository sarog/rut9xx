local uci = require "luci.model.uci".cursor()
local utl = require ("luci.util")
local sys = require("luci.sys")

local m, s, o

m = Map("network", translate("SSTP"))
m.redirect = luci.dispatcher.build_url("admin/services/vpn/sstp/")

s = m:section(TypedSection, "interface", translate("SSTP Configuration"))
s.field_length = 10
s.addremove = true
s.nosectionname = true
s.extedit = luci.dispatcher.build_url("admin", "services", "vpn", "sstp", "%s")
s.template = "cbi/tblsection"
s:depends("proto", "sstp") -- Only show those with "gre"
s.defaults.proto = "sstp"
s.novaluetext = translate("There are no SSTP Tunnel configurations yet")

local name = s:option( DummyValue, "name", translate("Tunnel name"), translate("Name of the tunnel. Used for easier tunnels management purpose only"))
function name.cfgvalue(self, section)
	return section or "Unknown"
end

function s.create(self, section)
	if string.len(section) > 10 then
		m.message = "err: Name \'" .. section .. "\' is too long. Maximum 10 characters."
	else
		return TypedSection.create(self, section)
	end
end

o = s:option(Flag, "enabled", "Enabled") -- Creates an element list (select box)
o.rmempty = false

function m.on_commit(map)
	clients = string.gsub(utl.trim(sys.exec("cat /etc/config/network | grep 'sstp_name' | awk '{print $3}'")),"'","")
	clients = clients:split("\n")
	for i = 1, #clients do
		VPN_INST = clients[i]
		local sstpEnable = m:formvalue("cbid.network." .. VPN_INST .. ".enabled")
		if sstpEnable then
			sys.call("ifup " .. VPN_INST .. " > /dev/null")
		else
			sys.call("ifdown " .. VPN_INST .. " > /dev/null")
		end
	end
end

return m
