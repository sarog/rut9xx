--[[
Teltonika R&D. ver 0.1
]]--

local fs = require "nixio.fs"
local fw = require "luci.model.firewall"
require("luci.fs")
require("luci.config")

local sys = require"luci.sys"
local util = require "luci.util"
local types = require "luci.cbi.datatypes"
require "teltonika_lua_functions"

local logDir, o, needReboot = false
local deathTrap = {}

local m, m2, s, s2, o

m = Map("system", translate("Administration Settings"),
	translate(""))

s2 = m:section(TypedSection, "system", translate("Router Name And Host Name"))
s2.addremove = false
s2.anonymous = true

routername = s2:option(Value, "routername", translate("Router name"), translate("Specifies router name, it will be seen in Status page"))
routername.datatype = "network"

function routername.write(self, section, value)
	m.uci:set("system", section, "routername", value)
	m.uci:save("system")
end

hostname = s2:option(Value, "hostname", translate("Host name"), translate("Specifies how router will be seen by other devices"))
hostname.datatype = "tlt_hostname"
hostname.rmempty = false

function hostname.write(self, section, value)
	m.uci:set("system", section, "hostname", value)
	m.uci:save("system")

	luci.sys.hostname(value)
end


s = m:section(TypedSection, "_dummy", translate("Administrator Password"), translate("Password requirements: Minimum 8 characters, at least one uppercase letter, one lowercase letter and one number."))
s.addremove = false
s.anonymous = true

pw0 = s:option(Value, "pw0", translate("Current password"), translate("Enter your current administration password"))
pw0.password = true

pw1 = s:option(Value, "pw1", translate("New password"), translate("Enter your new administration password. Password must contain a capital letter and a number. Minimum length is 8 symbols."))
pw1.password = true
pw1.datatype = "root_password_validate(8,32)"

pw2 = s:option(Value, "pw2", translate("Confirm new password"), translate("Re-enter your new administration password. Password must contain a capital letter and a number. Minimum length is 8 symbols."))
pw2.password = true
pw2.datatype = "root_password_validate(8,32)"

function s.cfgsections()
	return { "_pass" }
end

se = m:section(TypedSection, "system", translate("Language Settings"))
se.addremove = false
se.anonymous = true

o2 = se:option(ListValue, "lang", translate("Language"), translate("Website will be translated into selected language"))
o2:value("en","English")

local i18ndir = luci.i18n.i18ndir .. "base."
--  luci.http.prepare_content("application/json")
--  luci.http.write_json(luci.util.kspairs(luci.config.languages))
for k, v in luci.util.kspairs(luci.config.languages) do
	local file = i18ndir .. k:gsub("_", "-")
	if k:sub(1, 1) ~= "." and luci.fs.access(file .. ".lmo") then
		o2:value(k, v)
	end
end

function o2.cfgvalue(...)
	return m.uci:get("luci", "main", "lang")
end

function o2.write(self, section, value)
	m.uci:set("luci", "main", "lang", value)
	m.uci:save("luci")
	m.uci:commit("luci")
end

function m.parse(map)
	local v0 = pw0:formvalue("_pass")
	local v1 = pw1:formvalue("_pass")
	local v2 = pw2:formvalue("_pass")

	if v0 and v1 and v2 and #v0 > 0 and #v1 > 0 and #v2 > 0 then
		v0 = escape_shell(v0)
		local pw_good = (sys.call("/sbin/check_passwd.sh " .. v0) == 0)

		if pw_good == false then
			m.message = translate("err: Wrong current password, password not changed!")
		else
			if v1 == v2 then
				if types.root_password(v1) then
					if luci.sys.user.setpasswd(luci.dispatcher.context.authuser, v1) == 0 then
						m.message = translate("scs: Password successfully changed!")
						m.uci:set("teltonika", "sys", "pass_changed", "1")
						m.uci:save("teltonika")
						m.uci:commit("teltonika")
					else
						m.message = translate("err: Unknown error, password not changed!")
					end
				else
					m.message = translate("err: Password too weak, password not changed!")
				end
			else
				m.message = translate("err: Given password confirmation did not match, password not changed!")
			end
		end
	end

	if needReboot then
		m.message = translate("scs: You must reboot the router for the changes to take effect.")
	end

	Map.parse(map)
end

ipv = m:section(TypedSection, "ipv6", translate("IPv6 Support"))
ipv.addremove = false

ip = ipv:option(Flag, "enable", translate("Enable"), translate("Enable IPv6 support for all services"))
ip.rmempty = false
ip.default = '0'

function ip.write(self, section, value)
	local old_value=util.trim(sys.exec("uci get -q system.ipv6.enable"))
	if tonumber(value) ~= tonumber(old_value) then
		sys.call("uci set -q system.ipv6.enable="..tonumber(value))
		if tonumber(value)==1 then
			sys.call("sed -i 's/.*net.ipv6.conf.default.disable_ipv6.*/net.ipv6.conf.default.disable_ipv6=0/' /etc/sysctl.conf")
			sys.call("sed -i 's/.*net.ipv6.conf.all.disable_ipv6.*/net.ipv6.conf.all.disable_ipv6=0/' /etc/sysctl.conf")
			proto=util.trim(sys.exec("uci get -q network.wan.proto"))
			if proto=="dhcp" then
				sys.call("uci set -q network.wan6=interface; uci set -q network.wan6.ifname='@wan'; uci set -q network.wan6.proto='dhcpv6'; uci commit network")
			else
				sys.call("uci delete -q network.wan6; uci commit network")
			end
		else
			sys.call("uci delete -q dhcp.lan.enable_ra")
			sys.call("uci delete -q network.wan6; uci commit network")
			sys.call("sed -i 's/.*net.ipv6.conf.default.disable_ipv6.*/net.ipv6.conf.default.disable_ipv6=1/' /etc/sysctl.conf")
			sys.call("sed -i 's/.*net.ipv6.conf.all.disable_ipv6.*/net.ipv6.conf.all.disable_ipv6=1/' /etc/sysctl.conf")
		end
		sys.call("sysctl -p >/dev/null 2>/dev/null &")
	end
end

--m2 = Map("teltonika")
md = m:section(TypedSection, "system", translate("Login Page"))
md.addremove = false

if luci.tools.status.show_mobile() then
	o1 = md:option(Flag, "shw3g", translate("Show mobile info at login page"), translate("Show operator and signal strenght at login page"))
	o1.rmempty = false


	function o1.cfgvalue(...)
		return m.uci:get("teltonika", "sys", "shw3g")
	end

	function o1.write(self, section, value)
		m.uci:set("teltonika", "sys", "shw3g", value)
		m.uci:save("teltonika")
		m.uci:commit("teltonika")
	end
end
o2 = md:option(Flag, "showwan", translate("Show WAN IP at login page"), translate("Show WAN IP at login page"))
o2.rmempty = false

function o2.cfgvalue(...)
	return m.uci:get("teltonika", "sys", "showwan")
end

function o2.write(self, section, value)
	m.uci:set("teltonika", "sys", "showwan", value)
	m.uci:save("teltonika")
	m.uci:commit("teltonika")
end

mo = m:section(TypedSection, "leds", translate("LEDs Indication"))
mo.addremove = false

o = mo:option(Flag, "enable", translate("Enable"),
	translate("Enable signal strenght, LAN and connection status indication using LEDs"))
o.rmempty = false

s2 = m:section(TypedSection, "button", translate("Reset button configuration"))
s2.template = "cbi/tblsection"
s2.addremove = true
s2.anonymous = true
s2.defaults = { action = "released" }

o = s2:option(Flag, "enabled", translate("Enabled"))
o.default = o.enabled

o = s2:option(Value, "min", translate("Min time"),
	translate("Minimum time 0-60 (in seconds) that the button needs to be held to perform an action"))
o.datatype = "range(0, 60)"
o.placeholder = "0"
o.rmempty = false

local min_boundaries = {}
local max_boundaries = {}

function o.validate(self, value, section)
	table.insert(min_boundaries, tonumber(value))
	return value
end

o = s2:option(Value, "max", translate("Max time"),
	translate("Maximum time 1-60 (in seconds) that the button can be held to perform an action, after which no action will be performed."))
o.datatype = "range(1, 60)"
o.placeholder = "60"
o.rmempty = false

function o.validate(self, value, section)

	table.insert(max_boundaries, tonumber(value))

	local index = 1
	while index < #max_boundaries do
		local min_bound = tonumber(min_boundaries[index])
		local max_bound = tonumber(max_boundaries[index])
		local min_current_bound = tonumber(min_boundaries[#min_boundaries])
		local max_current_bound = tonumber(value)
		if max_current_bound <= min_current_bound then
			return nil, translatef("Invalid value. Max Time: %s. Max bound is lower or equal to lower bound.", value)
		end
		if  not ((min_current_bound <= min_bound and max_current_bound <= min_bound) or
			(min_current_bound >= max_bound and max_current_bound >= max_bound)) then
			return nil, translatef("Error: intervals overlap. [%d;%d], [%d;%d]", min_bound, max_bound, min_current_bound, max_current_bound)
		end
		index = index + 1
	end

	return value
end

o = s2:option(ListValue, "handler", translate("Action"),
	translate("The action to be performed when this rule is met"))
o:value("reboot", translate("Reboot"))
o:value("default", translate("User\'s defaults configuration"))
o:value("firstboot", translate("Factory defaults configuration"))

------lua kodas skirtas resetui ivykdyti
md = m:section(TypedSection, "system", translate("Restore Default Settings"))
md.addremove = false
md.anonymous = true
md.add_template = "admin_system/reset"
md.user_conf_avail = fs.access("/etc/default-config/config.tar.gz") and true or false

oldledcfg = nil
function m.on_before_save(self)
	oldledcfg = util.trim(luci.sys.exec("uci get -q system.@leds[0].enable"))
end
function m.on_after_save(self)
	local newledcfg = util.trim(luci.sys.exec("uci get -q system.@leds[0].enable"))
	if oldledcfg ~= newledcfg then
		luci.sys.exec("/etc/init.d/ledsman restart")
	end
end

return m
