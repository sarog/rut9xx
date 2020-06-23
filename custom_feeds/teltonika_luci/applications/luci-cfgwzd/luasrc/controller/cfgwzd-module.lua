module("luci.controller.cfgwzd-module", package.seeall)

function index()
	local translatef = luci.i18n.translatef
	local order	-- Used to indicate the step number
	order = 1

	entry({"admin", "system", "wizard"}, firstchild(), "Setup Wizard" , 1).dependent=false

	entry({"admin", "system", "wizard", "step-pwd"}, cbi("cfgwzd-module/step_pwd"), translatef("Step %d - General", order), 10)
	order = order + 1

	if luci.tools.status.show_mobile() then
		entry({"admin", "system", "wizard", "step-mobile"}, cbi("cfgwzd-module/step_3g"), translatef("Step %d - Mobile ", order), 20)
		order = order + 1
	end

	entry({"admin", "system", "wizard", "step-lan"}, cbi("cfgwzd-module/step_lan"),translatef("Step %d - LAN", order),  30)
	order = order + 1

	entry({"admin", "system", "wizard", "step-wifi"}, cbi("cfgwzd-module/step_wifi"), translatef("Step %d - WiFi", order), 40)
	entry({"admin", "system", "wizard", "apply_lan"}, call("apply_lan_and_dhcp_changes")).leaf = true
	entry({"admin", "system", "wizard", "calcIP"}, call("calculate_ip_range")).leaf = true
	order = order + 1

	entry({"admin", "system", "wizard", "step-rms"}, cbi("cfgwzd-module/step_rms"), translatef("Step %d - RMS", order), 50)
	entry({"admin", "system", "wizard", "enable_RMS"}, call("enable_RMS"), nil, nil)
end

function fileExists(path, name)
	local string = "ls ".. path
	local h = io.popen(string)
	local t = h:read("*all")
	h:close()

	for i in string.gmatch(t, "%S+") do
		if i == name then
			return 1
		end
	end
end

function split_by_newline(string)
	result = {}
	for match in string:gmatch("([^\n]+)") do
		table.insert(result, match)
	end
	return result
end

function calculate_ip_range()
	local address = luci.http.formvalue("address")
	local mask = luci.http.formvalue("mask")
	local start = luci.http.formvalue("start")
	local limit = luci.http.formvalue("limit")
	local information = split_by_newline(luci.sys.exec("ipcalc.sh " .. address .. " " .. mask .. " " .. start .. " " .. limit))
	local ip_range = {
		start_range = information[6]:gsub("START=", '') or "Not available",
		end_range = information[7]:gsub("END=", '') or "Not available"
	}
	luci.http.prepare_content("application/json")
	luci.http.write_json(ip_range)
end

function get_redirection_link(ip_address, protocol)
	protocol = (protocol:find("https")) and "https://" or "http://"
	local wifi = luci.dispatcher.build_url("admin/system/wizard/step-wifi")
	return protocol .. ip_address .. wifi
end

function apply_lan_and_dhcp_changes()
	local uci = require("uci").cursor()
	local lan_ip_address = luci.http.formvalue("ipaddr")
	local protocol = luci.http.formvalue("protocol")
	local host = luci.http.formvalue("current_host")
	uci:commit("network")
	uci:commit("dhcp")
	if host == luci.util.trim(luci.sys.exec("ip addr show dev br-lan | grep inet | cut -d' ' -f6 | cut -d'/' -f1")) then
		if lan_ip_address and lan_ip_address ~= host and protocol then
			local redirection_address = get_redirection_link(lan_ip_address, protocol)
			luci.http.prepare_content("application/json")
			luci.http.write_json({link = redirection_address})
			luci.sys.exec("luci-reload &")
			return
		end
	end
	luci.sys.exec("luci-reload &")
	luci.http.prepare_content("application/json")
	luci.http.write_json({link = luci.dispatcher.build_url("admin/system/wizard/step-wifi")})
	return
end

function enable_RMS()
	local uci = require("uci").cursor()
	local enabled = luci.http.formvalue("is_enabled")
	if enabled == "1" or enabled == "2" then
		local vpn_enabled = enabled
		if vpn_enabled == "2" then
			vpn_enabled = "1"
		end
		uci:set("rms_connect", "rms_connect", "enable", enabled)
		uci:commit("rms_connect")

		uci:set("rms_connect_mqtt", "rms_connect_mqtt", "enable", enabled)
		uci:commit("rms_connect_mqtt")

		luci.sys.call("/etc/init.d/rms_connect restart")
	end
	local rv = {}
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
	return
end
