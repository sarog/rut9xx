local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()
local util = require ("luci.util")
local ntm = require "luci.model.network".init()

local section_name

if arg[1] then
	section_name = arg[1]
else
	luci.http.redirect(luci.dispatcher.build_url("admin", "network", "routes", "dynamic_routes", "proto_nhrp"))
end

nhrp_map = Map("quagga", translate("NHRP interface configuration"))

local nhrp_instance = nhrp_map:section(NamedSection, section_name, "nhrp_instance", translate("NHRP Parameters Configuration"))

local enabled = nhrp_instance:option(Flag, "enabled", translate("Enabled"), translate("Enables DMVPN client"))

local interface = nhrp_instance:option(ListValue, "interface", translate("Interface"), translate("Interface which will be using NHRP"))
local ifaces = ntm:get_interfaces()
for _, iface in ipairs(ifaces) do
    local net = iface:get_network()
    if net ~= nil and net:name() ~= nil and string.match(iface:name(), "^[a-zA-Z0-9%-%.].*") then
        interface:value(iface:name(), iface:name())
    end
end

local network_id = nhrp_instance:option(Value, "network_id", translate("Network ID"), translate("Network ID of NHRP"))
    network_id.datatype    = "min(1)"

local proto_address = nhrp_instance:option(Value, "proto_address", translate("NHS"), translate("IP address of Next-Hop Server"))
    proto_address:value("dynamic", translate("Dynamic"))

local nbma_address = nhrp_instance:option(Value, "nbma_address", translate("NBMA"), translate("Non-Broadcast Multi-Access(NBMA) network IP address"))


local nhrp_hldtm = nhrp_instance:option(Value, "holdtime", translate("Hold-time"), translate("Specifies the holding time for NHRP Registration Requests and Resolution Replies sent from this interface or shortcut-target. The holdtime is specified in seconds and defaults to two hours."))
    nhrp_hldtm.default = "7200"


local enabled_ipsec = nhrp_instance:option(Flag, "ipsec_support", translate("IPsec support"), translate("Use NHRP over IPsec"))

local ipsec_instance = nhrp_instance:option(ListValue, "ipsec_instance", translate("IPsec instance"), translate("Select IPsec instance name"))
    ipsec_instance:depends("ipsec_support", "1")
uci:foreach("strongswan", "conn", function (ipsec)
    if ipsec.enabled == "1" then
        ipsec_instance:value(ipsec[".name"], ipsec[".name"].." (Enabled)")
    else
        ipsec_instance:value(ipsec[".name"], ipsec[".name"].." (Disabled)")
    end
end)

local nhrp_map_instance = nhrp_map:section(TypedSection, section_name.."_map", translate("NHRP mappings configuration"))
nhrp_map_instance.addremove = true
nhrp_map_instance.anonymous = true
nhrp_map_instance.template = "cbi/tblsection"
nhrp_map_instance.novaluetext = translate("There are no NHRP map configurations yet")
nhrp_map_instance.defaults = {enabled = "0"}

local enabled = nhrp_map_instance:option(Flag, "enabled", translate("Enabled"), translate("Enables DMVPN client"))

local ip_addr = nhrp_map_instance:option(Value, "ip_addr", translate("IP address"), translate("Network ID of NHRP"))

local nbma = nhrp_map_instance:option(Value, "nbma", translate("NBMA"), translate("IP address of Next-Hop Server"))

function explode(delimiter, text)
    local text_arr, arr_length
    text_arr={}
    arr_length=0
    if not text or (#text == 1) then
        return {text}
    end
    while true do
        l = string.find(text, delimiter, arr_length, true)
        if l ~= nil then
            table.insert(text_arr, string.sub(text, arr_length, l - 1))
            arr_length = l + 1
        else
            table.insert(text_arr, string.sub(text, arr_length))
            break
        end
    end
    return text_arr
end

function nhrp_map.on_before_save(self)
    local selected_interface = nhrp_map:get(section_name, "interface")
    if selected_interface and string.match(selected_interface, "(.*)-") == "gre4" then
        local nhrp_interface = ""
		local data = { ipaddrs = { } }
        local interface_name = string.match(selected_interface, "-(.*)")
        local local_hub_address = nhrp_map.uci:get("network", interface_name, "ipaddr")

        if local_hub_address then
            for _, interface in ipairs(ntm.get_interfaces()) do
                for i, a in ipairs(interface:ipaddrs()) do
                    data.ipaddrs[#data.ipaddrs+1] = {
                        addr      = a:host():string()
                    }
                    if (data.ipaddrs[#data.ipaddrs].addr == local_hub_address) then
                        nhrp_interface = interface.ifname
                    end
                end
            end
        else
            local tunnellink_name_selected = nhrp_map.uci:get("network", interface_name, "tunlink")

            if tunnellink_name_selected then
                local exploded = explode("_", tunnellink_name_selected)
                local newtunlink = exploded[1]

                for _, ex in ipairs(exploded) do
                    if nhrp_map.uci:get("network", newtunlink, "proto") then
                        tunnellink_name_selected = newtunlink
                        break
                    else
                        newtunlink = newtunlink .. "_".. ex
                    end
                end

                nhrp_interface = uci:get("network", tunnellink_name_selected, "ifname")
            end
        end


        if nhrp_interface ~= "" then
            nhrp_map:set(section_name, "tunnel_source", nhrp_interface)
        end
    end
end


return nhrp_map