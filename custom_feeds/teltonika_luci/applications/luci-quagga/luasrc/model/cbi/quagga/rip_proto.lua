require("luci.sys")
local dsp = require "luci.dispatcher"

m=Map("quagga", "RIP protocol's configuration")

FileUpload.size = "1000000"
FileUpload.sizetext = translate("Selected file is too large, max 1 MB")
FileUpload.sizetextempty = translate("Selected file is empty")

-- Section BGP Instances:
sect_general = m:section(NamedSection, "rip", "rip", "General", "")

sect_general:option(Flag, "enabled", "Enable", "Enable/Disable RIP protocol")
sect_general:option(Flag, "enabled_vty", "Enable vty", "Enable/Disable vty access from LAN")
sect_general:option(FileUpload, "bgpd_custom_conf", "Import config", "Use imported RIP configuration")
o = sect_general:option(ListValue, "version", "Version", "Specify the version of RIP")
	o:value("2", "2")
	o:value("1", "1")
	o.default = "2"
sect_general:option(DynamicList, "neighbors", "Neighbor", "Specify RIP neighbor")

sect_peers = m:section(TypedSection, "interface", "RIP interfaces", "")
	sect_peers.addremove = true
	sect_peers.anonymous = true
	sect_peers.template = "cbi/tblsection"
	sect_peers.novaluetext = "There are no RIP interfaces created yet."

sect_peers:option(Flag, "enabled", "Enable", "Enable/Disable RIP interface")
o = sect_peers:option(ListValue, "ifname", "Interface", "Interface name")
	for k, v in pairs(luci.sys.net.devices()) do
		o:value(v)
	end

sect_peers:option(Flag, "passive_interface", "Passive interface", "Specify interface to passive mode")


sect_access = m:section(TypedSection, "rip_access_list", "Access list filters", "")
	sect_access.addremove = true
	sect_access.anonymous = true
	sect_access.template = "cbi/tblsection"
	sect_access.novaluetext = "There are no RIP filters created yet."

o = sect_access:option(Flag, "enabled", "Enable", "Enable/Disable RIP filter")

o = sect_access:option(ListValue, "target", "RIP interface")
	m.uci:foreach(m.config, "interface", function(s)
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
	local enabled = m:formvalue("cbid." .. m.config .. ".rip.enabled" ) or "0"
	local firewall_rule = m.uci:get("firewall", "A_RIP")

	if firewall_rule then
		local firewall_ed = m.uci:set("firewall", "A_RIP", "enabled", enabled)
	else
		local options = {
			enabled=enabled,
			target='ACCEPT',
			src='wan',
			proto='udp',
			dest_port='520',
			name='Allow-RIP-WAN-traffic'
		}

		m.uci:set("firewall", "A_RIP", "rule")
		m.uci:tset("firewall", "A_RIP", options)
	end

	m.uci:commit("firewall")
end

return m
