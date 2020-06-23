--[[
LuCI - Lua Configuration Interface

Copyright 2008 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: trunks.lua 4025 2009-01-11 23:37:21Z jow $

]]--

local ast = require("luci.asterisk")
local uci = require("luci.model.uci").cursor()

--[[
	Dialzone overview table
]]

if not arg[1] then
	zonemap = Map("asterisk", translate("Dial Zones"), translate(" Dial zones hold patterns of dialed numbers to match. Each zone has one or more trunks assigned. If the first trunk is congested, Asterisk will try to use the next available connection. If all trunks fail, then the following zones in the parent dialplan are tried. "))

	local zones, znames = ast.dialzone.zones()

	zonetbl = zonemap:section(Table, zones, translate("Zone Overview"))
	zonetbl.sectionhead = "Zone"
	zonetbl.addremove   = true
	zonetbl.anonymous   = false
	zonetbl.extedit     = luci.dispatcher.build_url(
		"admin", "asterisk", "dialplans", "zones", "%s"
	)

	function zonetbl.cfgsections(self)
		return znames
	end

	function zonetbl.parse(self)
		for k, v in pairs(self.map:formvaluetable(
			luci.cbi.REMOVE_PREFIX .. self.config
		) or {}) do
			if k:sub(-2) == ".x" then k = k:sub(1, #k - 2) end
			uci:delete("asterisk", k)
			uci:save("asterisk")
			self.data[k] = nil
			for i = 1,#znames do
				if znames[i] == k then
					table.remove(znames, i)
					break
				end
			end
		end

		Table.parse(self)
	end

	zonetbl:option(DummyValue, "description", translate("Description"))
	zonetbl:option(DummyValue, "addprefix")

	match = zonetbl:option(DummyValue, "matches")
	function match.cfgvalue(self, s)
		return table.concat(zones[s].matches, ", ")
	end

	trunks = zonetbl:option(DummyValue, "trunk")
	trunks.template = "asterisk/cbi/cell"
	function trunks.cfgvalue(self, s)
		return ast.tools.hyperlinks(zones[s].trunks)
	end

	return zonemap

--[[
	Zone edit form
]]

else
	zoneedit = Map("asterisk", translate("Edit Dialzone"))

	entry = zoneedit:section(NamedSection, arg[1])
	entry.title = translatef("Zone %q"), arg[1];

	back = entry:option(DummyValue, "_overview", translate("Back to dialzone overview"))
	back.value = ""
	back.titleref = luci.dispatcher.build_url(
		"admin", "asterisk", "dialplans", "zones"
	)

	desc = entry:option(Value, "description", translate("Description"))
	function desc.cfgvalue(self, s, ...)
		return Value.cfgvalue(self, s, ...) or s
	end

	trunks = entry:option(MultiValue, "uses", translate("Used trunks"))
	trunks.widget = "checkbox"
	uci:foreach("asterisk", "sip",
		function(s)
			if s.provider == "yes" then
				trunks:value(
					"SIP/%s" % s['.name'],
					"SIP/%s (%s)" %{ s['.name'], s.host or 'n/a' }
				)
			end
		end)


	match = entry:option(DynamicList, "match", translate("Number matches"))

	intl = entry:option(DynamicList, "international", translate("Intl. prefix matches (optional)"))

	aprefix = entry:option(Value, "addprefix", translate("Add prefix to dial out (optional)"))
	ccode = entry:option(Value, "countrycode", translate("Effective countrycode (optional)"))

	lzone = entry:option(ListValue, "localzone", translate("Dialzone for local numbers"))
	lzone:value("", translate("no special treatment of local numbers"))
	for _, z in ipairs(ast.dialzone.zones()) do
		lzone:value(z.name, "%q (%s)" %{ z.name, z.description })
	end
	--for _, v in ipairs(find_outgoing_contexts(zoneedit.uci)) do
	--	lzone:value(unpack(v))
	--end

	lprefix = entry:option(Value, "localprefix", translate("Prefix for local calls (optional)"))

	return zoneedit

end
