--[[
LuCI - Lua Configuration Interface

Copyright 2008 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: phone_sip.lua 4356 2009-03-21 05:00:40Z jow $

]]--

local ast = require("luci.asterisk")

local function find_outgoing_contexts(uci)
	local c = { }
	local h = { }

	uci:foreach("asterisk", "dialplan",
		function(s)
			if not h[s['.name']] then
				c[#c+1] = { s['.name'], translatef("Dialplan: %s"), s['.name'] }
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


--
-- SIP phone info
--
if arg[2] == "info" then
	form = SimpleForm("asterisk", translate("SIP Phone Information"))
	form.reset  = false
	form.submit = translate("Back to overview")

	local info, keys = ast.sip.peer(arg[1])
	local data = { }

	for _, key in ipairs(keys) do
		data[#data+1] = {
			key = key,
			val = type(info[key]) == "boolean"
				and ( info[key] and "yes" or "no" )
				or  ( info[key] == nil or #info[key] == 0 )
					and "(none)"
					or  tostring(info[key])
		}
	end

	itbl = form:section(Table, data, translatef("SIP Phone %q"), arg[1])
	itbl:option(DummyValue, "key", translate("Key"))
	itbl:option(DummyValue, "val", translate("Value"))

	function itbl.parse(...)
		luci.http.redirect(
			luci.dispatcher.build_url("admin", "asterisk", "phones")
		)
	end

	return form

--
-- SIP phone configuration
--
elseif arg[1] then
	cbimap = Map("asterisk", translate("Edit SIP Client"))

	peer = cbimap:section(NamedSection, arg[1])
	peer.hidden = {
		type        = "friend",
		qualify     = "yes",
		host        = "dynamic",
		nat         = "no",
		canreinvite = "no"
	}

	back = peer:option(DummyValue, "_overview", translate("Back to phone overview"))
	back.value = ""
	back.titleref = luci.dispatcher.build_url("admin", "asterisk", "phones")

	active = peer:option(Flag, "disable", translate("Account enabled"))
	active.enabled  = "yes"
	active.disabled = "no"
	function active.cfgvalue(...)
		return AbstractValue.cfgvalue(...) or "yes"
	end

	exten = peer:option(Value, "extension", translate("Extension Number"))
	cbimap.uci:foreach("asterisk", "dialplanexten",
		function(s)
			exten:value(
				s.extension,
				"%s (via %s/%s)" %{ s.extension, s.type:upper(), s.target }
			)
		end)

	display = peer:option(Value, "callerid", translate("Display Name"))

	username  = peer:option(Value, "username", translate("Authorization ID"))
	password  = peer:option(Value, "secret", translate("Authorization Password"))
	password.password = true

	regtimeout = peer:option(Value, "registertimeout", translate("Registration Time Value"))
	function regtimeout.cfgvalue(...)
		return AbstractValue.cfgvalue(...) or "60"
	end

	sipport = peer:option(Value, "port", translate("SIP Port"))
	function sipport.cfgvalue(...)
		return AbstractValue.cfgvalue(...) or "5060"
	end

	linekey = peer:option(ListValue, "_linekey", translate("Linekey Mode (broken)"))
	linekey:value("", "Off")
	linekey:value("trunk", translate("Trunk Appearance"))
	linekey:value("call", translate("Call Appearance"))

	dialplan = peer:option(ListValue, "context", translate("Assign Dialplan"))
	dialplan.titleref = luci.dispatcher.build_url("admin", "asterisk", "dialplans"))
	for _, v in ipairs(find_outgoing_contexts(cbimap.uci)) do
		dialplan:value(unpack(v))
	end

	incoming = peer:option(StaticList, "incoming", translate("Receive incoming calls from"))
	for _, v in ipairs(find_incoming_contexts(cbimap.uci)) do
		incoming:value(unpack(v))
	end

	--function incoming.cfgvalue(...)
		--error(table.concat(MultiValue.cfgvalue(...),"."))
	--end

	return cbimap
end
