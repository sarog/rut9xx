local fs  = require "nixio.fs"
local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()
local utl = require ("luci.util")

local function debug(string)
	sys.call("logger " .. string)
end

local m2, m ,s, s2, o
local save = 0 --prevents two validations from occuring

function string:split( inSplitPattern, outResults )
	if not outResults then
		outResults = { }
	end
	local theStart = 1
	local theSplitStart, theSplitEnd = string.find( self, inSplitPattern, theStart )
	while theSplitStart do
		table.insert( outResults, string.sub( self, theStart, theSplitStart-1 ) )
		theStart = theSplitEnd + 1
		theSplitStart, theSplitEnd = string.find( self, inSplitPattern, theStart )
	end
	table.insert( outResults, string.sub( self, theStart ) )
	return outResults
end

m = Map("pptpd", translate("PPTP"))
m.spec_dir = nil
--m.pageaction = false

s = m:section( TypedSection, "service", translate("PPTP Configuration"), translate("") )
s.addremove = true
s.anonymous = true
s.template = "pptp/tblsection_pptp"
s.sectionhead = "Tunnel name"
s.addremoveAdd = true

s.extedit = luci.dispatcher.build_url("admin", "services", "vpn", "pptp", "s_%s")

local name = s:option( DummyValue, "_name", translate("Tunnel name"), translate("Name of the PPTPD configuration. Used for easier PPTPD configurations management purpose only"))
name.rawhtml = true
name.width   = "25%"

local ptype = s:option( DummyValue, "_type", translate("Type"), translate("A role that the PPTPD configuration is in"))
ptype.rawhtml = true
ptype.width   = "25%"
function ptype.cfgvalue(self, section)
	return "Server"
end


--local status = s:option( DummyValue, "enabled", translate("Enabled"), translate("Indicates whether a configuration is active or not"))
status = s:option(Flag, "enabled", translate("Enable"), translate("Make a rule active/inactive"))
status.rawhtml = true
status.width   = "25%"

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
	m.uci:foreach("firewall", "zone", function(s)
		if s.name == "vpn" then
			if form_value == "1" then
				m.uci:set("firewall", s[".name"], "device", "tun+ gre+ pptp+")
			else
				m.uci:set("firewall", s[".name"], "device", "tun+ gre+")
			end
		end
	end)

	m.uci:save("firewall")
	m.uci:commit("firewall")
end

--[[
function status.cfgvalue(self, section)
	local val = AbstractValue.cfgvalue(self, section)
	if val == "1" then
		return translate("Yes")
	else
		return translate("No")
	end
end
--]]

m2 = Map("network", "")
m2.spec_dir = nil
--m2.pageaction = false

s = m2:section( TypedSection, "interface", "", "" )
s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection_list2"
s.template_addremove = "openvpn/add_rem"
s.addremoveAdd = true
s.number = utl.trim(sys.exec("uci -q show pptpd | grep -c service"))

s.extedit = luci.dispatcher.build_url("admin", "services", "vpn", "pptp", "c_%s")

local name = s:option( DummyValue, "_name", translate("Name"), translate("Name of the PPTPD configuration. Used for easier PPTPD configurations management purpose only"))

local ptype = s:option( DummyValue, "dev", translate("Type"), translate("A role that the PPTPD configuration is in"))
function ptype.cfgvalue(self, section)
	return "Client"
end


status = s:option(Flag, "enabled", translate("Enable"), translate("Make a rule active/inactive"))
--[[
local status = s:option( DummyValue, "enabled", translate("Enabled"), translate("Indicates whether a configuration is active or not"))

function status.cfgvalue(self, section)
	local val = AbstractValue.cfgvalue(self, section)
	if val == "1" then
		return translate("Yes")
	else
		return translate("No")
	end
end
--]]

function s.validate(self, value)
	proto = utl.trim(sys.exec("uci get network." .. value .. ".proto")) or "0"
	if proto ~= "pptp" then
		return nil
	end
	return value
end

function s.parse(self, section)
	local cfgname = luci.http.formvalue("cbid." .. self.config .. "." .. self.sectiontype .. ".name")
	local delButtonFormString = "cbi.rts." .. self.config .. "."
	local delButtonPress
	local configName


	m.uci:foreach("network", "interface", function(x)
		configName = x[".name"] or ""
		if luci.http.formvalue(delButtonFormString .. configName) then
			delButtonPress = true
			uci:delete(self.config, configName)
			uci:commit("network")
		end
	end)

	if luci.http.formvalue("cbi.rts.pptpd.pptpd") then
		delButtonPress = true
		local changes = false
		uci:foreach("firewall", "rule", function(x)
			if x._name == "pptpd" then
				m.uci:set("firewall", x[".name"], "enabled", "0")
				changes = true
			end
		end)

		m.uci:foreach("firewall", "zone", function(s)
			if s.name == "vpn" then
				m.uci:set("firewall", s[".name"], "device", "tun+ gre+")
			end
		end)
		m.uci:delete("pptpd", "pptpd")
		m.uci:commit("pptpd")
		m.uci:save("firewall")
		m.uci:commit("firewall")

		if changes then
			sys.call("/etc/init.d/firewall reload >/dev/null 2>/dev/null")
		end
	end

	if delButtonPress then
		sys.call("/etc/init.d/pptpd restart >/dev/null 2>/dev/null")
		sys.call("/etc/init.d/network restart >/dev/null 2>/dev/null")
	elseif cfgname and cfgname ~= '' then
		openvpn_new(self, cfgname)
	end

	TypedSection.parse( self, section )
end

function m2.on_commit(map)
	clients = string.gsub(utl.trim(sys.exec("cat /etc/config/network | grep '_name' | awk '{print $3}'")),"'","")
	clients = clients:split("\n")
	for i = 1, #clients do
		VPN_INST = clients[i]
		local pptpEnable = m:formvalue("cbid.network." .. VPN_INST .. ".enabled")
		if pptpEnable then
			sys.call("ifup " .. VPN_INST .. " > /dev/null")
		else
			sys.call("ifdown " .. VPN_INST .. " > /dev/null")
		end

		m.uci:foreach("firewall", "zone", function(s)
			if s.name == "vpn" then
				if pptpEnable == "1" then
					m.uci:set("firewall", s[".name"], "device", "tun+ gre+ pptp+")
				else
					m.uci:set("firewall", s[".name"], "device", "tun+ gre+")
				end
			end
		end)
	end
	m.uci:save("firewall")
	m.uci:commit("firewall")
end

function openvpn_new(self,name)
	local exist = false
	local count = 0
	local t = {}
	local role = luci.http.formvalue("cbid." .. self.config .. "." .. self.sectiontype .. ".role"
	)

	if name and #name > 0 and role then
		if not (string.find(name, "[%c?%p?%s?]+") == nil) then
			m.message = translate("err: Only alphanumeric characters are allowed.")
		elseif #name > 10 then
			m.message = "err: Name \'" .. name .. "\' is too long. Maximum 10 characters."
		else
			if role == "server" then
				local number = utl.trim(sys.exec("uci -q show pptpd | grep -c service"))
				if number == "0" then
					t["_name"] = name
					t["enabled"] = "0"
					t["localip"] = "192.168.0.1"
					t["remoteip"] = "192.168.0.20-30"
					uci:section("pptpd", "service", "pptpd", t)
					uci:save("pptpd")
					m2.message = translate("scs: New PPTP server instance created successfully. Configure it now")
				else
					m2.message = translate("Only one PPTP server instance is allowed.")
				end
			elseif role == "client" then
				uci:foreach("network", "interface", function(x)
					if x["proto"] == "pptp" then
						count = count + 1
					end
					if x[".name"] == name then
						exist = true
						if save == 0 then
							m2.message = translate("err: Can't create new PPTP client instance with the same name.")
						end
					end
				end)
				if count >= 5 then
					exist = true
					m2.message = translate("err: Maximum PPTP client instance count has been reached")
				else
					if not exist then
						t["_name"] = name
						t["enabled"] = "0"
						t["proto"] = "pptp"
						t["buffering"] = "1"
						uci:section("network", "interface", name,t)
						uci:save("network")
						m2.message = translate("scs: New PPTP client instance created successfully. Configure it now")
					end
				end
			end

		end
	else
		m2.message = translate("To create a new PPTP instance it's name has to be entered!")
	end
	save = 1
end


return m, m2
