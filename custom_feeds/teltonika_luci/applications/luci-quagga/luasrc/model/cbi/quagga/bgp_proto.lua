require("luci.sys")
local dsp = require "luci.dispatcher"

m=Map("quagga", "BGP protocol's configuration")

FileUpload.size = "1000000"
FileUpload.sizetext = translate("Selected file is too large, max 1 MB")
FileUpload.sizetextempty = translate("Selected file is empty")

-- Section BGP Instances:
sect_general = m:section(NamedSection, "general", "general", "General Settings", "")

sect_general:option(Flag, "enabled", "Enable", "Enable/Disable BGP protocol")
sect_general:option(Flag, "enabled_vty", "Enable vty", "Enable/Disable vty access from LAN")
sect_general:option(FileUpload, "bgpd_custom_conf", "Import config", "Use imported BGP configuration")

sect_instances = m:section(TypedSection, "instance", "BGP Instance", "Configuration of the BGP protocol instance")
	sect_instances.anonymous = true
	sect_instances.addremove = false
	--sect_instances.template = "cbi/tblsection"
	sect_instances.novaluetext = "There are no BGP instances created yet."

enabled = sect_instances:option(Flag, "enabled", "Enable", "Enable/Disable BGP instance")

o = sect_instances:option(Value, "as", "AS", "")
	o.datatype = "integer"

sect_instances:option(Value, "id", "BGP router ID", "")
sect_instances:option(DynamicList, "network", "Network", "Add the announcement network")

local redis = sect_instances:option(DynamicList, "redistribute", "Redistribution options", "")
	redis:value("connected", translate("Connected routes"))
	redis:value("kernel", translate("Kernel added routes"))
	redis:value("nhrp", translate("NHRP routes"))
	redis:value("ospf", translate("OSPF routes"))
	redis:value("static", translate("Static routes"))

local deter = sect_instances:option(Flag, "deterministic_med", "Deterministic MED", "Compare MED between same AS ignoring their age")

sect_peers = m:section(TypedSection, "peer", "BGP peers", "")
	sect_peers.addremove = true
	sect_peers.anonymous = true
	sect_peers.template = "cbi/tblsection"
	sect_peers.novaluetext = "There are no BGP peers created yet."
	sect_peers.extedit   = dsp.build_url("admin/network/routes/dynamic_routes/bgp_peer/%s")
	sect_peers.defaults.instance = "default"

	sect_peers:option(Flag, "enabled", "Enable", "Enable/Disable BGP peer")

o = sect_peers:option(Value, "as", "Remote AS", "")
	o.datatype = "integer"

o = sect_peers:option(Value, "ipaddr", "Remote address", "")
	o.datatype = "ip4addr"

local peer_group_section = m:section(TypedSection, "peer_group", "BGP peer groups", "")
	peer_group_section.addremove = true
	peer_group_section.template = "quagga/tblsection"
	peer_group_section.novaluetext = "There are no BGP peer groups created yet."
	peer_group_section.extedit   = dsp.build_url("admin/network/routes/dynamic_routes/bgp_peer_group/%s")
	peer_group_section.defaults = {enabled = "0"}
	peer_group_section.sectionhead = "Name"

o = peer_group_section:option(Value, "as", "Remote AS", "")
	o.datatype = "integer"

sect_access = m:section(TypedSection, "access_list", "Access list filters", "")
	sect_access.addremove = true
	sect_access.anonymous = true
	sect_access.template = "cbi/tblsection"
	sect_access.novaluetext = "There are no BGP filters created yet."

o = sect_access:option(Flag, "enabled", "Enable", "Enable/Disable BGP filter")

o = sect_access:option(ListValue, "target", "Peer")
	m.uci:foreach(m.config, "peer", function(s)
		o:value(s[".name"], s[".name"])
	end)
o = sect_access:option(ListValue, "action", "Action")
	o:value("permit", "Permit")
	o:value("deny", "Deny")

o = sect_access:option(Value, "net", "Network", "Filter network")
	o:value("any", "Any")
	o.default = "any"

o = sect_access:option(ListValue, "direction", "Direction")
	o:value("in", "Inbound")
	o:value("out", "Outbound")

function m.on_commit(self)
	local enabled = m:formvalue("cbid." .. m.config .. ".general.enabled" ) or "0"
	local firewall_rule = m.uci:get("firewall", "A_BGP")

	if firewall_rule then
		local firewall_ed = m.uci:set("firewall", "A_BGP", "enabled", enabled)
	else
		local options = {
			enabled=enabled,
			target='ACCEPT',
			src='wan',
			proto='tcp udp',
			dest_port='179',
			name='Allow-BGP-WAN-traffic'
		}

		m.uci:set("firewall", "A_BGP", "rule")
		m.uci:tset("firewall", "A_BGP", options)
	end

	m.uci:commit("firewall")
end

return m
