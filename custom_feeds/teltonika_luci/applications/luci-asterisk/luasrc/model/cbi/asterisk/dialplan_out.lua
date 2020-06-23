--[[
LuCI - Lua Configuration Interface

Copyright 2008 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: dialplan_out.lua 4385 2009-03-29 19:11:57Z jow $

]]--

local ast = require("luci.asterisk")

local function find_outgoing_contexts(uci)
	local c = { }
	local h = { }

--	uci:foreach("asterisk", "dialplan",
--		function(s)
--			if not h[s['.name']] then
--				c[#c+1] = { s['.name'], "Dialplan: %s" % s['.name'] }
--				h[s['.name']] = true
--			end
--		end)

	uci:foreach("asterisk", "dialzone",
		function(s)
			if not h[s['.name']] then
				c[#c+1] = { s['.name'], translatef("Dialzone: %s"), s['.name'] }
				h[s['.name']] = true
			end
		end)

	return c
end

local function find_incoming_contexts(uci)
	local c = { }
	local h = { }

	uci:foreach("asterisk", "sip",
		function(s)
			if s.context and not h[s.context] and
			   uci:get_bool("asterisk", s['.name'], "provider")
			then
				c[#c+1] = { s.context, translatef("Incoming: %s"), s['.name'] or s.context }
				h[s.context] = true
			end
		end)

	return c
end

local function find_trunks(uci)
	local t = { }

	uci:foreach("asterisk", "sip",
		function(s)
			if uci:get_bool("asterisk", s['.name'], "provider") then
				t[#t+1] = {
					"SIP/%s" % s['.name'],
					"SIP: %s" % s['.name']
				}
			end
		end)

	uci:foreach("asterisk", "iax",
		function(s)
			t[#t+1] = {
				"IAX/%s" % s['.name'],
				"IAX: %s" % s.extension or s['.name']
			}
		end)

	return t
end

--[[

dialzone {name} - Outgoing zone.
	uses          - Outgoing line to use: TYPE/Name
	match (list)  - Number to match
	countrycode   - The effective country code of this dialzone
	international (list) - International prefix to match
	localzone     - dialzone for local numbers
	addprefix     - Prexix required to dial out.
	localprefix   - Prefix for a local call

]]


--
-- SIP dialzone configuration
--
if arg[1] then
	cbimap = Map("asterisk", translate("Edit Dialplan Entry"))

	entry = cbimap:section(NamedSection, arg[1])

	back = entry:option(DummyValue, "_overview", translate("Back to dialplan overview"))
	back.value = ""
	back.titleref = luci.dispatcher.build_url("admin", "asterisk", "dialplans")

	desc = entry:option(Value, "description", translate("Description"))
	function desc.cfgvalue(self, s, ...)
		return Value.cfgvalue(self, s, ...) or s
	end

	match = entry:option(DynamicList, "match", translate("Number matches"))

	intl = entry:option(DynamicList, "international", translate("Intl. prefix matches (optional)"))

	trunk = entry:option(MultiValue, "uses", translate("Used trunk"))
	for _, v in ipairs(find_trunks(cbimap.uci)) do
		trunk:value(unpack(v))
	end

	aprefix = entry:option(Value, "addprefix", translate("Add prefix to dial out (optional)"))
	--ast.idd.cbifill(aprefix)

	ccode = entry:option(Value, "countrycode", translate("Effective countrycode (optional)"))
	ast.cc.cbifill(ccode)

	lzone = entry:option(ListValue, "localzone", translate("Dialzone for local numbers"))
	lzone:value("", translate("no special treatment of local numbers"))
	for _, v in ipairs(find_outgoing_contexts(cbimap.uci)) do
		lzone:value(unpack(v))
	end

	lprefix = entry:option(Value, "localprefix", translate("Prefix for local calls (optional)"))

	return cbimap
end
