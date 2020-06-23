local m ,s, s2, o
local ROLE_CLIENT = "client"
local ROLE_SERVER = "server"
local save = 0 --prevents two validations from occuring

function hex_to_string(str)
    return (str:gsub('..', function (cc)
        return string.char(tonumber(cc, 16))
    end))
end

local cfgvalue_dvalue = function(self, section)
	local value = DummyValue.cfgvalue(self, section)
	if self.map.uci:get("openvpn", section, "enable_custom") == "1" then
		return "Not available"
	end
	return value or "-"
end

local function count_instances(self)
	local servers, clients = 0, 0
	self.map.uci:foreach(self.config, self.sectiontype, function(s)
		if s._role then
			if s._role  == ROLE_CLIENT then
				clients = clients + 1
			elseif s._role == ROLE_SERVER then
				servers = servers + 1
			end
		end
	end)

	return clients, servers
end

m = Map("openvpn", translate("OpenVPN"))

s = m:section(TypedSection, "openvpn", translate("OpenVPN Configuration"))
s.addremove = true
s.anonymous = false
s.nosectionname = true
s.template = "cbi/tblsection"
s.template_addremove = "openvpn/vpn_add_rem"
s.novaluetext = translate("There are no openVPN configurations yet")
s.extedit = luci.dispatcher.build_url("admin", "services", "vpn", "openvpn-tlt", "%s")
s.defaults = {
	persist_key = 1,
	verb = 5,
	port = 1194,
	proto = "udp"
}
s.validate = function(_, value)
	local ignore = {teltonika_auth_service = true, teltonika_management_service = true }
	return not ignore[value] and value
end
s.create = function(self, section)
	local stat
	local role = luci.http.formvalue("cbid." .. self.config .. "." .. self.sectiontype .. ".role")

	if role then
		local full_name = role .. "_" .. section
		local clients, servers = count_instances(self)
		if save == 0 then
			if self:cfgvalue(full_name) then
				self.map.message = translatef("err: Name %s already exists.", full_name)
				return
			end
			if role == ROLE_CLIENT and clients > 4 then
				self.map.message = translatef("err: Maximum OpenVPN client count has been reached")
				return
			end
			if role == ROLE_SERVER and servers > 0 then
				self.map.message = translatef("err: Only one OpenVPN server instance is allowed")
				return
			end
		end
		if #section > 10 then
		m.message = "err: Name \'" .. section .. "\' is too long. Maximum 10 characters."
		return
		end
		self.defaults["_role"] = role
		if role == "server" then
			self.defaults["dev"] = "tun_s_" .. section
			self.defaults["keepalive"] = "10 120"
		elseif role == "client" then
			self.defaults["dev"] = "tun_c_" .. section
			self.defaults["nobind"] = 1
		end

		stat = AbstractSection.create(self, full_name)
		if stat then
			self.map.message = translate("scs:New OpenVPN instance was created successfully. Configure it now")
		end
	end
	save = 1
	return stat
end

s.remove = function(self, section)
	local res = TypedSection.remove(self, section)
	if res then
		self.map.uci:delete("overview","show","open_vpn_" .. section)
		self.map.uci:save("overview")
		self.map.uci:commit("overview")
	end

	return res
end

o = s:option( DummyValue, "_name", translate("Tunnel name"),
	translate("Name of the tunnel. Used for easier tunnels management purpose only"))
o.cfgvalue = function(self, section)
	local name_is_hexed = self.map:get(section, "name_is_hexed") or "0"

	return name_is_hexed == "1" and hex_to_string(section) or section
end

o = s:option( DummyValue, "dev", translate("TUN/TAP"),
	translate("Virtual VPN interface type"))
	o.cfgvalue = cfgvalue_dvalue

o = s:option( DummyValue, "proto", translate("Protocol"),
	translate("A transport protocol used for connection"))
	o.cfgvalue = cfgvalue_dvalue

o = s:option( DummyValue, "port", translate("Port"),
	translate("TCP or UDP port number used for connection"))
	o.cfgvalue = cfgvalue_dvalue

o = s:option(Flag, "enable", translate("Enable"), translate("Make a rule active/inactive"))
o.rmempty = false
o.write = function(self, section, value)
	local curr_value = Flag.cfgvalue(self, section) or self.disabled
	if value ~= curr_value then
		self.map.uci:delete(section, "usr_enable")
	end

	return Flag.write(self, section, value)
end

return m
