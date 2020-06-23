local dsp = require "luci.dispatcher"

local section_name

if arg[1] then
	section_name = arg[1]
else
	luci.http.redirect(luci.dispatcher.build_url("admin", "network", "routes", "dynamic_routes"))
end

local peer_group_map = Map("quagga", translate("BGP configuration"))
peer_group_map.redirect = dsp.build_url("admin/network/routes/dynamic_routes/proto_bgp")

sect_peer = peer_group_map:section(NamedSection, section_name, "peer_group", "BGP peer-group "..section_name, "")
	sect_peer.anonymous = false

sect_peer:option(Flag, "enabled", "Enable", "Enable/Disable BGP peer")

o = sect_peer:option(Value, "as", "Remote AS", "")
    o.datatype = "integer"

o = sect_peer:option(DynamicList, "neighbor", "Neighbor address", "")
    o.datatype = "ip4addr"

o = sect_peer:option(Value, "adv_int", "Advertisement interval", "")
    o.datatype = "integer"

o = sect_peer:option(ListValue, "cl_config_type", "Neighbor configuration", "Configure a neighbor as Route Reflector or Route Server client", "")
    o:value("", "None")
    o:value("route-reflector-client", "Route Reflector client")
    o:value("route-server-client", "Route Server client")

sect_peer:option(Flag, "next_hop_self", "Disable next hop calculation", "Disable the next hop calculation for this group")

local nhs = sect_peer:option(Flag, "next_hop_self_all", "Apply also to ibgp-learned routes", "Apply also to ibgp-learned routes when acting as a route reflector")
    nhs:depends("next_hop_self", "1")

local nhs = sect_peer:option(Flag, "soft_rec_inbound", "Inbound soft-reconfiguration", "Allow inbound soft reconfiguration for this neighbor")

local con_check = sect_peer:option(Flag, "con_check", "Disable connected check", "One-hop away EBGP peer using loopback address")

return peer_group_map
