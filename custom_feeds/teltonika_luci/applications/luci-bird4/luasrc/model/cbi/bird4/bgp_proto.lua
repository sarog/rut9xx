require("luci.sys")

local dsp = require "luci.dispatcher"
local uci = require "luci.model.uci"
local uciout = uci.cursor()

m=Map("bird4", "BGP protocol's configuration")

-- Section BGP Templates

sect_templates = m:section(TypedSection, "bgp_template", "BGP Templates", "Configuration of the templates used in BGP instances.")
	sect_templates.addremove = true
	sect_templates.anonymous = false
	sect_templates.template = "cbi/tblsection"
	sect_templates.novaluetext = "There are no BGP templates created yet."
	sect_templates.extedit   = dsp.build_url("admin/network/routes/dynamic_routes/bgp_template/%s")
	sect_templates.defaults.local_as = "100"

	function sect_templates.filter(self, name)
		if name and name ~= "" and not name:match("^[a-zA-Z]+$") then
			m.message = translatef("err: Invalid name '%s'", name)
			return false
		end

		return true
	end

local_address = sect_templates:option(DummyValue, "local_address", "Local BGP address", "")

	function local_address.cfgvalue(self, section)
		return self.map:get(section, self.option) or "-"
	end

local_as = sect_templates:option(DummyValue, "local_as", "Local AS", "")

	function local_as.cfgvalue(self, section)
		return self.map:get(section, self.option) or "-"
	end

-- Section BGP Instances:

sect_instances = m:section(TypedSection, "bgp", "BGP Instances", "Configuration of the BGP protocol instances")
	sect_instances.anonymous = false
	sect_instances.addremove = true
	sect_instances.template = "cbi/tblsection"
	sect_instances.novaluetext = "There are no BGP instances created yet."

	function sect_instances.filter(self, name)
		if name and name ~= "" and not name:match("^[a-zA-Z]+$") then
			m.message = translatef("err: Invalid name '%s'", name)
			return false
		end

		return true
	end

	function sect_instances.create(self, section)
		local stat
		local template = false
		-- Ignore if tamplate section not created
		self.map.uci:foreach(self.config, "bgp_template", function(s)
			template = true
		end)

		if not template then
			m.message = translatef("err: There are no BGP templates created.")
			return false
		end

		if section then
			stat = section:match("^[%w_]+$") and self.map:set(section, nil, self.sectiontype)
		else
			section = self.map:add(self.sectiontype)
			stat = section
		end

		if stat then
			for k,v in pairs(self.children) do
				if v.default then
					self.map:set(section, v.option, v.default)
				end
			end

			for k,v in pairs(self.defaults) do
				self.map:set(section, k, v)
			end
		end

		self.map.proceed = true

		return stat
	end

enabled = sect_instances:option(Flag, "enabled", "Enable", "Enable/Disable BGP Protocol")
	enabled.default=nil

templates = sect_instances:option(ListValue, "template", "Templates", "Available BGP templates")
	uciout:foreach("bird4", "bgp_template",
		function(s)
			templates:value(s[".name"])
		end)
	templates:value("")

neighbor_address = sect_instances:option(Value, "neighbor_address", "Neighbor IP Address", "")

neighbor_as = sect_instances:option(Value, "neighbor_as", "Neighbor AS", "")

-- Section BGP Filters

-- sect_filters = m:section(TypedSection, "filter", "BGP Filters", "Filters of the BGP instances")
-- 	sect_filters.addremove = true
-- 	sect_filters.anonymous = false
-- 	sect_filters.template = "cbi/tblsection"
-- 	sect_filters.novaluetext = "There are no BGP filters created yet."
-- 	sect_filters:depends("type", "bgp")
--
-- instance = sect_filters:option(ListValue, "instance", "BGP instance", "Filter's BGP instance")
-- 	instance:depends("type", "bgp")
--
-- 	uciout:foreach("bird4", "bgp",
-- 		function (s)
-- 			instance:value(s[".name"])
-- 		end)
--
-- type = sect_filters:option(Value, "type", "Filter type", "")
-- 	type.default = "bgp"
--
-- path = sect_filters:option(Value, "file_path", "Filter's file path", "Path to the Filter's file")
-- 	path:depends("type", "bgp")

-- function m.on_commit(self,map)
--         luci.sys.call('/etc/init.d/bird4 stop; /etc/init.d/bird4 start')
-- end

return m
