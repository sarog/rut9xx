
module("luci.controller.coovachilli", package.seeall)

function index()
	local uci = require("luci.model.uci").cursor()
	local listofssids = {}
	local listofids = {}

	uci:foreach("wireless", "wifi-iface", function(s)
		local arr = { id = s.hotspotid, ssid = s.ssid or "" }
		if s.hotspotid then
			if uci:get("coovachilli", s.hotspotid, "enabled") == "1" then
				table.insert(listofssids, s.ssid)
			end
			table.insert(listofids, arr)
		end
	end)

	entry( { "admin", "services", "hotspot" }, alias("admin", "services", "hotspot", "general"), _("Hotspot"), 90)
	entry({"admin", "services", "hotspot", "general"},
		arcombine(template("chilli/hotspot_overview"), cbi("coovachilli"), template("chilli/clients_managing")),
		_("General"), 1).leaf = true
	entry({"admin", "services", "hotspot", "user_edit"}, cbi("user_edit")).leaf = true
	entry({"admin", "services", "hotspot", "session_edit"}, cbi("session_edit")).leaf = true
	if listofssids[1] ~= nil then
		entry({"admin", "services", "hotspot", "statistics"},
			alias("admin", "services", "hotspot", "statistics", listofssids[1]), _("Statistics"), 7)

		for Index, Value in pairs( listofssids ) do
			entry({"admin", "services", "hotspot", "statistics", Value},
				template("chilli/statistics"), _(Value), Index).leaf = true
		end
	end

	if #listofids > 0 then
		entry({"admin", "services", "hotspot", "hotspot_scheduler"},
			alias("admin", "services", "hotspot", "hotspot_scheduler", listofids[1]["id"]),
			_("Restricted Internet Access"), 2)

		for Index, Value in pairs( listofids ) do
			entry({"admin", "services", "hotspot", "hotspot_scheduler", Value.id}, cbi("hotspot_scheduler"),
				_(Value.ssid), Index).leaf = true
		end
	else
		entry({"admin", "services", "hotspot", "hotspot_scheduler"}, template("hotspot_scheduler_nossid"),
			_("Restricted Internet Access"), 2).leaf = true
	end

	entry({"admin", "services", "hotspot", "loging"}, alias("admin", "services", "hotspot", "loging", "configuration"),
		_("Logging"), 3)
	entry({"admin", "services", "hotspot", "loging", "configuration"}, cbi("coovachilli_logging"),
		_("Configuration"), 1).leaf = true
	entry({"admin", "services", "hotspot", "loging", "wifilog"}, template("chilli/log"), _("Log"), 2).leaf = true
	entry({"admin", "services", "hotspot", "loging", "smsotplog"}, template("chilli/smsotplog"),
		_("SMS OTP Log"), 3).leaf = true
 	entry({"admin", "services", "hotspot", "landing"}, alias("admin", "services", "hotspot", "landing", "general"),
		_("Landing Page"), 4)
	entry({"admin", "services", "hotspot", "landing", "general"}, cbi("coovachilli_landing"),
		_("General"), 1).leaf = true
	entry({"admin", "services", "hotspot", "landing", "edit"}, cbi("coovachilli_landing_edit"),
		_("Template"), 2).leaf=true
	entry({"admin", "services", "hotspot", "url_parameters"}, cbi("url_parameters"), _("URL parameters"), 6).leaf=true

	entry({"admin", "services", "hotspot", "delete_mac"}, call("delete_mac")).leaf = true
	entry({"admin", "services", "hotspot", "onoff"}, call("enable_disable"), nil).leaf = true
	entry({"admin", "services", "hotspot", "logout"}, call("hotspot_logout"), nil).leaf = true
	entry({"admin", "services", "hotspot", "profile_config"}, call("profile_config"), nil).leaf = true
	entry({"admin", "services", "hotspot", "redirect"}, call("redirect_to"), nil).leaf = true

	entry({"admin", "services", "tmpldownload"}, call("tmpl_download"), nil, nil)
end

function redirect_to()
	local id = luci.http.formvalue("id")

	luci.http.redirect(luci.dispatcher.build_url("admin/services/hotspot/general/" .. id))
end

function tmpl_download()
	local uci = require("luci.model.uci").cursor()
	local path = uci:get("landingpage", "general", "loginPage") or "/etc/chilli/www/hotspotlogin.tmpl"
	--FIXME: I'm not sure about "cat"
	local reader = ltn12_popen("cat %s - 2>/dev/null" % path)
	luci.http.header('Content-Disposition', 'attachment; filename="hotspotlogin.tmpl"')
	luci.http.prepare_content("application/text")
	luci.ltn12.pump.all(reader, luci.http.write)
end

function hotspot_logout()
	local mac_addr = luci.http.formvalue("mac")
	local socket = luci.http.formvalue("socket")
	local response = logout(mac_addr, socket) or 0
	luci.http.prepare_content("application/json")
	luci.http.write_json({response = response})
end

function logout(mac_addr, socket)
	if mac_addr and socket then
		local command = string.format("/usr/sbin/chilli_query -s %s logout %s", socket, mac_addr)
		--FIXME: use luci.util.exec here
		local res = io.popen(command)
		if res and res:read("*all") == "" then
			return  1
		end
	end

	return 0
end

function enable_disable()
	local uci = require "luci.model.uci".cursor()
	local rv={}
	local section = luci.http.formvalue("sid")
	local config = "coovachilli"
	local option = "enabled"
	local response
	local wlan_bridge = "lan"
	local commit = false

	if section then
		local enabled = uci:get(config, section, option)
		if enabled ~= nil and enabled == "1" then
			uci:set(config, section, option, "0")
			response = 0
		else
			uci:set(config, section, option, "1")
			uci:set("firewall", "Hotspot_input", "enabled", "1")
			uci:commit("firewall")
			response = 1
		end

		uci:commit(config)

		--If hotspot enabled, remove wlan from bridge with lan and via versa
		uci:foreach(config, "general", function(s)
			if s.enabled and s.enabled == "1" then
				wlan_bridge = ""
			end

			uci:foreach("wireless", "wifi-iface", function(ws)
				if ws.hotspotid and s[".name"] == ws.hotspotid then
					network = ws.network or ""

					if network ~= wlan_bridge then
						uci:set("wireless", ws[".name"], "network", wlan_bridge)
						commit = true
					end
				end
			end)

			wlan_bridge = "lan"
		end)

		if commit then
			uci:commit("wireless")
		end


 		response = luci.sys.exec("/sbin/wifi up; echo $?")
		luci.sys.exec("/sbin/luci-reload")
		luci.sys.exec("/etc/init.d/chilli restart")
		rv = {
			response = response
		}
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)

	return
end

function delete_mac()
	require "teltonika_lua_functions"
	local sqlite = require "lsqlite3"
	local dbPath = "/var/"
	local dbName = "hotspot.db"
	local stat = 1
	local mac = luci.http.formvalue("mac")
	local ifname = luci.http.formvalue("ifname")
	local socket = string.format("/var/run/chilli.%s.sock", ifname)

	--FIXME: use fs.access instead

	if fileExists(dbPath, dbName) and mac and mac ~= "" then
		local db
		db = sqlite.open(dbPath .. dbName)

		if db then
			local query = "DELETE FROM statistics WHERE mac='" .. mac .. "'"
			stmt = db:prepare(query)

			if stmt then
				stmt:step()
				stmt:finalize()
				stat = 0
				logout(mac, socket)
			end
			closeDB(db)
		end
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json({stat = stat})
	return
end

function ltn12_popen(command)

	local fdi, fdo = nixio.pipe()
	local pid = nixio.fork()

	if pid > 0 then
		fdo:close()
		local close
		return function()
			local buffer = fdi:read(2048)
			local wpid, stat = nixio.waitpid(pid, "nohang")
			if not close and wpid and stat == "exited" then
				close = true
			end

			if buffer and #buffer > 0 then
				return buffer
			elseif close then
				fdi:close()
				return nil
			end
		end
	elseif pid == 0 then
		nixio.dup(fdo, nixio.stdout)
		fdi:close()
		fdo:close()
		nixio.exec("/bin/sh", "-c", command)
	end
end


--FIXME: use library functions instead of this
function write_to_file(file, mode, string)
	local outfile = io.open(file, mode)

	if outfile then
		outfile:write(string)
		outfile:close()

		return 0
	end

	return 1
end

function genrate_tmp(config, section_types)
	local named_sections = {genral = 1}
	local tmp_conf = ""
	local uci = require "luci.model.uci".cursor("/etc/chilli/configs", "/tmp/.uci")
	local arr = {}

	for i, type in ipairs(section_types) do
		uci:foreach(config, type, function(s)
			sign = "+"

			if type == "general" or type == "ftp" or type == "link" or type == "session" or type == "url" then
				sign = ""
			end

			tmp_conf = string.format("%s%scoovachilli.%s=%s\n", tmp_conf, sign, s[".name"], type)

			for name, value in pairs(s) do
				if name ~= ".name" and name ~= ".type" and name ~= ".anonymous" and name ~= ".index" then
					if value == true then
						value = 1
					elseif value == false then
						value = 0
					end

					tmp_conf = string.format("%scoovachilli.%s.%s=%s\n", tmp_conf, s[".name"], name, value)
				end
			end
		end)
	end

	return tmp_conf
end

function generate_del(section_types)
	local uci = require "luci.model.uci".cursor()
	local uci_string = ""

	for i, type in ipairs(section_types) do
		uci:foreach("coovachilli", type, function(s)
			uci_string = string.format("%s-coovachilli.%s=%s\n", uci_string, s[".name"], type)
		end)
	end

	return uci_string
end

function main_generate(tmp_conf, profile)
	local sec_types = {"general", "ftp", "interval", "link", "session", "uamallowed", "users", "url"}
	local uci_string = generate_del(sec_types)
	local res = write_to_file(tmp_conf, "w", uci_string)

	if res == 0 then
		local profile_conf = genrate_tmp(profile, sec_types)
		res = write_to_file(tmp_conf, "a", profile_conf)
	end

	return res
end

function profile_config()
	require "teltonika_lua_functions"
	local uci = require "luci.model.uci".cursor()
	local tmp_conf = "/tmp/.uci/coovachilli"
	local dir  = "/etc/chilli/configs/"
	local profile = luci.http.formvalue("profile")
	local section = luci.http.formvalue("section") or ""
	local res = 1

	if profile and profile == "custom" then
		res = write_to_file(tmp_conf, "w", "")
		local curr_profile = uci:get("coovachilli", section, "profile") or ""

		if profile ~= curr_profile then
			res = main_generate(tmp_conf, profile)
		end
	elseif profile and fileExists(dir, profile) then
		res = main_generate(tmp_conf, profile)
	else
		res = 2
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json({stat = res})

	return
end
