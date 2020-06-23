local http = require "luci.http"
local sys = require("luci.sys")

local VPN_INST

if arg[1] then
	VPN_INST = arg[1]
else
	return nil
end

m = Map("network", translatef("SSTP Instance: %s", VPN_INST:gsub("^%l", string.upper)), "")

m.redirect = luci.dispatcher.build_url("admin/services/vpn/sstp/")

--~ SSTP PROTOCOL INTERFACE SECTION
s = m:section( NamedSection, arg[1], "interface", translate("Main Settings"), "")

FileUpload.size = "1000000"
FileUpload.sizetext = translate("Selected file is too large, max 1 MB")
FileUpload.sizetextempty = translate("Selected file is empty")

o = s:option(Flag, "enabled", translate("Enabled"), translate("Enable SSTP "..VPN_INST.."tunnel"))
o.rmempty = false

--function o.write(self, section, value)
--	self.map:set(section, self.option, value)
--	self.map:set(section, "defaultroute", "1")
--end

o = s:option(Flag, "defaultroute", translate("Use as default gateway"))
o.rmempty = false

dummy = s:option(DummyValue, "getinfo_ip_source_status", translate(""))
dummy.default = translate("Use this option when multiwan is off")

o = s:option(Value, "server", translate("Server IP address"), translate("IP address of the remote SSTP server."))

o = s:option(Value, "username", translate("User name"), translate(""))

o = s:option(Value, "password", translate("Password"), translate("Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:;[]"))
o.password = true
o.datatype = "password"

o = s:option(FileUpload, "ca", translate("CA cert"), translate("Upload CA certificate in PEM format"))


function m.on_commit(map)
	m.uci:set("network", VPN_INST, "sstp_name", VPN_INST)
	local sstpEnable = m:formvalue("cbid.network." .. VPN_INST .. ".enabled")
	if sstpEnable and sstpEnable == "1" then
		sys.call("ifup " .. VPN_INST .. " > /dev/null")
	else
		sys.call("ifdown " .. VPN_INST .. " > /dev/null")
	end
end

return m
