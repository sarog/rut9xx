local uci = require "luci.model.uci"
local uciout = uci.cursor()
local dsp = require "luci.dispatcher"

m=Map("bird4", "Bird4 BGP protocol's configuration")
	m.redirect = dsp.build_url("admin/network/routes/dynamic_routes/proto_bgp")

-- Section BGP Templates

sect_templates = m:section(NamedSection, arg[1], "bgp_template", "BGP Templates", "Configuration of the templates used in BGP instances.")
	sect_templates.anonymous = false

-- disabled = sect_templates:option(Flag, "enabled", "Enabled", "Enable/Disable BGP Protocol")

local_address = sect_templates:option(Value, "local_address", "Local BGP address", "")

local_as = sect_templates:option(Value, "local_as", "Local AS", "")

-- table = sect_templates:option(ListValue, "table", "Table", "Set the table used for BGP Routing")
-- 	uciout:foreach("bird4", "table",
-- 		function (s)
-- 			table:value(s.name)
-- 		end)
-- 	table:value("")

import = sect_templates:option(Value, "import", "Import","")
	import:value("all", translate("All"))
	import:value("none", translate("None"))
	import.default= "all"

export = sect_templates:option(Value, "export", "Export", "")
	export:value("all", translate("All"))
	export:value("none", translate("None"))
	export.default= "all"

source_addr = sect_templates:option(Value, "source_address", "Source Address", "Source address for BGP routing. By default uses Router ID")

next_hop_self = sect_templates:option(Flag, "next_hop_self", "Next hop self", "Avoid next hop calculation and advertise own source address as next hop")
	next_hop_self.default = nil


next_hop_keep = sect_templates:option(Flag, "next_hop_keep", "Next hop keep", "Forward the received Next Hop attribute event in situations where the local address should be used instead, like subneting")
	next_hop_keep.default = nil


rr_client = sect_templates:option(Flag, "rr_client", "Route Reflector server", "This router serves as a Route Reflector server and treats neighbors as clients")
	rr_client.default = nil

rr_cluster_id = sect_templates:option(Value, "rr_cluster_id", "Route Reflector Cluster ID", "Identificator of the RR cluster. By default uses the Router ID")

import_limit = sect_templates:option(Value, "import_limit", "Routes import limit", "Specify an import route limit. By default is disabled '0'")
	import_limit.default= "0"

import_limit_action = sect_templates:option(ListValue, "import_limit_action", "Routes import limit action", "Action to take when import routes limit ir reached")
	import_limit_action:value("warn")
	import_limit_action:value("block")
	import_limit_action:value("disable")
	import_limit_action:value("restart")
	import_limit_action.default = "warn"

export_limit = sect_templates:option(Value, "export_limit", "Routes export limit", "Specify an export route limit. By default is disabled '0'")
	export_limit.default="0"

export_limit_action = sect_templates:option(ListValue, "export_limit_action", "Routes export limit action", "Action to take when export routes limit is reached")
	export_limit_action:value("warn")
	export_limit_action:value("block")
	export_limit_action:value("disable")
	export_limit_action:value("restart")
	export_limit_action.default = "warn"

receive_limit = sect_templates:option(Value, "receive_limit", "Routes received limit", "Specify a received route limit. By default is disabled '0'")
	receive_limit.default="0"

receive_limit_action = sect_templates:option(ListValue, "receive_limit_action", "Routes received limit action", "Action to take when received routes limit is reached")
	receive_limit_action:value("warn")
	receive_limit_action:value("block")
	receive_limit_action:value("disable")
	receive_limit_action:value("restart")
	receive_limit_action.default = "warn"

return m
