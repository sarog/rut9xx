module("luci.controller.admin.network", package.seeall)

local uci = require("luci.model.uci").cursor()

function index()
	local uci = require("luci.model.uci").cursor()
	local show = require("luci.tools.status").show_mobile()
	local dual_sim = uci:get("hwinfo", "hwinfo", "dual_sim")
	local usb_module_name = uci:get("network", "ppp_usb", "ifname")
	local page

	page = node("admin", "network")
	page.target = firstchild()
	page.title  = _("Network")
	page.order  = 30
	page.index  = true

	local has_wifi = false

	uci:foreach("wireless", "wifi-device",
		function(s)
			has_wifi = true
			return false
		end)

	if has_wifi then
		local staConfigPresent = false

		page = entry({"admin", "network", "wireless_join"}, call("wifi_join"), nil)
		page.leaf = true
		page = entry({"admin", "network", "wireless_add"}, call("wifi_add"), nil)
		page.leaf = true
		page = entry({"admin", "network", "wireless_delete"}, call("wifi_delete"), nil)
		page.leaf = true
		page = entry({"admin", "network", "wireless_delete_all"}, call("wifi_delete_all"), nil)
		page.leaf = true
		page = entry({"admin", "network", "wireless_status"}, call("wifi_status"), nil)
		page.leaf = true
		page = entry({"admin", "network", "wireless_reconnect"}, call("wifi_reconnect"), nil)
		page.leaf = true
		page = entry({"admin", "network", "wireless_shutdown"}, call("wifi_reconnect"), nil)
		page.leaf = true
		page = entry({"admin", "network", "wireless_scan"}, template("admin_network/wifi_join"), nil)
		page = entry({"admin", "network", "wireless"}, arcombine(template("admin_network/wifi_overview"), cbi("admin_network/wifi")), _("Wireless"), 16)
		page.leaf = true
		page.subindex = true
		entry({"admin", "network", "multi_ap"}, cbi("admin_network/multiple_ap"), nil, nil)

		uci:foreach("wireless", "wifi-iface", function(s)
			if s.mode == "sta" then
				staConfigPresent = true
			end
		end)
	end

	if has_wifi then
		entry({"admin", "network", "wireless_join"}, call("wifi_join"), nil).leaf = true
		entry({"admin", "network", "wireless_add"}, call("wifi_add"), nil).leaf = true
		entry({"admin", "network", "wireless_delete"}, call("wifi_delete"), nil).leaf = true
		entry({"admin", "network", "wireless_delete_all"}, call("wifi_delete_all"), nil).leaf = true
		entry({"admin", "network", "wireless_status"}, call("wifi_status"), nil).leaf = true
		entry({"admin", "network", "wireless_reconnect"}, call("wifi_reconnect"), nil).leaf = true
		entry({"admin", "network", "wireless_shutdown"}, call("wifi_reconnect"), nil).leaf = true
		entry({"admin", "network", "wireless_scan"}, template("admin_network/wifi_join"), nil).leaf = true
		page = entry({"admin", "network", "wireless"},
			arcombine(template("admin_network/wifi_overview"), cbi("admin_network/wifi")), _("Wireless"), 16)
		page.leaf = true
		page.subindex = true
	end

	entry({"admin", "network", "iface_disabled"}, call("iface_disabled"), nil).leaf = true

	if show then
		entry({"admin", "network", "mobile"}, alias("admin", "network", "mobile","general"), _("Mobile"), 10)
	end

	entry({"admin", "network", "mobile","general"}, cbi("admin_network/ifaces_mobile"), _("General"), 1).leaf = true
	if dual_sim == "1" then
		entry({"admin", "network", "mobile","sim_switch"}, cbi("admin_network/sim_switch"), _("SIM Management"), 2).leaf = true
	end

	entry({"admin", "network", "mobile","operators"}, alias("admin", "network", "mobile", "operators", "scan"),
		_("Network Operators"), 3)
	entry({"admin", "network", "mobile","operators", "scan"}, template("admin_network/operators_list"),
		_("Network Operators"), 1).leaf = true
	entry({"admin", "network", "mobile","operators", "list"}, cbi("admin_network/operators_list"),
		_("Operators List"), 2).leaf = true
	entry({"admin", "network", "mobile","limit"}, cbi("admin_network/data_limit"),
		_("Mobile Data Limit"), 4).leaf = true
	entry({"admin", "network", "mobile", "sms_limit"}, cbi("admin_network/sms_limit"), _("SMS Limit"), 6).leaf = true
	entry({"admin", "network", "mobile", "sim_idle_protection"},
		alias("admin", "network", "mobile", "sim_idle_protection", "settings"), _("SIM Idle Protection"), 7)
	entry({"admin", "network", "mobile", "sim_idle_protection", "settings"}, cbi("admin_network/sim_idle_protection"),
		_("Settings"), 1).leaf = true
	entry({"admin", "network", "mobile", "sim_idle_protection", "test"},
		template("admin_network/sim_idle_protection_test"), _("Test"), 2).leaf = true

	if usb_module_name == "wwan-usb0" then
		entry({"admin", "network", "mobile","usb_module"}, cbi("admin_network/usb_modem"), _("USB Modem"), 8).leaf = true
	end
	page = entry({"admin", "network", "wan"}, cbi("admin_network/ifacesWan"), _("WAN"), 11)
		entry({"admin", "network", "wan", "edit"}, cbi("admin_network/wanEdit")).leaf = true
	page = entry({"admin", "network", "lan"}, cbi("admin_network/ifacesLan"), _("LAN"), 12)

	page  = node("admin", "network", "routes")
	page.target = cbi("admin_network/routes")
	page.title  = _("Static Routes")
	page.order  = 55

	--===================================VLAN=====================================
	entry({"admin", "network", "vlan"}, alias("admin", "network", "vlan", "list"), _("VLAN"), 15)
	entry({"admin", "network", "vlan", "list"}, template("admin_network/vlan"), _("VLAN Networks"), 1).leaf = true
	entry({"admin", "network", "vlan", "lan"}, arcombine(cbi("admin_network/lan_list"), cbi("admin_network/ifacesLan")),
		_("LAN Networks"), 2).leaf = true

	entry({"admin", "network", "routes"}, alias("admin", "network", "routes", "static_routes"), _("Routing"), 61)
		entry({"admin", "network", "routes", "static_routes"}, cbi("admin_network/routes"),
			_("Static Routes"), 1).leaf = true
	entry({"admin", "network", "mobile", "clear_limit"}, call("clear_limit"), nil, nil)
	entry({"admin", "network", "wan", "xhr_the_data"}, call("get_ip"), nil, nil)

	-- SIM IDLE PROTECTION
	entry({"admin", "network", "mobile", "sim_idle_protection", "get_test_results"}, call("sim_idle_test"), nil, nil)
	entry({"admin", "network", "mobile", "sim_idle_protection", "switch"}, call("switch_sim"), nil, nil)
	entry({"admin", "network", "mobile", "sim_idle_protection", "enable"}, call("enable_mobile"), nil, nil)
	entry({"admin", "network", "mobile", "sim_idle_protection", "disable"}, call("disable_mobile"), nil, nil)
end

function disable_mobile()
	uci:set("network", "ppp", "enabled", "0")
	uci:set("network", "ppp", "disabled", "1")
	uci:commit("network")
	luci.sys.exec("luci-reload")
end

function enable_mobile()
	local was_enabled = "1";

	if uci:get("network", "ppp", "enabled") == "0" then
		uci:set("network", "ppp", "enabled", "1")
		uci:set("network", "ppp", "disabled", "0")
		uci:commit("network")
		luci.sys.exec("luci-reload")
		was_enabled = "0";
		luci.sys.exec("sleep 10") -- Wait for mobile interface to come up
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json({ was_enabled = was_enabled })
	return
end

function switch_sim()
	local sim = check_which_sim(luci.http.formvalue("step"), uci:get("simcard", "simcard", "default"))
	luci.sys.exec("/usr/sbin/sim_switch change " .. sim)
	local retry = 0
	while luci.sys.exec("/sbin/ifconfig " ..
			uci:get("network", "ppp", "ifname") .. " | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}'") == ""
			and retry < 30
	do
		retry = retry + 1
		luci.sys.exec("sleep 3")
	end
	luci.http.prepare_content("application/json")
	local rv = {  placeholder = "switched" }
	luci.http.write_json(rv)
	return
end

function sim_idle_ping(host, count, size, interface)
	local retry = 0
	while retry < 3 do
		local ping_exit = luci.sys.call("ping " .. host .. " -c " .. count .. " -s " .. size .. " -I " .. interface .. " >/dev/null 2>/dev/null") or "1"
		if ping_exit == 0 then return 1
		else retry = retry + 1 end
	end
	return 0
end

function check_which_sim(step, default_sim)
	if step == "1" then
		return default_sim
	elseif default_sim == "sim1" then
		return "sim2"
	end
	return "sim1"
end

function sim_result_array(sim, state, connection, imsi, iccid, wan, host, ping)
	if state == "inserted" and connection == "connected" then
		connection = "Connected"
		return { (sim or "-"), (connection or "-"), (imsi or "-"), (iccid or "-"), (wan or "-"), (host or "-"), (ping or "-") }
	else
		if imsi ~= "N/A" or iccid ~= "N/A" then state = "Disconnected"
		else state = "Not inserted" end
		ping = "Failed"
		return { (sim or "-"), (state or "-"), (imsi or "-"), (iccid or "-"), (wan or "-"), (host or "-"), (ping or "-") }
	end
end

function sim_idle_test()
	local default_sim, sim, sim_state, sim_conn, imsi, iccid, host, wan, sim_ping, sim_iface, ping_count, ping_size
	default_sim = uci:get("simcard", "simcard", "default")
	sim_iface = uci:get("network", "ppp", "ifname") or "wwan0"
	sim = check_which_sim(luci.http.formvalue("step"), default_sim)
	host = uci:get("sim_idle_protection", sim, "host") or "127.0.0.1"
	ping_count = uci:get("sim_idle_protection", sim, "count") or "2"
	ping_size = uci:get("sim_idle_protection", sim, "packet_size") or "56"

	local data = luci.util.split(luci.sys.exec("gsmctl -z -j -x -J -p " .. sim_iface),"\n")
	sim_state = data[1]
	sim_conn = data[2]
	imsi = data[3]
	iccid = data[4]
	wan = data[5]

	if sim_idle_ping(host, ping_count, ping_size, sim_iface) == 1 then
		sim_ping = "Successful"
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json(sim_result_array(sim, sim_state, sim_conn, imsi, iccid, wan, host, sim_ping))
	return
end

function wifi_join()

	local function param(x)
		return luci.http.formvalue(x)
	end
	local function ptable(x)
		x = param(x)
		return x and (type(x) ~= "table" and { x } or x) or {}
	end
	local dev  = param("device")
	local ssid = param("join")
	local staConfigPresent = false
	local wan_wifi_enabled = false

	uci:foreach("network", "interface", function(sct)
		if sct.ifname and sct.ifname:match("wlan") then
			if not sct.enabled or (sct.enabled and sct.enabled == "1") then
				wan_wifi_enabled = true
			end
		end
	end)

	uci:foreach("wireless", "wifi-iface", function(s)
			if s.mode == "sta" then
				staConfigPresent = true
			end
		end)

	if dev and ssid then
		local cancel  = (param("cancel") or param("cbi.cancel")) and true or false
		if cancel then
			luci.http.redirect(luci.dispatcher.build_url("admin/network/wireless_join") .. "?scan=start")
		else
			local cbi = require "luci.cbi"
			local tpl = require "luci.template"
			local map = luci.cbi.load("admin_network/wifi_add")[1]
			map:parse()
			tpl.render("header")
			map:render()
			tpl.render("footer")
		end
	else
			luci.template.render("admin_network/wifi_join")
	end
end

function ip_generate()
	local frame = "192.168.%d.254/24"
	local sub = 2
	local ips = {}
	local ip = "192.168.2.254/24"

	uci:foreach("coovachilli", "general",
		function(s)
			b1, b2, b3, b4 = string.match(s.net, "(%d+).(%d+).(%d+).(%d+)")
			table.insert(ips, b3)
			for n, net in ipairs(ips) do
				if sub ==  tonumber(net) then
					sub = sub + 1
				end
			end
		end)
	ip = string.format(frame, sub)
	return ip
end

function get_id()
	local id =  1
	local exists = false
	while true do
		uci:foreach("coovachilli", "general",
			function(s)
				if id == tonumber(string.match(s[".name"], "%d+")) then
					id = id + 1
					exists = true
				end
		end)
		if not exists then
			return id
		end
		exists = false
	end
end

function wifi_add()
	local uci = require "luci.model.uci".cursor()
	local dev = luci.http.formvalue("device")
	local ntm = require "luci.model.network".init()
	local id = get_id()
	local hotspotid = "hotspot" .. id

	dev = dev and ntm:get_wifidev(dev)
	if dev then
		local wifi_ssid = luci.util.trim(luci.sys.exec("brand 21"))
		local net = dev:add_wifinet({
			mode       = "ap",
			network    = "lan",
			ssid       = wifi_ssid,
			encryption = "none",
			hotspotid = hotspotid
		})

		ntm:save("wireless")
		uci:section("coovachilli", "general", hotspotid, {
			enabled = "0",
			mode = "norad",
			protocol = "http",
			net = ip_generate()
		})
		uci:section("coovachilli", "session", "unlimited" .. id, {
			name = 'unlimited',
			id = hotspotid,
			uamlogoutip = "1.1.1.1"
		})
		--Add scheduler config section for this wifi
		uci:section("hotspot_scheduler", "ap", hotspotid, {
			restricted = 0
		})

		uci:save("hotspot_scheduler")
		uci:commit("hotspot_scheduler")
		uci:save("coovachilli")
		uci:commit("coovachilli")
		luci.http.redirect(net:adminlink())
	end
end

function wifi_delete()
	local uci = require "luci.model.uci".cursor()
	local rv={}
	local network = luci.http.formvalue("status")
	if network then
		--luci.sys.exec("/etc/init.d/chilli stop &")
		local hotspot_id = uci:get("wireless", network, "hotspotid")
		if hotspot_id then
			uci:delete("hotspot_scheduler", hotspot_id)
			uci:commit("hotspot_scheduler")
			uci:delete("coovachilli", hotspot_id)
			uci:delete("overview","show",hotspot_id)
			uci:foreach("coovachilli", "users",
				function(c)
					if c.id == hotspot_id then
						uci:delete("coovachilli", c[".name"])
					end
			end)
			uci:foreach("coovachilli", "session",
				function(c)
					if c.id == hotspot_id then
						uci:delete("coovachilli", c[".name"])
					end
			end)
			uci:foreach("coovachilli", "uamallowed",
				function(c)
					if c.instance == hotspot_id then
						uci:delete("coovachilli", c[".name"])
					end
			end)
			uci:commit("coovachilli")
			uci:commit("overview")
		end
		--luci.sys.call("wifi down")
		luci.sys.call("uci delete wireless.%q; uci commit" % network)
		--luci.sys.exec("/sbin/wifi up")
		--luci.sys.exec("/etc/init.d/firewall restart")
		--luci.sys.exec("/etc/init.d/chilli start &")
		luci.sys.exec("luci-reload")

		rv={
			response=1
		}
	end
		luci.http.prepare_content("application/json")
		luci.http.write_json(rv)
	return
end

function wifi_delete_all()
	local uci = require "luci.model.uci".cursor()
	local rv={}
	local status = luci.http.formvalue("status")

	if status then
		uci:foreach("wireless","wifi-iface",
				function(c)
					local hotspot_id = uci:get("wireless", c['.name'], "hotspotid")
					local network = c['.name']

					if hotspot_id then
						uci:delete("hotspot_scheduler", hotspot_id)
						uci:commit("hotspot_scheduler")
						uci:delete("coovachilli", hotspot_id)
						uci:delete("overview","show",hotspot_id)
						uci:foreach("coovachilli", "users",
							function(c)
								if c.id == hotspot_id then
									uci:delete("coovachilli", c[".name"])
								end
						end)
						uci:foreach("coovachilli", "session",
							function(c)
								if c.id == hotspot_id then
									uci:delete("coovachilli", c[".name"])
								end
						end)
						uci:foreach("coovachilli", "uamallowed",
							function(c)
								if c.instance == hotspot_id then
									uci:delete("coovachilli", c[".name"])
								end
						end)
						luci.sys.call("uci delete wireless.%q" % network)
					end
			end)
			luci.sys.call("uci commit")
			uci:commit("coovachilli")
			uci:commit("overview")
		luci.sys.exec("luci-reload")
		rv={
			response=1
		}
	end
		luci.http.prepare_content("application/json")
		luci.http.write_json(rv)
	return
end

function iface_disabled()
	local rv = {}
	local sid = luci.http.formvalue("status")
	if sid then
		local sta = uci:get("wireless", sid, "mode") == "sta" and true or false
		local user_enable = uci:get("wireless", sid, "user_enable")
		if user_enable ~= nil and user_enable ~= "1" then
			uci:delete("wireless", sid, "disabled")
			uci:set("wireless", sid, "user_enable", "1")
			luci.sys.exec("/sbin/wifi up");
		else
			uci:set("wireless", sid, "disabled", "1")
			uci:set("wireless", sid, "user_enable", "0")
			local hotspot_id = uci:get("wireless", sid ,"hotspotid")
			if hotspot_id then
				local hotspot_enb = uci:get("coovachilli", hotspot_id, "enabled")
				if hotspot_enb and hotspot_enb == "1" then
					uci:set("coovachilli", hotspot_id, "enabled", "0")
				end
				uci:commit("coovachilli")
			end
		end
		uci:commit("wireless")
		rv = {
			response = 1
		}
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
	if rv.response == 1 then
		luci.sys.exec("luci-reload wireless &")
	end
	return
end

function wifi_status()
	local path = luci.dispatcher.context.requestpath
	local s    = require "luci.tools.status"
	local rv   = { }

	local dev
	for dev in path[#path]:gmatch("[%w%.%-]+") do
		rv[#rv+1] = s.wifi_network(dev)
	end

	if #rv > 0 then
		luci.http.prepare_content("application/json")
		luci.http.write_json(rv)
		return
	end

	luci.http.status(404, "No such device")
end

function wifi_reconnect()
	local path  = luci.dispatcher.context.requestpath
	local mode  = path[#path-1]
	local wnet  = path[#path]
	local netmd = require "luci.model.network".init()

	local net = netmd:get_wifinet(wnet)
	local dev = net:get_device()
	if dev and net then
		luci.sys.call("env -i /sbin/wifi down >/dev/null 2>/dev/null")

		dev:set("disabled", nil)
		net:set("disabled", (mode == "wireless_shutdown") and 1 or nil)
		netmd:commit("wireless")

		luci.sys.call("env -i /sbin/wifi up >/dev/null 2>/dev/null")
		luci.http.status(200, (mode == "wireless_shutdown") and "Shutdown" or "Reconnected")

		return
	end

	luci.http.status(404, "No such radio")
end

function get_ip()
	local ntm = require "luci.model.network".init()
	local network
	local value, wan, wan2, wan3
	local intf
	local data = { ipaddrs = { } }
	local ppp_method = uci:get("network", "ppp", "method") or "nat"
	local ppp_ifname = uci:get("network", "ppp", "ifname") or nil
	-- WAN section --
	network = ntm:get_network("wan")

	if network:ifname() then
		intf = ntm:get_interface(network:ifname())

		if intf and (ppp_method == "nat" or network:ifname() ~= ppp_ifname) then
			for _, a in ipairs(intf:ipaddrs()) do
				data.ipaddrs[#data.ipaddrs+1] = {
					addr = a:host():string()
				}
				value=data.ipaddrs[1].addr
			end
		else
			if ppp_method == "bridge" then
				value = "- (Bridge mode)"
			elseif ppp_method == "pbridge" then
				value = "- (Passthrough mode)"
			end
		end
	end

	wan = not value and "-" or value
	-- WAN2 section --
	value = nil
	data = { ipaddrs = { } }
	network = ntm:get_network("wan2")
	if network:ifname() then
		intf = ntm:get_interface(network:ifname())
		if intf then
			for _, a in ipairs(intf:ipaddrs()) do
				data.ipaddrs[#data.ipaddrs+1] = {
					addr      = a:host():string()
				}
				value=data.ipaddrs[1].addr
			end
		end
	end

	wan2 = not value and "-" or value
	-- WAN3 section --
	value = nil
	data = { ipaddrs = { } }
	network = ntm:get_network("wan3")
	if network:ifname() then
		intf = ntm:get_interface(network:ifname())
		if intf then
			for _, a in ipairs(intf:ipaddrs()) do
				data.ipaddrs[#data.ipaddrs+1] = {
					addr      = a:host():string()
				}
				value=data.ipaddrs[1].addr
			end
		end
	end

	wan3 = not value and "-" or value
	local rv = {
		wan_ip = wan,
		wan2_ip = wan2,
		wan3_ip = wan3
	}

	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end
