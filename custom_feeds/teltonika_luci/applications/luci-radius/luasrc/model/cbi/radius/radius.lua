--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: coovachilli.lua 3442 2008-09-25 10:12:21Z jow $
]]--
local ds = require "luci.dispatcher"
local function debug(string)
	luci.sys.call("logger \"" .. string .. "\"")
end

function round(num, idp)
  local mult = 10^(idp or 0)
  return math.floor(num * mult + 0.5) / mult
end

m = Map( "radius",	translate( "Radius Server Configuration" ), translate( "" ))

if arg[1] and arg[1] == "hotspot" then
	m.message=translate("scs:Please enable radius server and configure users")
end

scc = m:section( NamedSection, "general", "general", translate( "General Settings"))

cen = scc:option( Flag, "enabled", translate("Enable"), translate("Enable radius server functionality on the router"))

wan_enb = scc:option( Flag, "wan_enb", translate("Remote access"), translate("Enable remote radius server access"))
	wan_enb.enabled = "1"
	wan_enb.disabled = "0"
	wan_enb.rmempty = false

	function wan_enb.write(self, section, value)
		m.uci:foreach( "firewall", "rule", function(s)
			if s.name == "Enable_Radius_WAN" then
				m.uci:set("firewall", s[".name"], "enabled", value)
				m.uci:save("firewall")
				m:chain("firewall")
			end
		end)
		m.uci:commit("firewall")
	end

	function wan_enb.cfgvalue(self, section)
		local value
		m.uci:foreach( "firewall", "rule", function(s)
			if s.name == "Enable_Radius_WAN" then
				value = m.uci:get("firewall", s[".name"], "enabled")
			end
		end)
		return value or 0
	end

--Ateityje bus galima padaryti interface pasirinkima jei kam reikes
-- interface = scc:option(ListValue, "interface", translate("Iterface"), translate("Make's the server listen on a particular interface"))
-- 	interface:value("lan", "LAN")
-- 	interface:value("wan", "WAN")
-- 	interface:value("all", "All")
-- 	interface.default = "all"

acc_port = scc:option(Value, "acc_port", translate("Accounting port"), translate("Port on which to listen accounting. 0 means \\'use /etc/services for the proper port\\'"))
	acc_port.default="1813"

auth_port = scc:option(Value, "auth_port", translate("Authentication port"), translate("Port on which to listen authentication. 0 means \\'use /etc/services for the proper port\\'"))
	auth_port.default="1812"

ses = m:section(TypedSection, "session", translate("Session Settings"))
	ses.addremove = true
	ses.anonymous = true
	ses.template  = "cbi/tblsection_custom"
	ses.novaluetext = "There are no templates created yet."
	ses.extedit   = ds.build_url("admin/services/hotspot/radius_session/%s")
	ses.redirect = true
	ses.addfields = {
		{title = "Template name", type = "text", class="cbi-input-text", style="margin-left: 10px;",
		name="_newinput.seestion.template", id="template_input", maxlength="32", onchange="custom_valid(this, /^[a-zA-Z0-9_]+$/)"}
	}


	function ses.remove_button(self, section, k)
		if section and not section:find("unlimited") then
			return [[<input class="cbi-button cbi-button-remove" type="submit" value="Delete"  onclick="this.form.cbi_state='del-section'; return true" name="cbi.rts.]] .. self.config .. [[.]] .. k .. [[" alt="Delete" title="Delete" />]]
		end
	end

	function ses.create(self, section)
		local template_exists = false
		local name = m:formvalue("_newinput.seestion.template") or ""

		if name ~= "" then
			m.uci:foreach(self.config, "session", function(s)
				if s.name == name then
					template_exists=true
				end
			end)

			if not template_exists then
				local random_section = "sec" .. os.time()
				local created = TypedSection.create(self, random_section)
				self.map:set(random_section, "name",   name)
			else
				m.message = translate("err: Template is already \"" .. name .. "\" exists")
			end
		else
			m.message = translate("err: Template name is empty")
		end
	end

	function ses.remove(self, section)
		self.map.uci:foreach(self.config, "users", function(s)
			if s.template == section then
				self.map:set(s[".name"], "template", "unlimited")
			end
		end)

		self.map.uci:commit(self.config)
		self.map.proceed = true
		return self.map:del(section)
	end

uli = ses:option(DummyValue, "name", translate("Name"))

download_band = ses:option(DummyValue, "downloadbandwidth", translate("Download bandwidth"), translate("The max allowed download speed, in megabits." ))

	function download_band.cfgvalue(self, section)
		local unit_value = m.uci:get(self.config, section, "d_bandwidth_unit") or "kb"
		local multiplier = unit_value == "kb" and 1000 or 1000000
		local value = m.uci:get(self.config, section, self.option)
		value = value and tonumber(value) / multiplier or nil

		return value and string.format("%s %s/s", value, unit_value) or "Unlimited"
	end

upload_band = ses:option(DummyValue, "uploadbandwidth", translate("Upload bandwidth"), translate("The max allowed upload speed, in megabits." ))

	function upload_band.cfgvalue(self, section)
		local unit_value = m.uci:get(self.config, section, "u_bandwidth_unit") or "kb"
		local multiplier = unit_value == "kb" and 1000 or 1000000
		local value = m.uci:get(self.config, section, self.option)
		value = value and tonumber(value) / multiplier or nil

		return value and string.format("%s %s/s", value, unit_value) or "Unlimited"
	end

downloadlimit = ses:option(DummyValue, "downloadlimit", translate("Download limit"), translate("Disable hotspot user after download limit value in MB is reached"))

	function downloadlimit.cfgvalue(self, section)
		local value = m.uci:get(self.config, section, self.option)
		value = value and tonumber(value) / 1048576 or nil

		return value and string.format("%s MB", value) or "Unlimited"
	end

uploadlimit = ses:option(DummyValue, "uploadlimit", translate("Upload limit"), translate("Disable hotspot user after upload limit value in MB is reached"))

	function uploadlimit.cfgvalue(self, section)
		local value = m.uci:get(self.config, section, self.option)
		value = value and tonumber(value) / 1048576 or nil

		return value and string.format("%s MB", value) or "Unlimited"
	end

period = ses:option(DummyValue, "period", translate("Period"), translate("Period for which hotspot data limiting should apply"))

	function period.cfgvalue(self, section)
		local period = {"Day", "Week", "Month"}
		local value = m.uci:get(self.config, section, self.option)

		return period[tonumber(value)] or "-"
	end

users = m:section(TypedSection, "user", translate( "Users Configuration Settings"))
	users.addremove = true
	users.anonymous = true
	users.template  = "radius/users_tblsection"
	users.novaluetext = "There are no users created yet."
	users.extedit   = ds.build_url("admin/services/hotspot/radius_user/%s")
	users.redirect = true
	function users.create(self, section)

		local name = m:formvalue("_newinput.username") or ""
		local pass = m:formvalue("_newinput.pass") or ""
		local template = m:formvalue("_user.seestion.template") or "unlimited"

		if name ~= "" then
			m.uci:foreach(self.config, "user", function(s)
				if s.username == name then
					template_exists=true
				end
			end)

			if not template_exists then
				local created = TypedSection.create(self, section)
				self.map:set(created, "username",   name)
				self.map:set(created, "pass", pass)
				self.map:set(created, "enabled", "1")
				self.map:set(created, "template", template)
			else
				m.message = translate("err: Username is already \"" .. name .. "\" exists")
			end

			-- if name ~= "" and pass ~= "" then
			-- 	--self.redirect = false
			-- end
		else
			m.message = translate("err: Username is empty")
		end
	end

	function users.parse(self, ...)
		TypedSection.parse(self, ...)
		if created then
			m.uci:save("radius")
			if self.redirect then
				luci.http.redirect(ds.build_url("admin/services/hotspot/radius", created))
			end
		end
	end

enb_user = users:option(Flag, "enabled", translate("Enable"), translate("" ))
	enb_user.enabled = "1"
	enb_user.rmempty = false

name = users:option(DummyValue, "username", translate("User name"), translate("" ))
	name.rmempty = false

-- pass = users:option(Value, "pass", translate("User password"), translate("" ))
-- 	pass.pasword = true
-- 	pass.rmempty = false

-- message = users:option(DummyValue, "message", translate("Reply message"), translate("" ))
--
-- 	function message.cfgvalue(self, section)
-- 		local value = m.uci:get(self.config, section, self.option) or ""
-- 		if value == "" then
-- 			return "-"
-- 		else
-- 			return value
-- 		end
-- 	end

idle = users:option(DummyValue, "defidletimeout", translate("Idle timeout" ), translate("Max idle time in sec. (0, meaning unlimited)"))
	idle.datatype = "integer"

	function idle.cfgvalue(self, section)
		local template = m.uci:get(self.config, section, "template")
		local value = "Unlimited"

		if template then
			value = m.uci:get(self.config, template, self.option) or "0"
			if value == "0" then
				value = "Unlimited"
			else
				value = value .. " sec."
			end
		end

		return value
	end

timeout = users:option(DummyValue, "defsessiontimeout", translate("Session timeout" ), translate("Max session time in sec. (0, meaning unlimited)"))
	timeout.datatype = "integer"

	function timeout.cfgvalue(self, section)
		local template = m.uci:get(self.config, section, "template")
		local value = "Unlimited"

		if template then
			value = m.uci:get(self.config, template, self.option) or "0"

			if value == "0" then
				value = "Unlimited"
			else
				value = value .. " sec."
			end
		end

		return value
	end

download_band = users:option(DummyValue, "downloadbandwidth", translate("Download bandwidth"), translate("The max allowed download speed, in bits." ))
	download_band.datatype = "integer"

	function download_band.cfgvalue(self, section)
		local template = m.uci:get(self.config, section, "template")
		local value = "Unlimited"

		if template then
			value = m.uci:get(self.config, template, self.option) or "0"

			if value == "0" then
				value = "Unlimited"
			else
				local tail
				local measure = {"bps.", "Kbps", "Mbps.", "Gbps."}
				value = tonumber(value)
				for i, n in pairs(measure) do
					tail = n
					if value >= 1000 and n ~= "Gbps." then
						value = value / 1000
					else
						break
					end
				end
				value = string.format("%s %s", round(value, 3), tail)
			end
		end

		return value
	end

upload_band = users:option(DummyValue, "uploadbandwidth", translate("Upload bandwidth"), translate("The max allowed upload speed, in bits." ))
	upload_band.datatype = "integer"

	function upload_band.cfgvalue(self, section)
		local template = m.uci:get(self.config, section, "template")
		local value = "Unlimited"

		if template then
			value = m.uci:get(self.config, template, self.option) or "0"

			if value == "0" then
				value = "Unlimited"
			else
				local tail
				local measure = {"bps.", "Kbps", "Mbps.", "Gbps."}
				value = tonumber(value)
				for i, n in pairs(measure) do
					tail = n
					if value >= 1000 and n ~= "Gbps." then
						value = value / 1000
					else
						break
					end
				end
				value = string.format("%s %s", round(value, 3), tail)
			end
		end
		return value
	end

template = users:option(ListValue, "template", "Session template")

	m.uci:foreach(template.config, "session", function(sec)
		if sec.name then
			template:value(sec[".name"], sec.name)
		end
	end)

client = m:section(TypedSection, "client", translate( "Clients Configuration Settings"))
	client.addremove = true
	client.anonymous = true
	client.template  = "radius/tblsection"
	client.novaluetext = "There are no clients created yet."

enb_c = client:option(Flag, "enabled", translate("Enable"), translate("Enable client"))
	enb_c.default = "1"
	enb_c.rmempty = false

name = client:option(Value, "name", translate("Client name"), translate("" ))
	name.rmempty = false

	function name.validate(self, value)
		if value == "localhost" then
			m.message = translate("err:Name \"localhost\" is not available")
			return nil, translate("Name \"localhost\" is not available")
		end
		return value
	end

ipaddr = client:option(Value, "ipaddr", translate("IP address"), translate("The IP address of the client."))
	ipaddr.datatype = "ip4addr"

	function ipaddr.validate(self, value)
		if value == "127.0.0.1" then
			m.message = translate("err:IP address \"127.0.0.1\" is not available")
			return nil, translate("IP address \"127.0.0.1\" is not available")
		end
		return value
	end

mask = client:option(Value, "mask", translate("Netmask"))
	mask.datatype = "range(0,32)"

secret = client:option(Value, "secret", translate("Radius shared secret"), translate("The  RADIUS  shared  secret  used  for  communication  between the client/NAS and the radius server"))
	secret.rmempty = false
return m
