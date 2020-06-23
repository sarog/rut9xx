--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008-2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: ifaces.lua 7717 2011-10-13 16:26:59Z jow $
]]--
local m, s, o

local fs = require "nixio.fs"
local ut = require "luci.util"
local nw = require "luci.model.network"
local fw = require "luci.model.firewall"

arg[1] = "wan"

m = Map("network", translate("Wired"), translate("You have chosen the wired wan option. Here we will configure the basic settings of a typical wire wan configuration. The wizard will cover 2 basic configurations: static IP WAN and DHCP client. If you get your wired internet from some other configuration type, then after the wizard please follow through to Network->LAN->Edit as it covers many more options."))
m:chain("wireless")

if m:formvalue("cbid.network._wired._next") then
	luci.http.redirect(luci.dispatcher.build_url("admin/wizard/step4-lan"))
	return
end
if m:formvalue("cbi.wizard.skip") then
	luci.http.redirect(luci.dispatcher.build_url("/admin/status/sysinfo"))
end
if m:formvalue("cbi.apply") then
	m.message = translate("wrn:Configuration applied. If there are no error you can proccedd to the next step.")
end

nw.init(m.uci)

local net = nw:get_network(arg[1])

local function backup_ifnames(is_bridge)
	if not net:is_floating() and not m:get(net:name(), "_orig_ifname") then
		local ifcs = net:get_interfaces() or { net:get_interface() }
		if ifcs then
			local _, ifn
			local ifns = { }
			for _, ifn in ipairs(ifcs) do
				ifns[#ifns+1] = ifn:name()
			end
			if #ifns > 0 then
				m:set(net:name(), "_orig_ifname", table.concat(ifns, " "))
				m:set(net:name(), "_orig_bridge", tostring(net:is_bridge()))
			end
		end
	end
end


-- redirect to overview page if network does not exist anymore (e.g. after a revert)
if not net then
	luci.http.redirect(luci.dispatcher.build_url("admin/network/network"))
	return
end

-- protocol switch was requested, rebuild interface config and reload page
if m:formvalue("cbid.network.%s._switch" % net:name()) then
	-- get new protocol
	local ptype = m:formvalue("cbid.network.%s.proto" % net:name()) or "-"
	local proto = nw:get_protocol(ptype, net:name())
	if proto then
		-- backup default
		backup_ifnames()

		-- if current proto is not floating and target proto is not floating,
		-- then attempt to retain the ifnames
		--error(net:proto() .. " > " .. proto:proto())
		if not net:is_floating() and not proto:is_floating() then
			-- if old proto is a bridge and new proto not, then clip the
			-- interface list to the first ifname only
			if net:is_bridge() and proto:is_virtual() then
				local _, ifn
				local first = true
				for _, ifn in ipairs(net:get_interfaces() or { net:get_interface() }) do
					if first then
						first = false
					else
						net:del_interface(ifn)
					end
				end
				m:del(net:name(), "type")
			end

		-- if the current proto is floating, the target proto not floating,
		-- then attempt to restore ifnames from backup
		elseif net:is_floating() and not proto:is_floating() then
			-- if we have backup data, then re-add all orphaned interfaces
			-- from it and restore the bridge choice
			local br = (m:get(net:name(), "_orig_bridge") == "true")
			local ifn
			local ifns = { }
			for ifn in ut.imatch(m:get(net:name(), "_orig_ifname")) do
				ifn = nw:get_interface(ifn)
				if ifn and not ifn:get_network() then
					proto:add_interface(ifn)
					if not br then
						break
					end
				end
			end
			if br then
				m:set(net:name(), "type", "bridge")
			end

		-- in all other cases clear the ifnames
		else
			local _, ifc
			for _, ifc in ipairs(net:get_interfaces() or { net:get_interface() }) do
				net:del_interface(ifc)
			end
			m:del(net:name(), "type")
		end

		-- clear options
		local k, v
		for k, v in pairs(m:get(net:name())) do
			if k:sub(1,1) ~= "." and
			   k ~= "type" and
			   k ~= "ifname" and
			   k ~= "_orig_ifname" and
			   k ~= "_orig_bridge"
			then
				m:del(net:name(), k)
			end
		end

		-- set proto
		m:set(net:name(), "proto", proto:proto())
		m.uci:save("network")
		m.uci:save("wireless")

		-- reload page
		luci.http.redirect(luci.dispatcher.build_url("admin/wizard/step32-wire"))
		return
	end
end

local ifc = net:get_interface()

s = m:section(NamedSection, arg[1], "interface", translate("Common Configuration"))
s.addremove = false

s:tab("general",  translate("General Setup"))
s:tab("advanced", translate("Advanced Settings"))
s:tab("physical", translate("Physical Settings"))

p = s:taboption("general", ListValue, "proto", translate("Protocol"))
p.default = net:proto()

p_switch = s:taboption("general", Button, "_switch")
p_switch.title      = translate("Really switch protocol?")
p_switch.inputtitle = translate("Switch protocol")
p_switch.inputstyle = "apply"

--[[local _, pr
for _, pr in ipairs(nw:get_protocols()) do
	p:value(pr:proto(), pr:get_i18n())
	if pr:proto() ~= net:proto() then
		p_switch:depends("proto", pr:proto())
	end
end]]

p:value("static", "Static address")
p:value("dhcp", "DCHP Client")
if "static" ~= net:proto() then
	p_switch:depends("proto", "static")
else
	p_switch:depends("proto", "dhcp")
end

auto = s:taboption("advanced", Flag, "auto", translate("Bring up on boot"))
auto.default = (net:proto() == "none") and auto.disabled or auto.enabled

function p.write() end
function p.remove() end
function p.validate(self, value, section)
	if value == net:proto() then
		if not net:is_floating() and net:is_empty() then
			local ifn = ((br and (br:formvalue(section) == "bridge"))
				and ifname_multi:formvalue(section)
			     or ifname_single:formvalue(section))

			for ifn in ut.imatch(ifn) do
				return value
			end
			return nil, translate("The selected protocol needs a device assigned")
		end
	end
	return value
end


local form, ferr = loadfile(
	ut.libpath() .. "/model/cbi/admin_network/proto_%s.lua" % net:proto()
)

if not form then
	s:taboption("general", DummyValue, "_error",
		translate("Missing protocol extension for proto %q" % net:proto())
	).value = ferr
else
	setfenv(form, getfenv(1))(m, s, net)
end


local _, field
for _, field in ipairs(s.children) do
	if field ~= st and field ~= p and field ~= p_install and field ~= p_switch then
		if next(field.deps) then
			local _, dep
			for _, dep in ipairs(field.deps) do
				dep.deps.proto = net:proto()
			end
		else
			field:depends("proto", net:proto())
		end
	end
end

s = m:section(TypedSection, "_dummy", "")
s.addremove = false
s.anonymous = true

function s.cfgsections()
	return { "_wired" }
end

o = s:option(DummyValue, "_prevNext", translate(" "))
o.template = "cfgwzd-module/next_apply"

testButton = s:option(Button, "_next")

testButton.title      = translate(" ")
testButton.inputtitle = translate("Next")
testButton.inputstyle = "apply"

return m
