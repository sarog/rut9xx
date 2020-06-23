--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: ddns.lua 6588 2010-11-29 15:14:50Z jow $
]]--
local dsp  = require "luci.dispatcher"
local utl  = require "luci.util"
local sys  = require "luci.sys"

local NX   = require "nixio"
local NXFS = require "nixio.fs"
local WADM = require "luci.tools.webadmin"
local DTYP = require "luci.cbi.datatypes"
local DDNS = require "luci.tools.ddns"		-- ddns multiused functions
local CTRL = require "luci.controller.ddns"	-- this application's controller

local has_ssl    = DDNS.check_ssl()	-- HTTPS support

local font_red	= "<font color='red'>"
local font_off	= "</font>"
local bold_on	= "<strong>"
local bold_off	= "</strong>"

local DNS_INST
		
if arg[1] then
	DNS_INST = arg[1]
else
	--print("[Openvpn.cbasic] Fatal Err: Pass openvpn instance failed")
	--Shoud redirect back to overview
	return nil
end

local section	= arg[1]

local function _verify_ip_source()
	-- section is globally defined here be calling agrument (see above)
	local _arg

	local _source = false
			or  src4:formvalue(section)

	local command = CTRL.luci_helper .. [[ -]]

	if _source == "network" then
		_arg = false
				or  ipn4:formvalue(section)
		command = command .. [[n ]] .. _arg
	elseif _source == "web" then
		_arg = false
				or  iurl4:formvalue(section)
		command = command .. [[u ]] .. _arg

		-- proxy only needed for checking url
		_arg = (pxy) and pxy:formvalue(section) or ""
		if (_arg and #_arg > 0) then
			command = command .. [[ -p ]] .. _arg
		end
	elseif _source == "interface" then
		return true
	elseif _source == "script" then
		command = command .. [[s ]] .. ips:formvalue(section)
	end
	command = command .. [[ -- get_local_ip]]
	local res_code = sys.call(command)
	return (res_code == 0 or res_code == 143)
end

m = Map("ddns", translate("Dynamic DNS"),
	translate("Dynamic DNS allows you to reach your router using a fixed hostname while having a dynamically changing IP address."))
m:chain("network");

m.redirect = dsp.build_url("admin/services/ddns")

s = m:section(NamedSection, DNS_INST, "service",  translate("DDNS"))
s.addremove = false
s.anonymous = false
s.addtitle = translate("Name")

s:option(Flag, "enabled", translate("Enable"), translate("Enable current configuration"))

if has_ssl or ( ( m:get(section, "use_https") or "0" ) == "1" ) then
	https = s:option(Flag, "use_https",
		translate("Use HTTP Secure"), translate("Configure Root CA certificates in System->\nAdministration->Root CA"))
	https.orientation = "horizontal"
	https.rmempty = false -- force validate function
	function https.cfgvalue(self, section)
		local value = AbstractValue.cfgvalue(self, section)
		if not has_ssl and value == "1" then
			self.description = bold_on .. font_red ..
					translate("HTTPS not supported") .. font_off .. "<br />" ..
					translate("please disable") .. " !" .. bold_off
		else
			self.description = translate("Enable secure communication with DDNS provider")
		end
		return value
	end
	function https.parse(self, section)
		DDNS.flag_parse(self, section)
	end
	function https.validate(self, value)
		if (value == "1" and has_ssl ) or value == "0" then return value end
		return nil, err_tab_basic(self) .. translate("HTTPS not supported") .. " !"
	end
	function https.write(self, section, value)
		if value == "1" then
			m:set(section, "cacert", "/etc/cacert.pem")
			return self.map:set(section, self.option, value)
		else
			self.map:del(section, "cacert")
			return self.map:del(section, self.option)
		end
	end
end

h_dummy = s:option(DummyValue, "getinfo_use_https_status", translate(""))
h_dummy:depends("use_https", "1")
h_dummy.default = translate("Configure Root CA certificates in System->Administration->Root CA")

state = s:option(Label, "state", translate("Status"), translate("Timestamp of the last IP check or update"))

function state.cfgvalue(self, section)
	local val = sys.exec("cat /tmp/ddns_status_" .. section)
	if val and val ~= "\n" and val ~= "" then
		return val
	else
		return "N/A"
	end
end

svc = s:option(ListValue, "service_name", translate("Service"), translate("Your dynamic DNS service provider"))
svc.rmempty = false

local services = { }
local fd = io.open("/etc/ddns/services", "r")
if fd then
	local ln
	repeat
		ln = fd:read("*l")
		local s = ln and ln:match('^%s*"([^"]+)"')
		if s then services[#services+1] = s end
	until not ln
	fd:close()
end

local v
for _, v in luci.util.vspairs(services) do
	svc:value(v)
end

function svc.cfgvalue(...)
	local v = Value.cfgvalue(...)
	if not v or #v == 0 then
		return "-"
	else
		return v
	end
end

function svc.write(self, section, value)
	if value == "-" then
		m.uci:delete("ddns", section, self.option)
	else
		Value.write(self, section, value)
	end
end

svc:value("-", "-- "..translate("custom").." --")


url = s:option(Value, "update_url", translate("Custom update URL"), translate("Custom hostname and the update URL"))
url:depends("service_name", "-")
url.rmempty = true

s:option(Value, "lookup_host", translate("Lookup host"), translate("FQDN of ONE of your defined host at DDNS provider, required to verify what the current IP at DNS using nslookup/host command"))
s:option(Value, "domain", translate("Hostname"), translate("Domain name which will be linked with dynamic IP address")).rmempty = true
s:option(Value, "username", translate("User name"), translate("Name of the user account")).rmempty = true
pw = s:option(Value, "password", translate("Password"), translate("Password of the user account. Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:;[]"))
pw.rmempty = true
pw.password = true
pw.datatype = "password"

require("luci.tools.webadmin")

src4 = s:option(ListValue, "ipv4_source",
	translate("IP address source"),
	translate("Defines the source to read systems IPv4-Address from, that will be send to the DDNS provider") )
src4.default = "network"
src4:value("network", translate("Custom"))
src4:value("web", translate("Public"))
src4:value("interface", translate("Private"))
src4:value("script", translate("Script"))
function src4.cfgvalue(self, section)
	return DDNS.read_value(self, section, "ip_source")
end
function src4.validate(self, value)
	if not _verify_ip_source() then
		return nil, translate("can not detect local IP. Please select a different Source combination")
	else
		return value
	end
end
function src4.write(self, section, value)
	if value == "network" then
		self.map:del(section, "ip_url")		-- delete not need parameters
		self.map:del(section, "ip_interface")
		self.map:del(section, "ip_script")
	elseif value == "web" then
		self.map:del(section, "ip_network")	-- delete not need parameters
		self.map:del(section, "ip_interface")
		self.map:del(section, "ip_script")
		self.map:del(section, "interface")
	elseif value == "interface" then
		self.map:del(section, "ip_network")	-- delete not need parameters
		self.map:del(section, "ip_url")
		self.map:del(section, "ip_script")
		self.map:del(section, "interface")
	elseif value == "script" then
		self.map:del(section, "ip_network")
		self.map:del(section, "ip_url")		-- delete not need parameters
		self.map:del(section, "ip_interface")
	end
	self.map:del(section, self.option)		 -- delete "ipv4_source" helper
	return self.map:set(section, "ip_source", value) -- and write "ip_source
end

dummy = s:option(DummyValue, "getinfo_ip_source_status", translate(""))
dummy.default = translate("Public, Private, Custom or Script IP source setting, will disable DNS rebinding protection")

-- IPv4 - ip_network (default "wan") -- ########################################
ipn4 = s:option(ListValue, "ipv4_network",
	translate("Network"),
	translate("Defines the network to read systems IPv4-Address from") )
ipn4:depends("ipv4_source", "network")
ipn4.default = "wan"
WADM.cbi_add_networks(ipn4)
function ipn4.cfgvalue(self, section)
	return DDNS.read_value(self, section, "ip_network")
end
function ipn4.validate(self, value)
	if src4:formvalue(section) ~= "network" then
		-- ignore if IPv6 selected OR
		-- ignore everything except "network"
		return ""
	else
		return value
	end
end
function ipn4.write(self, section, value)
	if src4:formvalue(section) ~= "network" then
		-- ignore if IPv6 selected OR
		-- ignore everything except "network"
		return true
	else
		-- set also as "interface" for monitoring events changes/hot-plug
		self.map:set(section, "interface", value)
		self.map:del(section, self.option)		  -- delete "ipv4_network" helper
		return self.map:set(section, "ip_network", value) -- and write "ip_network"
	end
end

-- IPv4 - ip_url (default "checkip.dyndns.com") -- #############################
iurl4 = s:option(Value, "ipv4_url",
	translate("URL to detect"),
	translate("Defines the Web page to read systems IPv4-Address from") )
iurl4:depends("ipv4_source", "web")
iurl4.default = "http://checkip.dyndns.com"
function iurl4.cfgvalue(self, section)
	return DDNS.read_value(self, section, "ip_url")
end
function iurl4.validate(self, value)
	if src4:formvalue(section) ~= "web" then
		-- ignore if IPv6 selected OR
		-- ignore everything except "web"
		return ""
	elseif not value or #value == 0 then
		return nil, translate("missing / required")
	end

	local url = DDNS.parse_url(value)
	if not (url.scheme == "http" or url.scheme == "https") then
		return nil, translate("must start with 'http://'")
	elseif not url.host then
		return nil, "<HOST> " .. translate("missing / required")
	elseif sys.call([[nslookup ]] .. url.host .. [[>/dev/null 2>&1]]) ~= 0 then
		return nil, translate("can not resolve host: ") .. url.host
	else
		return value
	end
end
function iurl4.write(self, section, value)
	if src4:formvalue(section) ~= "web" then
		-- ignore if IPv6 selected OR
		-- ignore everything except "web"
		return true
	else
		self.map:del(section, self.option)		-- delete "ipv4_url" helper
		return self.map:set(section, "ip_url", value)	-- and write "ip_url"
	end
end

-- IPv4 + IPv6 - ip_script (NEW) -- ############################################
ips = s:option(Value, "ip_script",
	translate("Script"),
	translate("User defined script to read systems IP-Address") )
ips:depends("ipv4_source", "script")	-- IPv4
ips.rmempty	= false
ips.placeholder = "/path/to/script.sh"
function ips.validate(self, value)
	local split
	if value then split = utl.split(value, " ") end

	if src4:formvalue(section) ~= "script" then
		return ""
	elseif not value or not (#value > 0) or not NXFS.access(split[1], "x") then
		return nil, translate("not found or not executable - Sample: '/path/to/script.sh'")
	else
		return value
	end
end
function ips.write(self, section, value)
	if src4:formvalue(section) ~= "script" then
		return true
	else
		return self.map:set(section, self.option, value)
	end
end

-- IPv4 - interface - default "wan" -- #########################################
-- event network to monitor changes/hotplug/dynamic_dns_updater.sh
-- only needs to be set if "script"
-- if "ip_source"="network" or "interface" we use their network
eif4 = s:option(ListValue, "ipv4_interface",
	translate("Event Network"),
	translate("Network on which the ddns-updater scripts will be started") )
eif4:depends("ipv4_source", "script")
eif4.default = "wan"
WADM.cbi_add_networks(eif4)
function eif4.cfgvalue(self, section)
	return DDNS.read_value(self, section, "interface")
end
function eif4.validate(self, value)
	if src4:formvalue(section) == "network"
			or src4:formvalue(section) == "interface" then
		return ""	-- ignore IPv6, network, interface
	else
		return value
	end
end
function eif4.write(self, section, value)
	if src4:formvalue(section) == "network"
			or src4:formvalue(section) == "interface" then
		return true	-- ignore IPv6, network, interface
	else
		self.map:del(section, self.option)		 -- delete "ipv4_interface" helper
		return self.map:set(section, "interface", value) -- and write "interface"
	end
end

ch_int = s:option(Value, "check_interval",
	translate("IP renew interval"), translate("Time interval to check if the IP address of the device has changed. Range [5 - 600000]"))
ch_int.default = 10
ch_int.datatype ="range(5,600000)"
ch_int.displayInline = true

check_unit = s:option(ListValue, "check_unit",
	translate("IP renew interval unit"),
	translate("Defines unit in which to check for IP renew") )
check_unit:value("minutes", translate("Minutes"))
check_unit:value("hours", translate("Hours"))
check_unit:value("days", translate("Days"))
check_unit.default = "minutes"
check_unit.displayInline = true

f_int = s:option(Value, "force_interval", translate("Force IP renew"), translate("Time interval to force IP address renewal. Range [5 - 600000]. NOTE: Must be greater than Check interval"))
f_int.default = 72
f_int.datatype = "range(5,600000)"
f_int.displayInline = true

function f_int.validate(self, value)
	local check_int = DDNS.calc_seconds(luci.http.formvalue("cbid.ddns." .. DNS_INST .. ".check_interval"),
											luci.http.formvalue("cbid.ddns." .. DNS_INST .. ".check_unit"))
	local force_int = DDNS.calc_seconds(luci.http.formvalue("cbid.ddns." .. DNS_INST .. ".force_interval"),
											luci.http.formvalue("cbid.ddns." .. DNS_INST .. ".force_unit"))
	if tonumber(check_int) >= tonumber(force_int) then
		m.message = "err:Check interval is bigger than force check interval"
		return false
	end
	return value
end

force_unit = s:option(ListValue, "force_unit",
	translate("Force IP renew unit"),
	translate("Defines unit in which to force IP renew") )
force_unit:value("minutes", translate("Minutes"))
force_unit:value("hours", translate("Hours"))
force_unit:value("days", translate("Days"))
force_unit.default = "minutes"
force_unit.displayInline = true

function m.on_commit()

	--set dnsmasq rebind protection for private IPs 
	--checking if atleast one enabled ddns needs to work with private IPs
	local private_ip_possible = "0"
	local usr_enable = "0"
	
	m.uci:foreach("ddns", "service", function(s)
		usr_enable = s.enabled or "0"
		if usr_enable == "1" then
			private_ip_possible = "1"
		end
	end)
	
	if private_ip_possible == "1" then
		sys.exec("uci -q set dhcp.@dnsmasq[0].rebind_protection=0")
	else
		sys.exec("uci -q set dhcp.@dnsmasq[0].rebind_protection=1")
	end

	sys.exec("uci -q commit dhcp")

	--Delete all usr_enable from ddns config
	local dnsEnable = m:formvalue("cbid.ddns." .. DNS_INST .. ".enabled") or "0"
	if dnsEnable ~= dns_enable then
		m.uci:foreach("ddns", "service", function(s)
			local usr_enable = s.usr_enable or ""
			dns_inst2 = s[".name"] or ""
			if usr_enable == "1" then
				m.uci:delete("ddns", dns_inst2, "usr_enable")
			end
		end)
	end
	m.uci:save("ddns")
	m.uci.commit("ddns")
end

return m
