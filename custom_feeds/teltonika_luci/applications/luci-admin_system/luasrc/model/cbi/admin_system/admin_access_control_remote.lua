require("luci.fs")
require("luci.config")
require "teltonika_lua_functions"

local utl = require "luci.util"
local nw = require "luci.model.network"
local sys = require "luci.sys"


m = Map("rms_connect_mqtt", translate("Remote Management System"), translate(""))

s = m:section(NamedSection, "rms_connect_mqtt", "rms_connect_mqtt", translate("RMS Settings"))
type_of_delay = s:option(ListValue, "enable", translate("Connection type"))
type_of_delay:value("1", translate("Enabled"))
type_of_delay:value("2", translate("Standby"))
type_of_delay:value("0", translate("Disabled"))
type_of_delay.default = 2
type_of_delay.template = "admin_system/netinfo_monitoring_connect"

function type_of_delay.write(self, section, value)
    local has_been_enabled = m.uci:get("rms_connect", "rms_connect", "enable")
    local has_been_enabled_mqtt = m.uci:get("rms_connect_mqtt", "rms_connect_mqtt", "enable")

    if value ~= has_been_enabled then
        m.uci:set("rms_connect", "rms_connect", "enable", value)
        m.uci:commit("rms_connect")
    end
    if value ~= has_been_enabled_mqtt then
        m.uci:set("rms_connect_mqtt", "rms_connect_mqtt", "enable", value)
        m.uci:commit("rms_connect_mqtt")

        luci.sys.call("/etc/init.d/rms_connect restart")
    end
end

o = s:option(Value, "remote", translate("Hostname"))
o.datatype = "or(hostname,ip4addr)"
o.default = "rms.teltonika.lt"

p = s:option(Value, "port", translate("Port"), translate("Port number."))
p.datatype = "port"
p.default = "15009"


s = m:section(NamedSection, "", "", translate(""));
s.template = "admin_system/netinfo_monitoring"

function m.on_after_commit(self)
    local is_enabled = m:formvalue("cbid.rms_connect_mqtt.rms_connect_mqtt.enable") or "0"
    if is_enabled == "2" then
        is_enabled = "1"
    end
    local remote = m:formvalue("cbid.rms_connect_mqtt.rms_connect_mqtt.remote") or "rms.teltonika.lt"
    luci.sys.call("touch /etc/config/rms_connect")
    luci.sys.call("uci set rms_connect.rms_connect=rms_connect")
    m.uci:set("rms_connect", "rms_connect", "retry", "60")
    m.uci:set("rms_connect", "rms_connect", "port", "15000")
    m.uci:set("rms_connect", "rms_connect", "enable", is_enabled)
    m.uci:save("rms_connect")
    luci.sys.call("uci set openvpn.teltonika_auth_service=teltonika_auth_service")
    m.uci:set("openvpn", "teltonika_auth_service", "enable", is_enabled)
    m.uci:set("openvpn", "teltonika_auth_service", "remote", remote)
    m.uci:save("openvpn")
end

return m
