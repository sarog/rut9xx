require("luci.fs")
require("luci.config")
require "teltonika_lua_functions"

local utl = require "luci.util"
local nw = require "luci.model.network"
local sys = require "luci.sys"


m = Map("rms_connect_mqtt", translate("Remote Management System"), translate(""))
m.wizStep = 99


s = m:section(NamedSection, "rms_connect_mqtt", "rms_connect_mqtt", translate("RMS Settings"))

--s = m:section(NamedSection, "rms_connect_delay", "rms_connect_delay", translate("Remote Management System"))

type_of_delay = s:option(ListValue, "enable", translate("Connection type"))
type_of_delay:value("1", translate("Enabled"))
type_of_delay:value("2", translate("Standby"))
type_of_delay:value("0", translate("Disabled"))
type_of_delay.default = 2
type_of_delay.template = "cfgwzd-module/rms_state_lvalue"

function type_of_delay.write(self, section, value)
	local has_been_enabled = m.uci:get("rms_connect", "rms_connect", "enable")
	local vpn_value = value
	if vpn_value == "2" then
		vpn_value = "1"
	end
	local has_been_enabled_mqtt = m.uci:get("rms_connect_mqtt", "rms_connect_mqtt", "enable")

	if vpn_value ~= has_been_enabled and value ~= has_been_enabled_mqtt then
		m.uci:set("rms_connect_mqtt", "rms_connect_mqtt", "enable", value)
		m.uci:commit("rms_connect_mqtt")

		m.uci:set("rms_connect", "rms_connect", "enable", vpn_value)
		m.uci:commit("rms_connect")

		luci.sys.call("/etc/init.d/rms_connect restart")
	end
end

s = m:section(NamedSection, "", "", translate(""));
s.template = "cfgwzd-module/monitoring"


if m:formvalue("cbi.wizard.finish") then
	luci.http.redirect(luci.dispatcher.build_url("/admin/status/overview"))
end

if m:formvalue("cbi.wizard.skip") then
	luci.http.redirect(luci.dispatcher.build_url("/admin/status/overview"))
end

return m
