
module("luci.controller.admin.status", package.seeall)

local uci = require("luci.model.uci").cursor()
require "teltonika_lua_functions"

function index()
	local show = require("luci.tools.status").show_mobile()
	local uci = require("uci").cursor()
	local bus = require "ubus"
	local _ubus = bus.connect()
	local has_3g   = false
	local has_wimax= false
	local modelservice = "3G"
	local moduleVidPid = luci.util.trim(luci.sys.exec("uci get -q system.module.vid"))..":"..luci.util.trim(luci.sys.exec("uci get -q system.module.pid"))

	local moduleType = luci.util.trim(luci.sys.exec("uci get -q system.module.type"))
	local moduleIface = luci.util.trim(luci.sys.exec("uci get -q system.module.iface"))
	local speedtest = luci.util.trim(luci.sys.exec("uci get -q system.system.speedtest"))

	if moduleType == "3g" or moduleType == "3g_ppp" then
		has_3g = true
	end

	uci:foreach("network", "interface",
	function (section)
		local ifname = uci:get(
			"network", section[".name"], "ifname"
		)
		local metric = uci:get(
			"network", section[".name"], "metric"
		)
		local info1
		local string1
		if "usb0" == ifname then
			string1 = "network.interface." .. tostring(section[".name"])
			info1 = _ubus:call(string1, "status", { })

			if info1 and info1['ipv4-address'] then
				local a
				for _, a in ipairs(info1['ipv4-address']) do
					if a.address  then
						has_wimax = true
					end
				end
			end
		end
	end
)
	if moduleVidPid == "12D1:1573" or moduleVidPid == "12D1:15C1" or moduleVidPid == "12D1:15C3" then
		modelservice = "LTE"
 	end
	local function rut_model()
		x = uci.cursor()
		local rutName
		x:foreach("system", "system", function(s)
			rutName = s.type
		end)
		return rutName or "unknown"
	end
	entry({"admin", "status"}, alias("admin", "status", "overview"), _("Status"), 20).index = true
	entry({"admin", "status", "overview"}, template("admin_status/index"), _("Overview"), 1)
	entry({"admin", "status", "sysinfo"}, template("admin_status/system"), _("System"), 2)

	if show then
		entry({"admin", "status", "netinfo"}, alias("admin", "status", "netinfo", "mobile" ), _("Network"), 3)
	else
		entry({"admin", "status", "netinfo"}, alias("admin", "status", "netinfo", "wan" ), _("Network"), 3)
	end
	if has_wimax then
		entry({"admin", "status", "netinfo", "wimax"}, template("admin_status/netinfo_wimax"), _("WiMAX"), 2).leaf = true
	else
		entry({"admin", "status", "netinfo", "wimax"}, call("go_to_overview"), nil, nil)
	end

	if show then
		entry({"admin", "status", "netinfo", "mobile" }, template("admin_status/netinfo_lte"), _("Mobile"), 1).leaf = true
	end

	local usb_module_name = uci:get("network", "ppp_usb", "ifname")
	if usb_module_name == "wwan-usb0" then
		entry({"admin", "status", "netinfo", "usb_modem" }, template("admin_status/netinfo_usb_modem"), _("USB Modem"), 2).leaf = true
	end
	entry({"admin", "status", "netinfo","wan"}, template("admin_status/netinfo_wan"), _("WAN"), 3).leaf = true
	entry({"admin", "status", "netinfo","lan"}, template("admin_status/netinfo_lan"), _("LAN"), 4).leaf = true
	entry({"admin", "status", "netinfo","wireless"}, template("admin_status/netinfo_wireless"), _("Wireless"), 5).leaf = true
	entry({"admin", "status", "netinfo","openvpn"}, template("admin_status/netinfo_openvpn"), _("OpenVPN"), 6).leaf = true
	entry({"admin", "status", "netinfo","vrrp"}, template("admin_status/netinfo_vrrp"), _("VRRP"), 7).leaf = true
	entry({"admin", "status", "netinfo","topology"}, template("admin_status/netinfo_topology"), _("Topology"), 8).leaf = true
	entry({"admin", "status", "netinfo","access"}, template("admin_status/netinfo_access"), _("Access"), 9).leaf = true

	entry({"admin", "status", "device"}, template("admin_status/devinfo"), _("Device"), 4)
	entry({"admin", "status", "service"}, template("admin_status/services"), _("Services"), 5)

	entry({"admin", "status", "routes"}, template("admin_status/routes"), _("Routes"), 6)
	entry({"admin", "status", "syslog"}, call("action_syslog"), nil, nil)
	entry({"admin", "status", "dmesg"}, call("action_dmesg"), nil, nil)

	entry({"admin", "status", "operators"}, call("get_opers"), nil, nil)
	entry({"admin", "status", "connect_rsp"}, call("connect_rsp"), nil, nil)
	entry({"admin", "status", "connect_network"}, call("connect_network_switch"), nil, nil)
	entry({"admin", "status", "connect_auto"}, call("connect_network_auto"), nil, nil)
	entry({"admin", "status", "auto_select"}, call("auto_select_switch"), nil, nil)
	entry({"admin", "status", "start_dmn"}, call("send_command"), nil, nil)
	entry({"admin", "status", "reconnect_interval"}, call("reconnect_interval"), nil, nil)

	if show then
		entry({"admin", "status", "realtime"}, alias("admin", "status", "realtime", "mobile"), _("Graphs"), 7)
		entry({"admin", "status", "realtime", "mobile"}, template("admin_status/mobile"), _("Mobile Signal"), 1).leaf = true
		if has_wimax then
			entry({"admin", "status", "realtime", "wimax"}, template("admin_status/wimax"), _("WiMAX Signal"), 1).leaf = true
		end
	else
		entry({"admin", "status", "realtime"}, alias("admin", "status", "realtime", "load"), _("Graphs"), 7)
	end

	entry({"admin", "status", "realtime", "mobile_status"}, call("action_mobile")).leaf = true
	entry({"admin", "status", "realtime", "wimax_status"}, call("action_wimax")).leaf = true

	entry({"admin", "status", "realtime", "load"}, template("admin_status/load"), _("Load"), 2).leaf = true
	entry({"admin", "status", "realtime", "load_status"}, call("action_load")).leaf = true

	entry({"admin", "status", "realtime", "bandwidth"}, template("admin_status/bandwidth"), _("Traffic"), 3).leaf = true
	entry({"admin", "status", "realtime", "bandwidth_status"}, call("action_bandwidth")).leaf = true

	entry({"admin", "status", "realtime", "wireless"}, template("admin_status/wireless"), _("Wireless"), 4).leaf = true
	entry({"admin", "status", "realtime", "wireless_status"}, call("action_wireless")).leaf = true

	entry({"admin", "status", "realtime", "connections"}, template("admin_status/connections"), _("Connections"), 5).leaf = true
	entry({"admin", "status", "realtime", "connections_status"}, call("action_connections")).leaf = true


	entry({"admin", "status", "nameinfo"}, call("action_nameinfo")).leaf = true
	if speedtest == "1" then
		entry({"admin", "status", "speedtest"}, template("admin_status/speedtest"), _("Speed Test"), 9)
	end

	entry({"admin", "status", "event"}, call("go_to"), _("Events Log"), 11)
	entry({"admin", "status", "event", "allevent"}, template("admin_status/allevent"), _("All Events"),1).leaf = true
	entry({"admin", "status", "event", "log"}, template("admin_status/eventlog"), _("System Events"),2).leaf = true
	entry({"admin", "status", "event", "connect"}, template("admin_status/connect"), _("Network Events"),3).leaf = true
	entry({"admin", "status", "event", "report"}, arcombine(cbi("events_reporting/events"), cbi("events_reporting/event-details")), _("Events Reporting"),4).leaf = true
	entry({"admin", "status", "event", "log_report"}, arcombine(cbi("eventslog_report/events"), cbi("eventslog_report/event-details")), _("Reporting Configuration"),5).leaf = true

end

function isempty(s)
	return s == nil or s == ''
end

function send_at(command)
	local response = assert(io.popen("/usr/sbin/gsmctl -A " .. command, 'r'))
	local value = ""

	if response then
		value = response:read("*a")
		response:close()
	end

	return value
end

function get_net_state()
	return luci.util.trim(luci.sys.exec("gsmctl -g"))
end

function get_registered_type(operator)
	local registered_operator
	local response = send_at("gsmctl -A AT+COPS?")

	if response == nil or response == "0" then
		return "automatic"
	end

	registered_operator = string.match(response, "\"%d+\"")
	registered_operator = string.gsub(registered_operator, '"', "")

	if registered_operator == operator then
		return "manual"
	else
		return "automatic"
	end

	return ""
end

function process_running(process)
	local pid = assert(io.popen("pgrep " .. process, "r"))
	local val = pid:read("*l")
	pid:close()

	return not isempty(val)
end

function wait_while_net_state(net_state, max_seconds)
	local elapsed = 0
	local increment = 1

	while (get_net_state() == net_state and elapsed < max_seconds) do
		sleep(increment)
		elapsed = elapsed + increment
	end
end

function wait_for_process(process, max_seconds)
	local elapsed = 0
	local increment = 5

	while not process_running("gsmd") do
		sleep(increment)
		elapsed = elapsed + increment

		if elapsed >= max_seconds then
			return false
		end
	end

	return true
end

function get_sim_id(var)
	if var == "0" then
		return "sim1"
	else
		return "sim2"
	end
end

function go_to_overview()
	luci.http.redirect(
	luci.dispatcher.build_url("admin", "status", "overview"))
end

function go_to()
 	luci.http.redirect(
	luci.dispatcher.build_url("admin", "status", "event", "allevent").."/")
end

function connect_network_auto(sim_id)
	luci.http.prepare_content("application/json")

	uci:delete("simcard", sim_id, "numeric")
	uci:delete("network", "ppp", "numeric")
	uci:delete("network", "ppp", "mode")
	uci:delete("simcard", sim_id, "mode")
	uci:set("network", "ppp", "service", "auto")

	uci:commit("network")
	uci:commit("simcard")

	if not wait_for_process("gsmd", 10) then
		luci.http.write_json("gsmd not running")
		return nil
	end

	luci.sys.exec("/etc/init.d/gsmd restart")

	wait_while_net_state("denied", 5)
	wait_while_net_state("searching", 20)

	luci.http.write_json(get_net_state())
end

function connect_network_manual(sim_id, number)
	luci.http.prepare_content("application/json")

	uci:delete("simcard", sim_id, "mode")
	uci:set("simcard", sim_id, "numeric", number)
	uci:commit("simcard")

	if not wait_for_process("gsmd", 10) then
		luci.http.write_json("gsmd not running")
		return nil
	end

	luci.sys.exec("/etc/init.d/gsmd restart")

	wait_while_net_state("searching", 20)

	luci.http.write_json(get_net_state())
end

function connect_network_manual_auto(sim_id, number)
	luci.http.prepare_content("application/json")

	uci:set("simcard", sim_id, "mode", "Manual-Auto")
	uci:set("simcard", sim_id, "numeric", number)
	uci:commit("simcard")

	if not wait_for_process("gsmd", 10) then
		luci.http.write_json("gsmd not running")
		return nil
	end

	luci.sys.exec("/etc/init.d/gsmd restart")

	wait_while_net_state("Timeout.", 25)
	wait_while_net_state("searching", 20)

	local response = get_net_state()
	if string.match(response, "^(registered \([^)]+\))") then
		response = string.format("%s %s", response, get_registered_type(number))
	end

	luci.http.write_json(response)
end

function sleep(n)
	require "socket"
	socket.select(nil, nil, n)
end

function send_command()
	luci.util.exec("ifdown ppp")
	luci.util.exec("/etc/init.d/ledsman stop")
	os.execute("sleep 2")
	os.execute("/sbin/switch_checker.sh &")
end

function reconnect_interval()
	local current = "sim1"
	local interval = tonumber(luci.http.formvalue("interval"))
	local tab = tonumber(luci.http.formvalue("tab"))

	if interval == nil or tab == nil then
		return
	end

	if tab == 1 then
		current = "sim2"
	end

	uci:set("simcard", current, "reconnect", interval)
	uci:commit("simcard")
end

function get_opers()
	--Parsina duomenis is failo
	if not fileExists("/tmp/", "operators") then
		return
	end

	local operators = {}
	local bwc = assert(io.open('/tmp/operators' , 'r'))
	local timeout = 10
	local moduleVidPid =
		luci.util.trim(luci.sys.exec("uci get system.module.vid"))..":"..
		luci.util.trim(luci.sys.exec("uci get system.module.pid"))
	luci.http.prepare_content("application/json")

	while true do
		local l = bwc:read("*l")
		if not l then
			luci.http.write_json("wait")
			bwc:close()
			return
		end

		luci.util.exec("ifup ppp")
		luci.util.exec("/etc/init.d/ledsman start")

		os.execute("cp /tmp/operators /tmp/opers")

		if l:match("OK") == "OK" then
			break
		end

		if l:match("Timeout") == "Timeout" then
			operators = "timeout"
			luci.http.write_json(operators)
			break
		end

		if l:match("error") == "error" then
			operators = "error"
			luci.http.write_json(operators)
			break
		end

		if l:match("+COPS:") then
			l = string.gsub(l, "+COPS:", "")
			local i = 1

			for word in string.gmatch(l, '%b()') do
				local n = 1
				operators[i] = {}

				for val in string.gmatch(word, '([^(),]+)') do
					val = string.gsub(val, '"', "")
					if val == "" then
						val = "-"
					end

					if moduleVidPid == "2C7C:0125" then
						local range_start
						local range_end
						local num_list
						range_start, range_end = string.match(val, '^(%d)%-(%d)$')

						if not isempty(range_start) and not isempty(range_end) then
							local j
							for j = range_start, range_end do
								operators[i][n] = tostring(j)
								n = n + 1
							end
						else
							operators[i][n] = tostring(val)
							n = n + 1
						end
					else
						operators[i][n] = tostring(val)
						n = n + 1
					end
				end
				i = i + 1
			end

			luci.http.write_json(operators)
			bwc:close()
			break
		end
	end

	os.remove("/tmp/operators")
end

function connect_rsp()
	luci.http.prepare_content("application/json")
	local operctl = luci.sys.exec("ps | grep operctl | head -1")

	if not string.find(operctl, "grep operctl") then
		luci.http.write_json("running")
		return
	end

	local mode = luci.http.formvalue("oper_mode")
	local response = get_net_state()

	if mode ~= "Manual-Auto" then
		luci.http.write_json(response)
		return
	end

	local number = luci.http.formvalue("numeric")
	if tonumber(number) == nil then
		luci.http.write_json("NAN")
		return nil
	end

	if string.match(response, "^(registered \([^)]+\))") then
		response = string.format("%s %s", response, get_registered_type(number))
	end

	luci.http.write_json(response)
end

function action_syslog()
	local syslog = luci.sys.syslog()
	luci.template.render("admin_status/syslog", {syslog=syslog})
end

function action_dmesg()
	local dmesg = luci.sys.dmesg()
	luci.template.render("admin_status/dmesg", {dmesg=dmesg})
end

function action_bandwidth()
	local path  = luci.dispatcher.context.requestpath
	local iface = path[#path]

	luci.http.prepare_content("application/json")

	local bwc = io.popen("luci-bwc -i %s 2>/dev/null" % escape_shell(iface))
	if bwc then
		luci.http.write("[")

		while true do
			local ln = bwc:read("*l")
			if not ln then break end
			luci.http.write(ln)
		end

		luci.http.write("]")
		bwc:close()
	end
end

function action_wireless()
	local path  = luci.dispatcher.context.requestpath
	local iface = path[#path]
	local ln

	luci.http.prepare_content("application/json")

	if iface:match("^[a-zA-Z0-9%-_]+$") ~= nil then
		str=luci.sys.exec("iwinfo %s info" % escape_shell(iface))
		for line in str:gmatch("[^\r\n]+") do
			if line:find("Signal: ") then
				s = string.gsub(line,"Signal: ","")
				if s:find(" dBm  Noise:") then
					signal = string.gsub(string.gsub(s," dBm  Noise:.*","")," ","")
				else
					signal = -100
				end
				n = string.gsub(line,"Signal:.*: ","")
				if n:find(" dBm") then
					noise = string.gsub(string.gsub(n," dBm", "")," ","")
				else
					noise = -100
				end
			end

			if line:find("Bit Rate: ") then
				r = string.gsub(string.gsub(line, "Bit Rate: ",""),".0","")
				if r:find(" MBit/s") then
					rate = string.gsub(string.gsub(r," MBit/s","")," ","")
				else
					rate = 0
				end
			end
		end
		ln = "[ " .. luci.sys.uptime() .. ", " .. rate .. ", " .. signal .. ", " .. noise .. " ]"
	else
		ln = "[ " .. luci.sys.uptime() .. ", 0, 0, 0 ]"
	end

	luci.http.write("[")
	luci.http.write(ln)
	luci.http.write("]")
end

function action_load()
	luci.http.prepare_content("application/json")

	local bwc = io.popen("luci-bwc -l 2>/dev/null")
	if bwc then
		luci.http.write("[")

		while true do
			local ln = bwc:read("*l")
			if not ln then break end
			luci.http.write(ln)
		end

		luci.http.write("]")
		bwc:close()
	end
end

function action_mobile()
	local s = io.popen("gsmctl -t 2>/dev/null")
	local signal = s:read("*l")
	s:close()
	luci.sys.call("usleep 10000")
	local str = io.popen("gsmctl -q 2>/dev/null")
	local strength = str:read("*l") or "-120"
	str:close()

	--2G:
	local GSM = -120
	local GPRS = -120
	local EDGE = -120

	--3G:
	local WCDMA = -120
	local HSDPA = -120
	local HSUPA = -120
	local HSPA = -120
	local HSPAplus = -120
	local DCHSPAplus = -120
	local HSDPAplusHSUPA = -120
	local UMTS = -120

	--4G:
	local LTE = -120

	local con_type = 12

	if signal == "GSM" then
		GSM = strength
		con_type = 0
	elseif signal == "GPRS" then
		GPRS = strength
		con_type = 1
	elseif signal == "EDGE" then
		EDGE = strength
		con_type = 2
	elseif signal == "WCDMA" then
		WCDMA = strength
		con_type = 3
	elseif signal == "HSDPA" then
		HSDPA = strength
		con_type = 4
	elseif signal == "HSUPA" or signal == "HSDPA/HSUPA" then
		HSUPA = strength
		con_type = 5
	elseif signal == "HSPA" then
		HSPA = strength
		con_type = 6
	elseif signal == "HSPA+" then
		HSPAplus = strength
		con_type = 7
	elseif signal == "DC-HSPA+" then
		DCHSPAplus = strength
		con_type = 8
	elseif signal == "HSDPA+HSUPA" then
		HSDPAplusHSUPA = strength
		con_type = 9
	elseif signal == "UMTS" then
		UMTS = strength
		con_type = 10
	elseif signal == "LTE" then
		LTE = strength
		con_type = 11
	end

	luci.http.prepare_content("application/json")
	luci.http.write("[")
	local ln = "[ " .. luci.sys.uptime() .. ", " .. GSM .. ", " .. GPRS .. ", " .. EDGE .. ", " .. WCDMA .. ", " .. HSDPA .. ", " .. HSUPA .. ", " .. HSPA .. ", " .. HSPAplus .. ", " .. DCHSPAplus .. ", " .. HSDPAplusHSUPA .. ", " .. UMTS ..", " .. LTE .. ", " .. con_type .. "]"
	luci.http.write(ln)
	luci.http.write("]")
end
function action_wimax()
	local nw = require "luci.model.network"

	luci.http.prepare_content("application/json")

	local strength = nw:wimaxCGICall({ call ="signal-strength" })

	luci.http.write("[")
	local ln = "[ " .. luci.sys.uptime() .. ", " .. strength .."]"
	luci.http.write(ln)
	luci.http.write("]")
end
function action_connections()
	luci.http.prepare_content("application/json")
	luci.http.write("{ connections: ")
	luci.http.write_json(luci.sys.net.conntrack())

	local bwc = io.popen("luci-bwc -c 2>/dev/null")
	if bwc then
		luci.http.write(", statistics: [")

		while true do
			local ln = bwc:read("*l")
			if not ln then break end
			luci.http.write(ln)
		end

		luci.http.write("]")
		bwc:close()
	end

	luci.http.write(" }")
end

function action_temperature()
	luci.http.prepare_content("application/json")
	local tmp = io.popen("cat /tmp/temperature")
	local temperature = tmp:read("*l")
	local dot = string.find(temperature, "%.")
	if string.len(temperature) - dot == 6 then
		luci.http.write("[")
		local ln = "[ " .. luci.sys.uptime() .. ", " .. temperature .. "]"
		luci.http.write(ln)
		luci.http.write("]")
	end
	tmp:close()
end

function action_nameinfo(...)
	local i
	local rv = { }
	for i = 1, select('#', ...) do
		local addr = select(i, ...)
		local fqdn = nixio.getnameinfo(addr)
		rv[addr] = fqdn or (addr:match(":") and "[%s]" % addr or addr)
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
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

function auto_select_switch()
	luci.http.prepare_content("application/json")
	local timeout = 30
	local tab = luci.http.formvalue("numeric") -- Should be numerictab for consistency reasons with connect_network_switch
	local sim_id = get_sim_id(tab)
	local activeSim = luci.http.formvalue("active")
	if sim_id == "sim1" and activeSim == "SIM 1" or sim_id == "sim2" and activeSim == "SIM 2" then
		connect_network_auto(sim_id)
	else
		auto_select(sim_id)
	end
end

function connect_network_switch() -- depends on selected tab and current active sim
	luci.http.prepare_content("application/json")
	local timeout = 30
	local number = luci.http.formvalue("numeric")

	if tonumber(number) == nil then
		luci.http.write_json("NAN")
		return nil
	end

	local activeSim = luci.http.formvalue("active")
	local tab = luci.http.formvalue("numerictab")
	local mode = luci.http.formvalue("oper_mode")
	local sim_id = get_sim_id(tab)

	if sim_id == "sim1" and activeSim == "SIM 1" or sim_id == "sim2" and activeSim == "SIM 2" then
		if mode == "Manual-Auto" then
			connect_network_manual_auto(sim_id, number)
		else
			connect_network_manual(sim_id, number)
		end
	else
		select_network(sim_id, number, mode)
	end
end

function select_network(sim_id, number, mode)
	luci.http.prepare_content("application/json")

	uci:set("simcard", sim_id, "numeric", number)

	if mode == "Manual-Auto" then
		uci:set("simcard", sim_id, "mode", mode)
	else
		uci:delete("simcard", sim_id, "mode")
	end

	uci:commit("simcard")

	if uci:get("simcard", sim_id, "numeric") ~= "" then
		luci.http.write_json("Selected")
	else
		luci.http.write_json("Not selected")
	end
end

function auto_select(sim_id)
	luci.http.prepare_content("application/json")

	uci:delete("simcard", sim_id, "numeric")
	uci:delete("simcard", sim_id, "mode")

	uci:commit("simcard")

	if uci:get("simcard", sim_id, "numeric") == "" then
		luci.http.write_json("Selected")
	else
		luci.http.write_json("Not selected")
	end
end
