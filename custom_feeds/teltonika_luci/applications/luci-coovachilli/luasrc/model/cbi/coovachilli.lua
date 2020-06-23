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

local show = require("luci.tools.status").show_mobile()
local nw = require "luci.model.network"
local fs = require "luci.fs"
local dsp = require "luci.dispatcher"

local localusers = "/etc/chilli/localusers"
local hotspot_id = arg[1] and arg[1] or ""
local m, s, o

function round(num, idp)
  local mult = 10^(idp or 0)
  return math.floor(num * mult + 0.5) / mult
end

function ListFiles(path)
	local string = "ls ".. path
	local h = io.popen(string)
	local t = h:read("*all")
	h:close()

	return string.gmatch(t, "%S+") or {}
end

m = Map( "coovachilli",	translate( "Wireless Hotspot Configuration" ))
m.redirect = dsp.build_url("admin/services/hotspot/general/")
nw.init(m.uci)

FileUpload.size = "1000000"
FileUpload.sizetext = translate("Selected file is too large, max 1 MB")
FileUpload.sizetextempty = translate("Selected file is empty")

-----------------------------------------------------------------------

scc = m:section( NamedSection, hotspot_id, "general", translate( "General Settings") )
	scc.hide_error = true
o = scc:option(ListValue, "profile", translate("Configuration profile"), translate(""))
	-- Logic to hide/show session and users written here
	o.template = "chilli/profile"
	o:value("custom", "Custom")

	for i in ListFiles("/etc/chilli/configs") do
		if i ~= "custom" then
			o:value(i, i:sub(1,1):upper()..i:sub(2))
		end
	end

cen = scc:option(Flag, "enabled", translate("Enable"),
	translate("Enable hotspot functionality on the router"))

function cen.on_enable(self, section)
	m.uci:set("firewall", "Hotspot_input", "enabled", "1")
	m.uci:commit("firewall")
end

function cen.on_disable(self, section)
	local hotspot_enabled = false
	m.uci:foreach(self.config, "general", function(sec)
				if sec.enabled and sec.enabled == "1" and sec[".name"] ~= section then
					hotspot_enabled = true
				end
			end)
	if not hotspot_enabled then
		m.uci:set("firewall", "Hotspot_input", "enabled", "0")
		m.uci:commit("firewall")
	end
end

net = scc:option(Value, "net", translate("AP IP"),
	translate("The IP address of this router on the hotspot network. 192.168.2.254/24 means network IP address 192.168.2.0 with a subnet mask 255.255.255.0" ))
	net.datatype = "ip4addr"

uli = scc:option(Value, "uamlogoutip", translate("Logout address" ),
	translate("IP address to instantly logout a client accessing it "))
	uli.default = "1.1.1.1"
	uli.datatype = "ip4addr"
	uli:depends("mode","extrad")
	uli:depends("mode","intrad")
	uli:depends("mode","norad")
	uli:depends("mode","mac")
	uli:depends("mode","sms")

mode = scc:option(ListValue, "mode", translate("Authentication mode"), translate("Specifies the method used to authenticate users"))
	mode:value("extrad", "External RADIUS")
	mode:value("intrad", "Internal RADIUS")
	mode:value("norad", "Without RADIUS")
	if show then
		mode:value("sms", "SMS OTP")
	end

	mode:value("add", "Advertisement")
	mode:value("mac", "MAC auth")
	mode.default="extrad"

	function mode.write(self, section, value)

		if value == "intrad" then
			m.uci:set("radius", "radius", "enabled", "1")
			m.uci:set("radius", "general", "enabled", "1")
			m.redirect = dsp.build_url("admin/services/hotspot/radius/hotspot")
		else
			local disable_radius = true

			m.uci:foreach(self.config, "general", function(s)
				if s.mode == "intrad" and s.enabled == "1" and s[".name"] ~= section then
					disable_radius = false
				end
			end)

			if disable_radius then
				m.uci:set("radius", "general", "enabled", "0")
				m.uci:set("radius", "radius", "enabled", "0")
			end
		end
		m.uci:save("radius")
		m.uci:commit("radius")
		m.uci:set(self.config, section, self.option, value)
	end

auth_proto = scc:option(ListValue, "auth_proto", translate("Authentication protocol"))
	auth_proto:value("pap", "PAP")
	auth_proto:value("chap", "CHAP")
	auth_proto:depends("mode","extrad")

tos_enb = scc:option(Flag, "tos_enb", translate("Terms of Service" ),
	translate("Cient device will be able to access the Internet after agreeing to Terms of Service (ToS)"))
	tos_enb:depends("mode","extrad")
	tos_enb:depends("mode","intrad")
	tos_enb:depends("mode","norad")
	tos_enb:depends("mode","mac")

mac_pass_enb = scc:option(Flag, "mac_pass_enb", translate("Password protection" ),
	translate("Client device will be able to access the internet after entering a password."))
	mac_pass_enb:depends("mode","mac")

mac_pass = scc:option(Value, "mac_pass", translate("Password"), translate("Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:;[]"))
	mac_pass.password = true
	mac_pass:depends("mac_pass_enb","1")
	mac_pass.datatype = "password"

web_page = scc:option(ListValue, "web_page", translate("Website access"),
	translate("Requested website access mode."))
	web_page:depends("mode","mac")
	web_page:value("link", "Link")
	web_page:value("auto", "Auto redirect")
	web_page:value("custom","Custom address")

cust_addr=scc:option(Value,"cust_address",translate("Address"))
	cust_addr:depends("web_page","custom")

rs1 = scc:option(Value, "radiusserver1", translate("RADIUS server #1" ),
	translate("The IP address of the first RADIUS server that is to be used to authenticate your wireless clients"))
	rs1:depends("mode","extrad")


rs2 = scc:option(Value, "radiusserver2", translate("RADIUS server #2" ),
	translate("The IP address of the second RADIUS server that is to be used to authenticate your wireless clients"))
	rs2:depends("mode","extrad")


rap = scc:option(Value, "radiusauthport", translate("Authentication port" ),
	translate("RADIUS server authentication port"))
	rap.datatype = "port"
	rap.default = "1812"
	rap:depends("mode","extrad")

ras = scc:option(Value, "radiusacctport", translate("Accounting port" ),
	translate("RADIUS server accounting port"))
	ras.datatype = "port"
	ras.default = "1813"
	ras:depends("mode","extrad")

-- hnm = scc:option( Value, "hotspotname", translate("Hotspot name" ), translate("The name of your hotspot. Will appear on the login screen"))

rcp = scc:option(Value, "radiussecret", translate("Radius secret key" ),
	translate("The secret key is used for authentication with the RADIUS server. Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:;[]"))
	rcp.password = true
	rcp:depends("mode","extrad")
	rcp.datatype = "password"

uam_port = scc:option(Value, "uamport", translate("UAM port" ),
	translate("Port to bind for authenticating clients "))
	uam_port.datatype = "port"
	uam_port.default="3990"
	uam_port:depends("mode","extrad")

uam_ui_port = scc:option(Value, "uamuiport", translate("UAM UI port" ),
	translate("HotSpot UAM 'UI' Port (on subscriber network, for embedded portal)"))
	uam_ui_port.datatype = "port"
	uam_ui_port.default="4990"
	uam_ui_port:depends("mode","extrad")

uam = scc:option(Value, "uamsecret", translate("UAM secret" ),
	translate("Shared secret between uamserver and hotspot. Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:;[]"))
	uam.password = true
	uam:depends("mode","extrad")
	uam.datatype = "password"

nasid = scc:option(Value, "nasid", translate("NAS Identifier" ),
	translate("NAS Identifier"))
	nasid:depends("mode","extrad")

swapoctets = scc:option(Flag, "swapoctets", translate("Swap octets" ),
	translate("Swap the meaning of input octets and output octets as it related to RADIUS attribtues"))
	swapoctets:depends("mode","extrad")

location_name = scc:option(Value, "locationname", translate("Location name" ))
	location_name:depends("mode","extrad")

etern = scc:option(Flag, "externalpage", translate("External landing page" ),
	translate("Use external landing page"))
	etern:depends("mode","extrad")
	etern:depends("mode","intrad")
	etern:depends("mode","norad")

landing = scc:option(Value, "externadress", translate("Landing page address" ),
	translate("External landing page address (http://www.example.com)"))
	landing:depends({externalpage = "1", mode = "extrad"})
	landing:depends({externalpage = "1", mode = "intrad"})
	landing:depends({externalpage = "1", mode = "norad"})

	function landing:validate(value)
		if not value then
			m.message = "err: Landing page address is a required field."
			return nil
		end
		local find = string.find(value:sub(0,8), "http://") or string.find(value:sub(0,8), "https://")
		if not find then
			m.message = "err: Landing page address must contain protocol (http:// or https://)"
			return nil
		end
		return value
	end

	function landing.parse(self, section, value)
		local mode = (luci.http.formvalue("cbid.coovachilli." .. arg[1] .. ".mode") or "")
		if mode == "norad" or mode == "intrad" or mode == "extrad" then
			local fvalue = self:formvalue(section)
			if fvalue and #fvalue == 0 then
				local _, val_err = self:validate(nil, section)
				self:add_error(section, "missing", val_err)
			end
			AbstractValue.parse(self, section, value)
		end
end

o = scc:option(Value, "success_url", translate("Success URL" ), translate("URL for user redirection after successful authentication (http://www.example.com)"))
	o:depends({mode = "extrad"})
	o:depends({mode = "intrad"})
	o:depends({mode = "norad"})

addvert = scc:option(Value, "addvert_address", translate("Advertisement address" ), translate("Advertisement address (http://www.example.com)"))
	addvert:depends("mode","add")

	function addvert:validate(value)
		if not value then
			m.message = "err: Advertisement address is a required field."
			return nil
		end
		local find = string.find(value:sub(0,8), "http://") or string.find(value:sub(0,8), "https://")
		if not find then
			m.message = "err: Advertisement address must contain protocol (http:// or https://)"
			return nil
		end
		return value
	end

	function addvert.parse(self, section, value)
		if (luci.http.formvalue("cbid.coovachilli." .. arg[1] .. ".mode") or "") == "add" then
			local fvalue = self:formvalue(section)
			if fvalue and #fvalue == 0 then
				local _, val_err = self:validate(nil, section)
				self:add_error(section, "missing", val_err)
			end
			AbstractValue.parse(self, section, value)
		end
	end

prot = scc:option(ListValue, "protocol", translate("Protocol"),
	translate("Protocol to be used for landing page"))
	prot.template = "chilli/profile"
	prot:value( "http", translate("HTTP"))
	prot:value( "https", translate("HTTPS"))
	prot.default = "http"
	prot:depends({mode = "extrad", externalpage = ""})
	prot:depends({mode = "intrad", externalpage = ""})
	prot:depends({mode = "norad", externalpage = ""})
	prot:depends({mode = "sms", externalpage = ""})
	prot:depends({mode = "mac", externalpage = ""})
	uhttpd_https = m.uci:get("uhttpd", "main", "redirect_https")
	if uhttpd_https == "1" then
		prot.hardDisabled = true
		prot.info = "Redirect to HTTPS enabled"
		prot.url = dsp.build_url('admin/system/admin/access_control')
	end

https = scc:option(Flag, "https", translate("HTTPS to landing page redirect"),
	translate("Redirect initial pre-landing page HTTPS requests to hotspot landing page"))

key = scc:option(FileUpload, "sslkeyfile", translate("SSL key file"), translate(""))
	key:depends("https","1")

cert = scc:option(FileUpload, "sslcertfile", translate("SSL certificate file"), translate(""))
	cert:depends("https","1")

custom_dns = scc:option(Flag, "custom_dns", translate("Use custom DNS"), translate(""))

dns1 = scc:option(Value, "dns1", translate("DNS server 1"), translate(""))
	dns1:depends("custom_dns","1")

dns2 = scc:option(Value, "dns2", translate("DNS server 2"), translate(""))
	dns2:depends("custom_dns","1")

ses = m:section(TypedSection, "session", translate("Session Settings"))
	ses.id = hotspot_id
	ses.addremove = true
	ses.anonymous = true
	ses.template  = "cbi/tblsection_custom"
	ses.novaluetext = "There are no templates created yet."
	ses.extedit   = dsp.build_url("admin/services/hotspot/session_edit/%s")
	ses.redirect = true
	ses.addfields = {
		{title = "Template name", type = "text", class="cbi-input-text", style="margin-left: 10px;",
		name="_newinput.seestion.template", id="template_input", maxlength="32", onchange="custom_valid(this, /^[a-zA-Z0-9_]+$/)"}
	}

	function ses.cfgsections(self, section)
		local sections = {}

		self.map.uci:foreach(self.map.config, self.sectiontype, function (section)
				if self:checkscope(section[".name"]) and section.id and section.id == hotspot_id then
					table.insert(sections, section[".name"])
				end
			end)

		return sections
	end

	function ses.remove_button(self, section, k)
		local disabled = ""

		if section and not section:find("unlimited") then
			return [[<input class="cbi-button cbi-button-remove" type="submit" value="Delete"  onclick="this.form.cbi_state='del-section'; return true" name="cbi.rts.]] .. self.config .. [[.]] .. k .. [[" alt="Delete" title="Delete" />]]
		end
	end

	function ses.create(self, section)
		local template_exists = false
		local name = m:formvalue("_newinput.seestion.template") or ""

		if name ~= "" then
			m.uci:foreach(self.config, "session", function(s)
				if s.name == name and s.id == hotspot_id then
					template_exists=true
				end
			end)

			if not template_exists then
				local random_section = "sec" .. os.time()
				local created = TypedSection.create(self, random_section)
				self.map:set(random_section, "name",   name)
				self.map:set(random_section, "id", hotspot_id)
			else
				m.message = translate("err: Template is already \"" .. name .. "\" exists")
			end
		else
			m.message = translate("err: Template name is empty")
		end
	end

	function ses.remove(self, section)
		--Get default template
		local id = string.match (hotspot_id, "%d+") or "1"
		local default_tmpl = "unlimited" .. id

		--Set default template
		self.map.uci:foreach(self.config, "users", function(s)
			if s.template == section then
				self.map:set(self.config, s[".name"], "template", default_tmpl)
			end
		end)

		--self.map.uci:commit(self.config)
		self.map.proceed = true
		return self.map:del(section)
	end

uli = ses:option(DummyValue, "name", translate("Name"))

download_band = ses:option(DummyValue, "downloadbandwidth", translate("Download bandwidth"),
	translate("The max allowed download speed"))

	function download_band.cfgvalue(self, section)
		local unit_value = m.uci:get(self.config, section, "d_bandwidth_unit") or "kb"
		local multiplier, unit
		if unit_value == "mb" then
			multiplier = 1000000
			unit = "Mbit"
		elseif unit_value == "gb" then
			multiplier = 1000000000
			unit = "Gbit"
		else
			multiplier = 1000
			unit = "Kbit"
		end
		local value = m.uci:get(self.config, section, self.option)
		value = value and tonumber(value) / multiplier or nil

		return value and string.format("%s %s/s", value, unit) or "Unlimited"
	end

upload_band = ses:option(DummyValue, "uploadbandwidth", translate("Upload bandwidth"),
	translate("The max allowed upload speed"))

	function upload_band.cfgvalue(self, section)
		local unit_value = m.uci:get(self.config, section, "u_bandwidth_unit") or "kb"
		local multiplier, unit
		if unit_value == "mb" then
			multiplier = 1000000
			unit = "Mbit"
		elseif unit_value == "gb" then
			multiplier = 1000000000
			unit = "Gbit"
		else
			multiplier = 1000
			unit = "Kbit"
		end
		local value = m.uci:get(self.config, section, self.option)
		value = value and tonumber(value) / multiplier or nil

		return value and string.format("%s %s/s", value, unit) or "Unlimited"
	end

downloadlimit = ses:option(DummyValue, "downloadlimit", translate("Download limit"),
	translate("Disable hotspot user after download limit value in MB is reached"))

	function downloadlimit.cfgvalue(self, section)
		local value = m.uci:get(self.config, section, self.option)
		value = value and tonumber(value) / 1048576 or nil

		return value and string.format("%s MB", value) or "Unlimited"
	end

uploadlimit = ses:option(DummyValue, "uploadlimit", translate("Upload limit"),
	translate("Disable hotspot user after upload limit value in MB is reached"))

	function uploadlimit.cfgvalue(self, section)
		local value = m.uci:get(self.config, section, self.option)
		value = value and tonumber(value) / 1048576 or nil

		return value and string.format("%s MB", value) or "Unlimited"
	end

period = ses:option(DummyValue, "period", translate("Period"),
	translate("Period for which hotspot data limiting should apply"))

	function period.cfgvalue(self, section)
		local period = {"Day", "Week", "Month"}
		local value = m.uci:get(self.config, section, self.option)

		return period[tonumber(value)] or "-"
	end

sc1 = m:section(TypedSection, "users", translate("Users Configuration"))
	sc1.id = hotspot_id
	sc1.addremove = true
	sc1.anonymous = true
	sc1.template  = "chilli/tblsection_hotspot"
	sc1.novaluetext = "There are no users created yet."
	sc1.extedit   = dsp.build_url("admin/services/hotspot/user_edit/%s")


	function sc1.cfgsections(self, section)
		local sections = {}

		self.map.uci:foreach(self.map.config, self.sectiontype, function (section)
				if self:checkscope(section[".name"]) and section.id and section.id == hotspot_id then
					table.insert(sections, section[".name"])
				end
			end)

		return sections
	end

	function sc1.create(self, section)
		local user_exists = false
		local name = m:formvalue("_newinput.username") or ""
		local pass = m:formvalue("_newinput.pass") or ""
		local template_id = m:formvalue("_user.seestion.template") or ""

		if template_id ~= "" then
			m.uci:foreach(self.config, "users", function(s)
				if s.username == name and s.id == hotspot_id then
					user_exists=true
				end
			end)

			if not user_exists then
				local created = TypedSection.create(self, section)
				self.map:set(created, "template",	template_id)
				self.map:set(created, "username",   name)
				self.map:set(created, "password", pass)
				self.map:set(created, "id", hotspot_id)
			else
				m.message = translate("err: User \"" .. name .. "\" exists")
			end
		else
			m.message = translate("err: Session template not selected")
		end
	end

user = sc1:option(DummyValue, "username", translate("User name"),
	translate("Names of authorized users which will have the right to use wireless hotspot"))

	function user.parse(self, section, novld) --Custominis parse reiklaingas, kad saugand  neistrintu pasleptu sekciju
		local fvalue = self:formvalue(section)
		local cvalue = self:cfgvalue(section)

		-- If favlue and cvalue are both tables and have the same content
		-- make them identical
		if type(fvalue) == "table" and type(cvalue) == "table" then
			local equal = #fvalue == #cvalue
			if equal then
				for i=1, #fvalue do
					if cvalue[i] ~= fvalue[i] then
						equal = false
					end
				end
			end
			if equal then
				fvalue = cvalue
			end
		end

		if fvalue and #fvalue > 0 then -- If we have a form value, write it to UCI
			local val_err
			fvalue, val_err = self:validate(fvalue, section)
			fvalue = self:transform(fvalue)

			if not fvalue and not novld then
				self:add_error(section, "invalid", val_err)
			end

			if fvalue and (self.forcewrite or not (fvalue == cvalue)) then
				if self:write(section, fvalue) then
					-- Push events
					self.section.changed = true
					--luci.util.append(self.map.events, self.events)
				end
			end
		end
	end


	function user.write(self, section, value)
		if value and value ~= "" then
			m.uci:set(self.config, section, self.option, value)
			m.uci:set(self.config, section, "id", hotspot_id)
		end
	end
pass = sc1:option(DummyValue, "password", translate("Password"),
	translate("Passwords of authorized users which will have the right to use wireless hotspot"))
	pass.password = true

	function pass.parse(self, section, novld) --Custominis parse reiklaingas, kad saugand  neistrintu pasleptu sekciju
		local fvalue = self:formvalue(section)
		local cvalue = self:cfgvalue(section)

		-- If favlue and cvalue are both tables and have the same content
		-- make them identical
		if type(fvalue) == "table" and type(cvalue) == "table" then
			local equal = #fvalue == #cvalue
			if equal then
				for i=1, #fvalue do
					if cvalue[i] ~= fvalue[i] then
						equal = false
					end
				end
			end
			if equal then
				fvalue = cvalue
			end
		end

		if fvalue and #fvalue > 0 then -- If we have a form value, write it to UCI
			local val_err
			fvalue, val_err = self:validate(fvalue, section)
			fvalue = self:transform(fvalue)

			if not fvalue and not novld then
				self:add_error(section, "invalid", val_err)
			end

			if fvalue and (self.forcewrite or not (fvalue == cvalue)) then
				if self:write(section, fvalue) then
					-- Push events
					self.section.changed = true
					--luci.util.append(self.map.events, self.events)
				end
			end
		end
	end

idle = sc1:option(DummyValue, "defidletimeout", translate("Idle timeout" ),
	translate("Max idle time in sec. (0, meaning unlimited)"))
	--idle.default = "600"
	idle.datatype = "integer"

	function idle.cfgvalue(self, section)
		local template = self.map:get(section, "template")

		if not template then return "-" end

		local value = m.uci:get(self.config, template, self.option) or "0"
		if value == "0" then
			return "Unlimited"
		else
			return value .. " sec."
		end
	end

timeout = sc1:option(DummyValue, "defsessiontimeout", translate("Session timeout" ),
	translate("Max session time in sec. (0, meaning unlimited)"))
	--timeout.default = "600"
	timeout.datatype = "integer"

	function timeout.cfgvalue(self, section)
		local template = self.map:get(section, "template")

		if not template then return "-" end

		local value = m.uci:get(self.config, template, self.option) or "0"
		if value == "0" then
			return "Unlimited"
		else
			return value .. " sec."
		end
	end

download_band = sc1:option(DummyValue, "downloadbandwidth", translate("Download bandwidth"),
	translate("The max allowed download speed" ))
	download_band.datatype = "integer"

	function download_band.cfgvalue(self, section)
		local template = self.map:get(section, "template")

		if not template then return "-" end

		local value = m.uci:get(self.config, template, self.option) or "0"
		if value == "0" then
			value = "Unlimited"
		else
			local tail
			local measure = {"bit/s", "Kbit/s", "Mbit/s", "Gbit/s"}
			value = tonumber(value)
			for i, n in pairs(measure) do
				tail = n
				if value >= 1000 and n ~= "Gbit/s" then
					value = value / 1000
				else
					break
				end
			end
			value = string.format("%s %s", round(value, 3), tail)
		end
		return value
	end

upload_band = sc1:option(DummyValue, "uploadbandwidth", translate("Upload bandwidth"),
	translate("The max allowed upload speed" ))
	upload_band.datatype = "integer"

	function upload_band.cfgvalue(self, section)
		local template = self.map:get(section, "template")

		if not template then return "-" end

		local value = m.uci:get(self.config, template, self.option) or "0"
		if value == "0" then
			value = "Unlimited"
		else
			local tail
			local measure = {"bit/s", "Kbit/s", "Mbit/s", "Gbit/s"}
			value = tonumber(value)
			for i, n in pairs(measure) do
				tail = n
				if value >= 1000 and n ~= "Gbit/s" then
					value = value / 1000
				else
					break
				end
			end
			value = string.format("%s %s", round(value, 3), tail)
		end
		return value
	end

template = sc1:option(ListValue, "template", "Session template")

	function template.parse(self, section, novld)
		local fvalue = self:formvalue(section)
		local exists = false
		m.uci:foreach("coovachilli", "session", function(s)
			if fvalue == s[".name"] then
				exists = true
			end
		end)
		if not exists then
			local id = string.match (hotspot_id, "%d+") or "1"
			fvalue = "unlimited" .. id
		end
		if fvalue then
			if self:write(section, fvalue) then
				self.section.changed = true
			end
		end
	end

	m.uci:foreach(template.config, "session", function(sec)
		if sec.id and sec.id == hotspot_id then
			if sec.name then
				template:value(sec[".name"], sec.name)
			end
		end
	end)

local allowed = m:section(TypedSection, "uamallowed", translate("Walled garden"),
	translate("List Of Addresses The Client Can Access Without First Authenticating"))
allowed.addremove = true
allowed.anonymous = true
allowed.template  = "cbi/tblsection"
allowed.novaluetext = "There are no addresses created yet."
allowed.defaults = { enabled = 1 }
allowed.cfgsections = function (self)
	local sections = {}
	self.map.uci:foreach(self.config, self.sectiontype, function(s)
		if s.instance and s.instance == hotspot_id then
			table.insert(sections, s[".name"])
		end
	end)

	return sections
end

function allowed.create(self, section)
	local created = TypedSection.create(self, section)
	self.map:set(created, "instance", hotspot_id)
end

allowed:option( Flag, "enabled", translate("Enable"))

allowed:option( Value, "domain", translate("Address"),
	translate("Domain name, IP address or network segment"))

local port = allowed:option( Value, "port", translate("Port"))
port.datatype = "port"
port:depends("subdomains", "")

allowed:option( Flag, "subdomains", translate("Allow subdomains"))

--If hotspot enabled, remove wlan from bridge with lan and via versa
function m.on_commit()
	local wlan_bridge = "lan"
	local commit = false

	m.uci:foreach("coovachilli", "general", function(s)
		if s.enabled and s.enabled == "1" then
			wlan_bridge = ""
		end

		m.uci:foreach("wireless", "wifi-iface", function(ws)
			if ws.hotspotid and s[".name"] == ws.hotspotid then
				network = ws.network or ""

				if network ~= wlan_bridge then
					m.uci:set("wireless", ws[".name"], "network", wlan_bridge)
					commit = true
				end
			end
		end)

		wlan_bridge = "lan"
	end)

	if commit then
		m.uci:commit("wireless")
	end

	response=luci.sys.exec("/sbin/wifi up; echo $?")
end

return m
