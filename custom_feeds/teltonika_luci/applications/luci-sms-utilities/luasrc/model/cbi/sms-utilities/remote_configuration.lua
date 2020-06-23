local uci = require "luci.model.uci".cursor()
local ut = require "luci.util"

m = Map("sms_utils", translate("Send Configuration"),translate(""))
m.pageaction = false

local section_cfgsms = m.uci:get("sms_utils", "cfgsms")
local section_message = m.uci:get("sms_utils", "message")

if not section_cfgsms then
	m.uci:set("sms_utils", "cfgsms", "cfgsms")
	m.uci:save("sms_utils")
	m.uci:commit("sms_utils")
end

if not section_message then
	m.uci:set("sms_utils", "message", "message")
	m.uci:save("sms_utils")
	m.uci:commit("sms_utils")
end

s2 = m:section(NamedSection, "cfgsms", "cfgsms", translate("Setup Configuration Message"))
s2:tab("primarytab", translate("Network"))
s2:tab("secondarytab", translate("VPN"))
local config = {}
local vpn_config = {}
local vpn_certificates = {}

e = s2:taboption("primarytab", ListValue, "_generate", translate("Generate SMS"), translate("Generate new SMS or use current configuration"))
	e:value("new", translate("New"))
	e:value("current", translate("From current configuration"))
	e.default = "new"

	function e.write() end

------------------------------------
---------Current--------------------
------------------------------------

e = s2:taboption("primarytab", Flag, "_mobile", translate("Mobile"), translate("Include configuration for mobile network"))
	e:depends("_generate", "current")

	function e.write(self, section, value)
		local sim1 = m.uci:get("simcard", "sim1")
		local sim2 = m.uci:get("simcard", "sim2")
		table.insert(config, "network.ppp")
		if sim1 then
			table.insert(config, "simcard.sim1")
		end
		if sim2 then
			table.insert(config, "simcard.sim2")
		end
		table.insert(config, "simcard.simcard.default")
	end

e = s2:taboption("primarytab", Flag, "_wan", translate("WAN"), translate("Include configuration for WAN (Wide Area Network)"))
e:depends("_generate", "current")

	function e.write(self, section, value)
		local wan1 = m.uci:get("network", "wan")
		local wan2 = m.uci:get("network", "wan2")
		local wan3 = m.uci:get("network", "wan3")
		if wan1 then
			table.insert(config, "network.wan")
		end
		if wan2 then
			table.insert(config, "network.wan2")
		end
		if wan3 then
			table.insert(config, "network.wan3")
		end
	end

e = s2:taboption("primarytab", Flag, "_lan", translate("LAN"), translate("Include configuration for LAN (Local Area Network)"))
	e:depends("_generate", "current")

	function e.write(self, section, value)
		table.insert(config, "network.lan")
	end

------------------------------------------------
------New---------------------------------------
------------------------------------------------

e = s2:taboption("primarytab", Flag, "_new_wan", translate("WAN"), translate("Include configuration for WAN (Wide Area Network)"))
	e:depends("_generate", "new")

	function e.write()
		table.insert(config, "network.wan.enabled=1")
		table.insert(config, "network.wan.disabled=0")
	end

e = s2:taboption("primarytab", ListValue, "_wan_iface", translate("Interface"), translate("Interface type used for WAN (Wide Area Network) connection"))
e:value("wired", translate("Wired"))
e:value("3g", translate("Mobile"))
e:depends("_new_wan", "1")
e.default = "3g"

	function e.write(self, section, value)
		if value == "3g" then
			table.insert(config, "network.ppp.enabled=1")
			table.insert(config, "network.ppp.disabled=0")
		elseif value == "wired" then
			table.insert(config, "network.wan.ifname=eth1")
			table.insert(config, "network.ppp.enabled=0")
			table.insert(config, "network.ppp.disabled=1")
		end
	end

e = s2:taboption("primarytab", ListValue, "_wan_proto", translate("Protocol"), translate("Network protocol used for network configuration parameters management"))
	e:value("dhcp", translate("DHCP"))
	e:value("static", translate("Static"))
	e:depends("_wan_iface", "wired")

	function e.write(self, section, value)
		if value == "dhcp" then
			table.insert(config, "network.wan.proto=dhcp")
		elseif value == "static" then
			table.insert(config, "network.wan.proto=static")
		end
	end

e = s2:taboption("primarytab", Value, "_wired_ip", translate("IP address"), translate("IP address that router will use to connect to the internet"))
	e:depends({_wan_iface = "wired", _wan_proto = "static"})
	e.datatype = "ip4addr"

	function e.parse(self, section, novld)
		local wan = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._new_wan")
		local static = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._wan_proto")
		local interface = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._wan_iface")
		if wan and  static and static == "static" and interface and interface == "wired" then
			custom_parse(self, section, novld)
		end
	end

	function e.write(self, section, value)
		if value then
			table.insert(config, string.format("network.wan.ipaddr=" ..value))
		else
			table.insert(config, string.format("network.wan.netmask="))
		end
	end

e = s2:taboption("primarytab", Value, "_wired_mask", translate("IP netmask"), translate("A subnet mask that will be used to define how large the WAN (Wide Area Network) network is"))
	e:depends({_wan_iface = "wired", _wan_proto = "static"})
	e.datatype = "ip4addr"
	e:value("255.255.255.0")
	e:value("255.255.0.0")
	e:value("255.0.0.0")
	e.default = "255.255.255.0"

	function e.parse(self, section, novld)
		local wan = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._new_wan")
		local static = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._wan_proto")
		local interface = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._wan_iface")
		if wan and  static and static == "static" and interface and interface == "wired" then
			custom_parse(self, section, novld)
		end
	end

	function e.write(self, section, value)
		if value then
			table.insert(config, string.format("network.wan.netmask=" ..value))
		else
			table.insert(config, string.format("network.wan.netmask="))
		end
	end

e = s2:taboption("primarytab", Value, "_wired_gateway", translate("IP gateway"), translate("Default gateway, the address where traffic destined for the internet is routed to"))
	e:depends({_wan_iface = "wired", _wan_proto = "static"})
	e.datatype = "ip4addr"

	function e.parse(self, section, novld)
		local wan = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._new_wan")
		local static = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._wan_proto")
		local interface = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._wan_iface")
		if wan and  static and static == "static" and interface and interface == "wired" then
			custom_parse(self, section, novld)
		end
	end

	function e.write(self, section, value)
		if value then
			table.insert(config, string.format("network.wan.gateway=" ..value))
		else
			table.insert(config, string.format("network.wan.gateway="))
		end
	end

e = s2:taboption("primarytab", Value, "_wired_broadcast", translate("IP broadcast"), translate("A logical address at which all devices connected to a multiple-access communications network are enabled to receive datagrams"))
	e:depends({_wan_iface = "wired", _wan_proto = "static"})
	e.datatype = "ip4addr"

	function e.parse(self, section, novld)
		local wan = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._new_wan")
		local static = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._wan_proto")
		local interface = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._wan_iface")
		if wan and  static and static == "static" and interface and interface == "wired" then
			custom_parse(self, section, novld)
		end
	end

	function e.write(self, section, value)
		if value then
			table.insert(config, string.format("network.wan.broadcast=" ..value))
		else
			table.insert(config, string.format("network.wan.broadcast="))
		end
	end

e = s2:taboption("primarytab", ListValue, "default_sim", translate("Primary SIM card"), translate("A SIM card that will be used as primary"))
	e:value("sim1", translate("SIM1"))
	e:value("sim2", translate("SIM2"))
	e.default = "sim1"
	-- e:depends("_new_mobile", "1")
	e:depends("_wan_iface", "3g")

	function e.write(self, section, value)
		if value then
			table.insert(config, "simcard.simcard.default=" .. value)
		end
	end

e = s2:taboption("primarytab", ListValue, "_mobile_connection", translate("Mobile connection"), translate("An underlying agent that will be used for mobile data connection creation and management"))
	e:value("3g-ppp", translate("Use PPP mode"))
	e:value("eth2", translate("Use NDIS mode"))
	e:value("ncm", translate("Use NCM mode"))
	e:value("qmi", translate("Use QMI mode"))
	e:value("qmi2", translate("Use QMI mode used for Quectel EC25-E modem"))
	-- e:depends("_new_mobile", "1")
	e:depends("_wan_iface", "3g")

	function e.write(self, section, value)
		if value then
			local sim_card = m:formvalue("cbid.sms_utils.cfgsms.default_sim") or "sim1"
			if value == "eth2" then
				table.insert(config, "network.wan.ifname=eth2")
				table.insert(config, "network.wan.proto=dhcp")
				table.insert(config, "simcard." .. sim_card .. ".proto=ndis")
				table.insert(config, "simcard." .. sim_card .. ".ifname=eth2")
			elseif value == "ncm" then
				table.insert(config, "network.wan.ifname=wwan0")
				table.insert(config, "network.wan.proto=ncm")
				table.insert(config, "simcard." .. sim_card .. ".ifname=wwan0")
				table.insert(config, "simcard." .. sim_card .. ".proto=ncm")
			elseif value == "qmi2" then
				table.insert(config, "network.wan.ifname=wwan0")
				table.insert(config, "network.wan.proto=qmi2")
				table.insert(config, "simcard." .. sim_card .. ".ifname=wwan0")
				table.insert(config, "simcard." .. sim_card .. ".proto=qmi2")
			elseif value == "qmi" then
				table.insert(config, "network.wan.ifname=wwan0")
				table.insert(config, "network.wan.proto=qmi")
				table.insert(config, "simcard." .. sim_card .. ".ifname=wwan0")
				table.insert(config, "simcard." .. sim_card .. ".proto=qmi")
			else
				table.insert(config, "network.wan.ifname=3g-ppp")
				table.insert(config, "network.wan.proto=none")
				table.insert(config, "simcard." .. sim_card .. ".proto=3g")
				table.insert(config, "simcard." .. sim_card .. ".ifname=3g-ppp")
			end
		end
	end

e = s2:taboption("primarytab", Value, "_mobile_apn", translate("APN"), translate("A network identifier that will be used by a router when connecting to a GSM carrier"))
	-- e:depends("_new_mobile", "1")
	e:depends("_wan_iface", "3g")

	function e.parse(self, section, novld)
		local wan = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._new_wan")
		local interface = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._wan_iface")
		if wan and interface and interface == "3g" then
			custom_parse(self, section, novld)
		end
	end

	function e.write(self, section, value)
			local sim_card = m:formvalue("cbid.sms_utils.cfgsms.default_sim") or "sim1"
		if value then
			table.insert(config, string.format("simcard." .. sim_card .. ".apn=" ..value))
		else
			table.insert(config, string.format("simcard." .. sim_card .. ".apn="))
		end
	end

e = s2:taboption("primarytab", Value, "_mobile_dialing", translate("Dialing number"), translate("A phone number that will be used to establish a mobile PPP (Point-to-Point Protocol) connection"))
	-- e:depends("_new_mobile", "1")
	e:depends("_wan_iface", "3g")

	function e.parse(self, section, novld)
		local wan = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._new_wan")
		local interface = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._wan_iface")
		if wan and interface and interface == "3g" then
			custom_parse(self, section, novld)
		end
	end

	function e.write(self, section, value)
		local sim_card = m:formvalue("cbid.sms_utils.cfgsms.default_sim") or "sim1"

		if value then
			table.insert(config, string.format("simcard." .. sim_card .. ".dialnumber=" ..value))
		else
			table.insert(config, string.format("simcard." .. sim_card .. ".dialnumber="))
		end
	end

e = s2:taboption("primarytab", ListValue, "_mobile_auth", translate("Authentication method"), translate("An authentication method that will be used to authenticate new connections on your GSM carrier\\'s network"))
	e:value("chap", translate("CHAP"))
	e:value("pap", translate("PAP"))
	e:value("none", translate("None"))
	-- e:depends("_new_mobile", "1")
	e:depends("_wan_iface", "3g")
	e.default = "none"

	function e.write(self, section, value)
		local sim_card = m:formvalue("cbid.sms_utils.cfgsms.default_sim") or "sim1"

		if value then
			table.insert(config, string.format("simcard." .. sim_card .. ".auth_mode=" ..value))
		end
	end

o = s2:taboption("primarytab", Value, "username", translate("User name"), translate("User name used for authentication on your GSM carrier\\'s network"))
	o:depends("_mobile_auth", "chap")
	o:depends("_mobile_auth", "pap")

	function o.parse(self, section, novld)
		local wan = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._new_wan")
		local interface = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._wan_iface")
		local auth = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._mobile_auth")

		if wan and interface and interface == "3g" and auth and (auth == "pap" or auth == "chap") then
			custom_parse(self, section, novld)
		end
	end

	function o.write(self, section, value)
		local sim_card = m:formvalue("cbid.sms_utils.cfgsms.default_sim") or "sim1"

		if value then
			table.insert(config, string.format("simcard." .. sim_card .. ".username=" ..value))
		else
			table.insert(config, string.format("simcard." .. sim_card .. ".username="))
		end
	end

o = s2:taboption("primarytab", Value, "password", translate("Password"), translate("Password used for authentication on your GSM carrier\\'s network"))
	o:depends("_mobile_auth", "chap")
	o:depends("_mobile_auth", "pap")
	o.password = true;

	function o.parse(self, section, novld)
		local wan = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._new_wan")
		local interface = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._wan_iface")
		local auth = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._mobile_auth")
		if wan and interface and interface == "3g" and auth and (auth == "pap" or auth == "chap") then
			custom_parse(self, section, novld)
		end
	end

	function o.write(self, section, value)
		local sim_card = m:formvalue("cbid.sms_utils.cfgsms.default_sim") or "sim1"

		if value then
			table.insert(config, string.format("simcard." .. sim_card .. ".password=" ..value))
		else
			table.insert(config, string.format("simcard." .. sim_card .. ".password="))
		end
	end

e = s2:taboption("primarytab", ListValue, "_mobile_service", translate("Service mode"), translate("Your network\\'s preference. If your local mobile network supports GSM (2G), UMTS (3G) or LTE (4G) you can specify to which network you prefer to connect to"))
	e:value("gprs-only", translate("2G only"))
	e:value("gprs", translate("2G preferred"))
	e:value("umts-only", translate("3G only"))
	e:value("umts", translate("3G preferred"))
	e:value("lte-only", translate("4G (LTE) only"))
	e:value("lte", translate("4G (LTE) preferred"))
	e:value("auto", translate("Automatic"))
	e.default = "umts"
	-- e:depends("_new_mobile", "1")
	e:depends("_wan_iface", "3g")

	function e.write(self, section, value)
		local sim_card = m:formvalue("cbid.sms_utils.cfgsms.default_sim") or "sim1"

		if value then
			table.insert(config, string.format("simcard." .. sim_card .. ".service=" ..value))
		end
	end


e = s2:taboption("primarytab", Flag, "_new_lan", translate("LAN"), translate("Include configuration for LAN (Local Area Network)"))
	e:depends("_generate", "new")
	function e.write() end

e = s2:taboption("primarytab", Value, "_lan_ip", translate("IP address"), translate("IP address that router will use on LAN (Local Area Network) network"))
	e:depends("_new_lan", "1")
	e.datatype = "ip4addr"

	function e.parse(self, section, novld)
		local lan = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._new_lan")
		if lan then
			custom_parse(self, section, novld)
		end
	end

	function e.write(self, section, value)
		if value then
			table.insert(config, string.format("network.lan.ipaddr=" ..value))
		else
			table.insert(config, string.format("network.lan.ipaddr="))
		end
	end

e = s2:taboption("primarytab", Value, "_lan_mask", translate("IP netmask"), translate("A subnet mask that will be used to define how large the LAN (Local Area Network) network is"))
	e:depends("_new_lan", "1")
	e.datatype = "ip4addr"
	e:value("255.255.255.0")
	e:value("255.255.0.0")
	e:value("255.0.0.0")
	e.default = "255.255.255.0"

	function e.parse(self, section, novld)
		local lan = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._new_lan")
		if lan then
			custom_parse(self, section, novld)
		end

	end

	function e.write(self, section, value)
			if value then
				table.insert(config, string.format("network.lan.netmask=" ..value))
			else
				table.insert(config, string.format("network.lan.netmask="))
			end
		end

e = s2:taboption("primarytab", Value, "_lan_broadcast", translate("IP broadcast"), translate("A logical address at which all devices connected to a multiple-access communications network are enabled to receive datagrams"))
	e:depends("_new_lan", "1")
	e.datatype = "ip4addr"

	function e.parse(self, section, novld)
		local lan = luci.http.formvalue("cbid." .. self.config .. "." .. section .. "._new_lan")
		if lan then
			custom_parse(self, section, novld)
		end
	end

	function e.write(self, section, value)
		if value then
			table.insert(config, string.format("network.lan.broadcast=" ..value))
		else
			table.insert(config, string.format("network.lan.broadcast="))
		end
	end

-- btn = s2:taboption("primarytab", Button, "_btn")
-- btn.template  = "sms-utilities/button"
-- btn.value = "Generate"

----------------------------------------------
	--vpn section--
----------------------------------------------

-- Selected protocol inclusion
local form, ferr = loadfile(
	ut.libpath() .. "/model/cbi/sms-utilities/vpn_server.lua"
)

if not form then
	s2:taboption("secondarytab", DummyValue, "_error",
		translate("Missing file")
	).value = ferr
else
	setfenv(form, getfenv(1))(m, s2, vpn_config, vpn_certificates, true)
end

-- btn = s2:taboption("secondarytab", Button, "_btn")
-- btn.template  = "sms-utilities/button"
-- btn.value = "Generate"

----------------------------------------------
	--SMS siuntimas
----------------------------------------------
s3 = m:section(NamedSection, "message", "message", translate("Send Message Settings"))
s3.template_addremove = "sms-utilities/send_status"

-- o = s3:option(TextValue, "_custom")
-- o.height = 150

function write_to_file(path, string)
	local file = io.open(path, "w")
	file:write(string)
	file:close()
end

function read_current_conf(value)
	local output
	local first = true
	local commands = ""
	for key,value in pairs(value) do
		output = assert (io.popen("uci show " ..value))

		for line in output:lines() do
			if string.match(line, value) then
				if line then
					if first then
						commands = line
						first = false
					else
						commands = commands ..", ".. line
					end
				end
			end
		end
		output:close()
	end

	return commands
end

function read_new_conf(command)
	local allCommands = ""
	local first = true
	for key,value in pairs(command) do
		if value then
			debug("DSDSDSDSDS value == " .. value)
			if first then
				allCommands = value
				first = false
			else
				allCommands = allCommands ..", "..value
			end
		end
	end
	debug("\nallCommands == " .. allCommands)
	return allCommands
end

function count_messages(commands)
	local len = string.len(commands)
	if len > 0 then
		local sms_count = tonumber(string.format("%.0f", string.len(commands) / 160))
		if string.len(commands) > 160 and string.len(commands) % 160 > 0 then
			sms_count = sms_count + 1
		end
		message = translate(sms_count.." SMS messages required")
	end
end

function custom_parse(self, section, novld)
	local fvalue = self:formvalue(section)
	self:write(section, fvalue)
end


function debug(string)
	os.execute("logger -s \"" ..string.. "\"")
end

num = s3:option(Value, "tel", translate("Phone number"), translate("A phone number of router which will receive the configuration"))
	num.datatype = "string_not_empty"

function num.validate(self, value)
	if value:match("^[0-9\*+#]+$") == nil then
		m.message = "err:Bad phone number format..."
		return nil
	else
		return value
	end
end

function num.write(self, section, value)
	local sendPressed = luci.http.formvalue("cbid."..self.config.."."..section.."._sendBtn")
	local serial_num = luci.http.formvalue("cbid."..self.config.."."..section..".serial")
	local password = luci.http.formvalue("cbid."..self.config.."."..section..".pass")
	local authorisation = serial_num or password or ""

	local current_conf = luci.http.formvalue("cbid.sms_utils.cfgsms._generate")
	local current_conf_vpn = luci.http.formvalue("cbid.sms_utils.cfgsms._generate_vpn")
	local commands, vpn_commands
	local text

	if sendPressed then
		if current_conf == "current" then
			commands = read_current_conf(config)
		elseif current_conf == "new" then
			commands = read_new_conf(config)
		end
		if current_conf_vpn == "current" then
			vpn_commands = read_current_conf(vpn_config)
		elseif current_conf_vpn == "new" then
			vpn_commands = read_new_conf(vpn_config)
		end

		if commands ~= "" and vpn_commands ~= "" then
			text = commands ..", " ..vpn_commands
		elseif commands ~= "" then
			text = commands
		elseif vpn_commands ~= "" then
			text = vpn_commands
		end

		if text and text ~= "" then
			local tmpFile = "/tmp/conf_msg"
			local tmpStatus = "/tmp/sms_status"
			write_to_file(tmpFile, text)
			--os.execute("/sbin/sms_utils/send_conf_messages request & 2>/dev/null >/dev/null")
			if authorisation and authorisation ~= "" then
				--os.execute("echo '" ..string.format('/usr/sbin/gsmctl -S -s "%s %s %s"', value, authorisation, command).."'>/tmp/lua.test")
				output = assert (io.popen(string.format('/usr/sbin/gsmctl -S -s "%s %s get_configure"', value, authorisation)))
			else
				--os.execute("echo '" ..string.format('/usr/sbin/gsmctl -S -s "%s %s %s"', value, authorisation, command).."'>/tmp/lua.test")
				output = assert (io.popen(string.format('/usr/sbin/gsmctl -S -s "%s get_configure"', value)))
			end
			local t = output:read()
-- 			os.execute("echo " ..t.." >/tmp/labas")
			output:close()
			m.message = "scs:Sending request message..."
			if t == "OK" then
				write_to_file(tmpStatus, "SC:1:W:Request message sent. Waiting for response...")
				m.uci:set("sms_utils", "message", "wait_response", "1")
				m.uci:set("sms_utils", "message", "number", value)
				m.uci:commit("sms_utils")
			elseif t then
				write_to_file(tmpStatus, "ER:0:W:Request message "..t)
			end
		else
			m.message = "err:No configuration selected..."
		end
	end
end

auth = s3:option(ListValue, "auth", translate("Authorization method"), translate("What kind of authorization to use for remote configuration"))
auth:value("noauth", "No authorization")
auth:value("serial", "By serial")
auth:value("password", "By router admin password")

	function auth.write() end

serial = s3:option(Value, "serial", translate("Router serial number"), translate("Serial number to use for authorization"))
serial:depends("auth", "serial")



	function serial:validate(value)
		local failure
		if value then
			if string.len(value) > 10 then
				m.message = translate("err: Serial number is too long!")
				return nil
			elseif string.len(value) < 8 then
				m.message = translate("err: Serial number is too short!")
			else
				return value
			end
		else
			m.message = translate("err: Serial number text field is empty!")
			return nil
		end
	end

	function serial.write() end

pass = s3:option(Value, "pass", translate("Router admin password"), translate("Admin password to use for authorization"))
pass:depends("auth", "password")

	function pass.write() end

send = s3:option(Button, "_sendBtn")
send.template  = "sms-utilities/button"
send.value = "Send"

return m
