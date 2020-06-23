local dsp = require "luci.dispatcher"

local section_name

if arg[1] then
    section_name = arg[1]
else
    return nil
end

local vrrpd_map = Map("vrrpd")
    vrrpd_map.redirect = dsp.build_url("admin/services/vrrp")

local vrrpd_section = vrrpd_map:section( NamedSection, section_name, "vrrpd", translate("VRRP Configuration Settings"))
    vrrpd_section.anonymous = true
    vrrpd_section.addremove = false

o = vrrpd_section:option( Flag, "enabled", translate("Enable"), translate("Enable VRRP (Virtual Router Redundancy Protocol) for LAN"))
    o.rmempty = false
    o.default = "0"

v_mac = vrrpd_section:option( Flag, "virtual_mac", translate("Virtualize MAC"), translate("Allow VRRP to use virtualized MAC for LAN"))
    v_mac.rmempty = true
    v_mac.default = "0"

local v_id = vrrpd_section:option( Value, "virtual_id", translate("Virtual ID"), translate("Routers with same IDs will be grouped in the same VRRP (Virtual Router Redundancy Protocol) cluster, range [1 - 255]"))
    v_id.datatype = "range(1,255)"
    v_id.default = "1"

    function v_id.validate(self, value, section)
        local vid_exists = false
        vrrpd_map.uci:foreach(self.config, "vrrpd", function(vrrp)
            if vrrp.virtual_id == value and vrrp[".name"] ~= section_name then
                vid_exists = true
                vrrpd_map.message = translatef("err: Instance with the virtual ID already exists")
                return false
            end
        end)

        if not vid_exists then
            return value
        end
    end

prior = vrrpd_section:option( Value, "priority", translate("Priority"), translate("Router with highest priority value on the same VRRP (Virtual Router Redundancy Protocol) cluster will act as a master, range [1 - 255]"))
    prior.datatype = "range(1,255)"
    prior.default = "100"

delay = vrrpd_section:option( Value, "delay", translate("Advertisement Interval"), translate("Time interval in seconds between advertisements, range [1 - 255]"))
delay.datatype = "range(1,255)"
delay.default = "1"

local iface = vrrpd_section:option( ListValue, "interface", translate("Interface"), translate("Select which interface VRRP will operate on"))
    vrrpd_map.uci:foreach("network", "interface", function(intf)
        if intf.proto == "pppoe" or intf.proto == "static" or intf.proto == "dhcp" then

            if string.match(intf.ifname, "eth0") and not string.match(intf.ifname, "eth0%.") then
                iface:value(intf[".name"], translate("LAN ("..string.upper(intf._name or intf[".name"])..")"))
            elseif string.match(intf.ifname, "eth0%.") then
                iface:value(intf[".name"], translate("VLAN ("..string.upper(intf._name or intf[".name"])..")"))
            end

            if intf.ifname == "eth1" then
                iface:value(intf[".name"], translate("Wired WAN ("..string.upper(intf[".name"])..")"))
            end
        end
    end)

    function iface.validate(self, value, section)
        local intf_exists = false
        vrrpd_map.uci:foreach(self.config, "vrrpd", function(vrrp)
            if vrrp.interface == value and vrrp[".name"] ~= section_name then
                intf_exists = true
                vrrpd_map:error_head(translatef("Instance with the same operating interface already exists"))
                return false
            end
        end)

        if not intf_exists then
            return value
        end
    end

ip = vrrpd_section:option( DynamicList, "virtual_ip", translate("IP address"), translate("Virtual IP address(es) for interface\'s VRRP (Virtual Router Redundancy Protocol) cluster"))
    ip.datatype = "ip4addr"

--ip = vrrpd_section:option( DynamicList, "virtual_ip", translate("IP address"), translate("Virtual IP address(es) for LAN\\'s VRRP (Virtual Router Redundancy Protocol) cluster"))
--    ip.datatype = "ip4addr"
--
--s2 = m:section( NamedSection, "ping","vrrpd", translate("Check Internet Connection"))
--
--o = s2:option( Flag, "enabled", translate("Enable"), translate("Check to enable internet connection checking"))
--o.rmempty = false
--o.default = "0"
--
--host = s2:option( Value, "host", translate("Ping IP address"), translate("e.g. 192.168.1.1 (or www.host.com if DNS server configured correctly)"))
--
--interval = s2:option( Value, "interval", translate("Ping interval"), translate("Time interval in seconds between two pings"))
--interval.datatype = "integer"
--interval.default = "10"
--
--t_out = s2:option( Value, "time_out", translate("Ping timeout (sec)"), translate("Specify time to receive ping, range [1-9999]"))
--t_out.datatype = "integer"
--t_out.default = "1"
--
--size = s2:option( Value, "packet_size", translate("Ping packet size"), translate("Ping packet size, range [0-1000]"))
--size.datatype = "integer"
--
--retry = s2:option( Value, "retry", translate("Ping retry count"), translate("Number of time trying to send ping to a server after time interval if echo receive was unsuccessful, range [1-9999]"))
--retry.datatype = "integer"


return vrrpd_map
