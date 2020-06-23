--
-- Copyright (C), 2019 Teltonika
--

require "teltonika_lua_functions"
local uci = require "luci.model.uci".cursor()
local ds = require "luci.dispatcher"
local nw = require "luci.model.network"
local bus = require "ubus"
local _ubus = bus.connect()

--values from config
local vid = uci:get("system", "module", "vid")
local pid = uci:get("system", "module", "pid")
local product_name = uci:get("hwinfo", "hwinfo", "mnf_code")
local primary_sim = uci:get("simcard", "simcard", "default")
local multiwan_on = uci:get("multiwan", "config", "enabled") or "0"
local dual_sim = uci:get("hwinfo", "hwinfo", "dual_sim")

local moduleVidPid = nil
local modem_detected = true

_ubus:call("file", "exec", { command="/usr/sbin/operctl", params={"-f"} })

if vid and pid then
	moduleVidPid = "%s:%s" % { vid , pid }
else
	modem_detected = false
end
-- init default modem variables

local ifname1 = "3g-ppp"
local ifname2 = "wwan0"
local protocol1 = "3g"
local proto_name1 = "PPP"
local protocol2 = "qmi2"
local proto_name2 = "QMI"
local bridge = true
local passthrough = true
local pref_2G = false
local pref_3G = false
local pref_3G_4G = false
local pref_4G = false
local only_2G_3G = false
local only_2G_4G = false
local only_3G_4G = false
local use_4G_only = true
local need_modify = false	-- option for TelitLTE_V2
local device = "/dev/modem_data"
local change_band = false
local apn_id, apn_list, info

-- add module exceptions not all module settings
-- if statment could marge some modules but its more clear to understant if separated it
if moduleVidPid == "1BC7:0021" then		--Telit HE910-EUD
	ifname2 = ""
	protocol2 = ""
	proto_name2 = ""
	bridge = false
	use_4G_only = false

elseif moduleVidPid == "1BC7:1201" then	--Telit LE910
	protocol2 = "qmi"

elseif moduleVidPid == "1BC7:0036" then	--Telit LE910_V2
	protocol2 = "ncm"
	proto_name2 = "NCM"
	need_modify = true

elseif moduleVidPid == "1199:68C0" then	--Sierra MC7354
	protocol2 = "qmi"
	pref_2G = true
	pref_3G = true
	pref_4G = true

elseif moduleVidPid == "12D1:1573" then	--Huawei ME909u
	ifname2 = "eth2"
	protocol2 = "ndis"
	proto_name2 = "NDIS"
	pref_2G = true
	pref_3G = true
	pref_3G_4G = true
	pref_4G = true

elseif moduleVidPid == "12D1:15C1" then	--Huawei ME906s
	ifname2 = "eth2"
	protocol2 = "ndis"
	proto_name2 = "NDIS"
	pref_2G = true
	pref_3G = true
	pref_4G = true

elseif moduleVidPid == "12D1:15C3" then	--Huawei ME909s
	ifname2 = "eth2"
	protocol2 = "ndis"
	proto_name2 = "NDIS"
	pref_2G = true
	pref_3G = true
	pref_4G = true

elseif moduleVidPid == "05C6:9215" then	--Quectel EC20
	device = "/dev/cdc-wdm0"

elseif moduleVidPid == "05C6:9003" then	--Quectel UC20
	device = "/dev/cdc-wdm0"
	bridge = false
	use_4G_only = false

elseif moduleVidPid == "2C7C:0125" then	--Quectel EC25
	device = "/dev/cdc-wdm0"
	change_band = true
	only_2G_3G = true
	only_2G_4G = true
	only_3G_4G = true
end

--to set parameters due to project
if product_name then
	local product_code = product_name:sub(1, 6)
	if product_code == "RUT8" then
		bridge = false
	end
end

local util = require "luci.util"
local sys = require "luci.sys"

local function debug(string, ...)
	luci.sys.call(string.format("/usr/bin/logger -t Webui \"%s\"", string.format(string, ...)))
end

m = Map("simcard", translate("Mobile Configuration"), translate(""))
m.addremove = false
nw.init(m.uci)

if not modem_detected then
	m.message = "err: Modem not detected"
end


s = m:section(NamedSection, "sim1", "", translate("Mobile Configuration"));
s.template = "cbi/mobile_tabs_switch"
s.addremove = false
s:tab("primarytab", translate("SIM 1"))

if dual_sim == "1" then
	for_loop = 2
	s:tab("secondarytab", translate("SIM 2"))
else
	for_loop = 1
end

for j = 1, for_loop, 1 do
	local current_tab = "primarytab"
	local current_sim = "sim" .. j

	local customrm = function(self, section)
		if current_sim == "sim1" then
			self.map:del(current_sim, self.option)
		elseif current_sim == "sim2" then
			self.map:del(current_sim, self.option:sub(1, -2))
		end
	end

	local customwrite = function (self, section, value)
		if current_sim == "sim1" then
			if util.instanceof(self, Value) then
				Value.write(self, current_sim, value)
			elseif util.instanceof(self, ListValue) then
				ListValue.write(self, current_sim, value)
			elseif util.instanceof(self, Flag) then
				Flag.write(self, current_sim, value)
			end
		elseif current_sim == "sim2" then
			option = self.option:sub(1, -2)
			self.map:set(current_sim, option, value)
		end
	end

	local customcfg = function (self, section)
		value = ""
		if current_sim == "sim1" then
			if util.instanceof(self, Value) then
				value = Value.cfgvalue(self, current_sim)
			elseif util.instanceof(self, ListValue) then
				value = ListValue.cfgvalue(self, current_sim)
			elseif util.instanceof(self, Flag) then
				value = Flag.cfgvalue(self, current_sim)
			end
		elseif current_sim == "sim2" then
			option = self.option:sub(1, -2)
			value = self.map:get(current_sim, option) or ""
		end
		return value
	end

	if j ~= 1 then
		current_tab = "secondarytab"
		i = j
	else
		i = ""
	end

	-- if it has just 3G no need to show protocol
	if ifname2 ~= "" then
		prot = s:taboption(current_tab, ListValue, "proto" .. i, translate("Connection type"), translate("An underlying agent for mobile data connection creation and management"))
			prot.template = "cbi/mobile_configuration"

			if primary_sim == current_sim then
				prot.javascript="mode_list_check('simcard', '".. current_sim .."', 'proto".. i .."', 'method".. i .."', " .. multiwan_on .. "); check_for_message('cbid.simcard." .. current_sim .. ".method".. i .."'); validate_data_on_demand(this.value)"
			else
				prot.javascript="mode_list_check('simcard', '" .. current_sim .."', 'proto".. i .."', 'method".. i .."', " .. multiwan_on .. "); check_for_message('cbid.simcard." .. current_sim .. ".method".. i .."')"
			end

			prot:value(protocol1, translate(proto_name1))
			prot:value(protocol2, translate(proto_name2))
			prot.cfgvalue = customcfg

		function prot.write(self, section, value)

			if value == protocol1 then
				m.uci:set(self.config, current_sim, "proto", protocol1)
				m.uci:set(self.config, current_sim, "ifname", ifname1)
				m.uci:set(self.config, current_sim, "device", "/dev/modem_data")
			else
				m.uci:set(self.config, current_sim, "proto", protocol2)
				m.uci:set(self.config, current_sim, "ifname", ifname2)
				m.uci:set(self.config, current_sim, "device", device)
			end
		end
	else
		m.uci:set("simcard", current_sim, "proto", protocol1)
		m.uci:set("simcard", current_sim, "ifname", ifname1)
		m.uci:set("simcard", current_sim, "device", device)
	end


	method = s:taboption(current_tab, ListValue, "method" .. i, translate("Mode"), translate("An underlying agent for mobile data connection creation and management"))
		method.template = "cbi/lvalue_onclick"
		method.javascript="mode_list_check('simcard', '" .. current_sim .."', 'proto".. i .."', 'method".. i .."', " .. multiwan_on .. "); check_for_message('cbid.simcard." .. current_sim ..".method".. i .."')"
		method.javascript_after_select="mode_list_check('simcard', '" .. current_sim .."', 'proto".. i .."', 'method".. i .."', " .. multiwan_on .. "); check_for_message('cbid.simcard." .. current_sim ..".method".. i .."')"

		method:value("nat", translate("NAT"))

		if multiwan_on == "0" then
			if bridge then
				method:value("bridge", translate("Bridge"))
				method:depends("proto" .. i, protocol2)
			end
			if passthrough then
				method:value("pbridge", translate("Passthrough"))
				method:depends("proto" .. i, protocol2)
			end
		else
			method.hardDisabled = true
			method.info = "Passthrough and Bridge modes are disabled when multiwan is enabled"
			method.url = ds.build_url('/admin/network/wan')
		end
		method.default = "nat"
		method.write = customwrite
		method.cfgvalue = customcfg
		method.remove = customrm


	o = s:taboption(current_tab, Value, "bind_mac" .. i, translate("Bind to MAC"), translate("Forward all incoming packets to specified MAC address"))
		o:depends("method" .. i, "bridge")
		o.datatype = "macaddr2"
		o.write = customwrite
		o.cfgvalue = customcfg
		o.remove = customrm
	o = s:taboption(current_tab, Flag, "auto_apn" .. i, translate("Auto APN"))
		o.rmempty = false
		o.write = customwrite
		o.cfgvalue = customcfg
		o.remove = customrm

	if not uci:get("simcard", current_sim, "force_apn") and not uci:get("simcard", current_sim, "apn") and
		not m:formvalue("cbid.simcard." .. current_sim ..".force_apn") and not m:formvalue("cbid.simcard." .. current_sim ..".apn") then
		o = s:taboption(current_tab, DummyValue, "_auto_apn_set" .. i, translate(" "))
			o:depends("auto_apn" .. i, "1")
			o.value = (uci:get("network", "ppp", "apn") and translate("Provided APN: ") .. uci:get("network", "ppp", "apn") or
							translate("Connection is or will be established without using APN"))
	end

	info = _ubus:call("network.interface.ppp", "status", { })
	if info then
		apn_id = info.data.apn_id or 0
		apn_list = info.data.apn_list
	end

	o = s:taboption(current_tab, ListValue, "force_apn" .. i, translate("APN"), translate("APN (Access Point Name) is configurable network identifier used by a mobile device when connecting to a carrier"))
		o.placeholder = "apn"
		o:depends("auto_apn" .. i, "")
		o.cfgvalue = customcfg
		o.remove = customrm
		o.write = customwrite

		if apn_list and next(apn_list) then
			for i, apn in pairs(apn_list) do
				o:value(apn.id, apn.carrier.." ("..apn.apn..")")
			end
		end
		o:value("-1", "-- custom --")

	o = s:taboption(current_tab, Value, "apn" .. i, translate("Custom APN"), translate("APN entered by the user"))
		o:depends("force_apn" .. i, "-1")
		o.write = customwrite
		o.cfgvalue = customcfg
		o.remove = customrm

	auth = s:taboption(current_tab, ListValue, "auth_mode" .. i, translate("Authentication method"), translate("Authentication method that your GSM carrier uses to authenticate new connections on it\\'s network"))
		auth:value("none", translate("None"))
		auth:value("chap", translate("CHAP"))
		auth:value("pap", translate("PAP"))
		auth.write = customwrite
		auth.cfgvalue = customcfg
		auth.remove = customrm
		auth:depends("force_apn" .. i, "-1")

	o_username = s:taboption(current_tab, Value, "username" .. i, translate("Username"), translate("Your username that you would use to connect to your GSM carrier\\'s network"))
		o_username:depends("auth_mode" .. i, "chap")
		o_username:depends("auth_mode" .. i, "pap")

	if need_modify then
		function o_username.cfgvalue(self, section)
			local value = customcfg(self, section)
			if value then
				value = value:gsub("\\22", "\"")
				value = value:gsub("\\27", "'")
			else
				value = ""
			end
			return value
		end

		function o_username.write(self, section, value)
			if value ~= "" and value ~= nil then
				value = value:gsub("\"", "\\22")
				value = value:gsub("'", "\\27")
			end
			customwrite(self, section, value)
		end
	else
		o_username.write = customwrite
		o_username.cfgvalue = customcfg
		o_username.remove = customrm
	end

	o_password = s:taboption(current_tab, Value, "password" .. i, translate("Password"), translate("Your password that you would use to connect to your GSM carrier\\'s network"))
		o_password:depends("auth_mode" .. i, "chap")
		o_password:depends("auth_mode" .. i, "pap")
		o_password.password = true;

	if need_modify then
		function o_password.cfgvalue(self, section)
			local value = customcfg(self, section)
			if value then
				value = value:gsub("\\22", "\"")
				value = value:gsub("\\27", "'")
			else
				value = ""
			end
			return value
		end


		function o_password.write(self, section, value)
			if value ~= "" and value ~= nil then
				value = value:gsub("\"", "\\22")
				value = value:gsub("'", "\\27")
			end
			customwrite(self, section, value)
		end
	else
		o_password.write = customwrite
		o_password.cfgvalue = customcfg
		o_password.remove = customrm
	end

	o = s:taboption(current_tab, Value, "pincode" .. i, translate("PIN number"), translate("SIM card\\'s PIN (Personal Identification Number) is a secret numeric password shared between a user and a system that can be used to authenticate the user"))
		o.datatype = "lengthvalidation(4,12,'^[0-9]+$')"
		o.write = customwrite
		o.cfgvalue = customcfg
		o.remove = customrm

	o = s:taboption(current_tab, Value, "pukcode" .. i, translate("PUK code"), translate("SIM card\\'s PUK (Personal Unlocking Key) is used to reset a personal identification number (PIN) that has been lost or forgotten"))
		o.datatype = "lengthvalidation(4,12,'^[0-9]+$')"
		o.write = customwrite
		o.cfgvalue = customcfg
		o.remove = customrm

	o = s:taboption(current_tab, Value, "dialnumber" .. i, translate("Dialing number"), translate("Dialing number is used to establish a mobile PPP (Point-to-Point Protocol) connection. For example *99#"))
		o.default = "*99#"
		o.write = customwrite
		o.cfgvalue = customcfg
		o.remove = customrm
		o:depends("proto" .. i, protocol1)

	o = s:taboption(current_tab, Value, "mtu" .. i, translate("MTU"), translate("The size (in bytes or octets) of the largest protocol data unit that the layer can pass onwards"))
		o:depends("proto" .. i, "3g")
		o:depends("proto" .. i, "qmi2")
		o:depends("proto" .. i, "ncm")
		o.default = "1500"
		o.datatype = "range(0,1500)"
		o.write = customwrite
		o.cfgvalue = customcfg
		o.remove = customrm

	o = s:taboption(current_tab, ListValue, "service" .. i, translate("Service mode"), translate("Your network\\'s preference. If your local mobile network supports GSM (2G), UMTS (3G) or LTE (4G) you can specify to which network you prefer to connect to"))
	o.template = "cbi/lvalue_onclick"
	if need_modify then
		o.javascript="service_mode_list_check('simcard', 'sim".. i .."', 'proto".. i .."', 'service".. i .."', " .. multiwan_on .. ");"
		o.javascript_after_select="service_mode_list_check('simcard', 'sim".. i .."', 'proto".. i .."', 'service".. i .."', " .. multiwan_on .. ");"
	end

	o:value("gprs-only", translate("2G only"))

	if pref_2G then
		o:value("gprs", translate("2G preferred"))
	end

	o:value("umts-only", translate("3G only"))

	if pref_3G == "0" then
		o:value("umts", translate("3G preferred"))
	end

	if pref_3G_4G then
		o:value("lte-umts", translate("4G (LTE) and 3G only"))
	end

	if use_4G_only then
		o:value("lte-only", translate("4G (LTE) only"))
	end

	if pref_4G then
		o:value("lte", translate("4G (LTE) preferred"))
	end

	if only_2G_3G then
		o:value("gprs-umts", translate("2G + 3G"))
	end

	if only_2G_4G then
		o:value("gprs-lte", translate("2G + 4G"))
	end

	if only_3G_4G then
		o:value("umts-lte", translate("3G + 4G"))
	end


	o:value("auto", translate("Automatic"))
	o.default = "auto"
	o.write = customwrite
	o.cfgvalue = customcfg
	o.remove = customrm

	dummy = s:taboption(current_tab, DummyValue, "_dummy", " ")
	dummy.template = "cbi/custom_label"
	dummy.customValue = translate("Call utilities will not work as 4G-only service mode is on")
	dummy:depends("service" .. i, "lte-only")

	o = s:taboption(current_tab, Flag, "roaming" .. i, translate("Deny data roaming"), translate("Deny data connection on roaming"))
	o.write = customwrite
	o.cfgvalue = customcfg
	o.remove = customrm

	o = s:taboption(current_tab, Flag, "pdptype" .. i, translate("Use IPv4 only"), translate("Specifies the type of packet data protocol"))
	o.write = customwrite
	o.cfgvalue = customcfg
	o.remove = customrm

	prot = s:taboption(current_tab, ListValue, "passthrough_mode" .. i, translate("DHCP mode"), translate(""))
		prot.template = "cbi/lvalue_onclick"
		prot.javascript="check_mod(this.id,'cbid.simcard.sim1.mac')"
		prot:value("static", translate("Static"))
		prot:value("dynamic", translate("Dynamic"))
		prot:value("no_dhcp", translate("No DHCP"))
		prot.default = "static"
		prot:depends("method" .. i, "pbridge")
		prot.write = customwrite
		prot.cfgvalue = customcfg
		prot.remove = customrm

	mac_address = s:taboption(current_tab, Value, "mac" .. i, translate("MAC Address"), translate(""))
		mac_address:depends("passthrough_mode" .. i, "static")
		mac_address.datatype = "macaddr2"
		mac_address.write = customwrite
		mac_address.cfgvalue = customcfg
		mac_address.remove = customrm

	ltime = s:taboption(current_tab, Value, "leasetime" .. i, translate("Lease time"), translate("Expire time for leased addresses."))
		ltime.rmempty = true
		ltime.displayInline = true
		ltime.datatype = "integer"
		ltime.default = "12"
		ltime:depends("passthrough_mode" .. i, "static")
		ltime:depends("passthrough_mode" .. i, "dynamic")
		ltime:depends("method" .. i, "bridge")
		ltime.write = customwrite
		ltime.cfgvalue = customcfg
		ltime.remove = customrm

	o = s:taboption(current_tab, ListValue, "letter" .. i, translate(""), translate(""))
		o:value("h", translate("Hours"))
		o:value("m", translate("Minutes"))
		o:value("d", translate("Days"))
		o.displayInline = true
		o:depends("passthrough_mode" .. i, "static")
		o:depends("passthrough_mode" .. i, "dynamic")
		o:depends("method" .. i, "bridge")
		o.cfgvalue = customcfg
		o.write = customwrite

end

s1 = m:section(NamedSection, "ppp", "interface", translate("Mobile Data On Demand"))
s1.addremove = false

o = s1:option(Flag, "demand_enable", translate("Enable"), translate("Mobile data on demand function enables you to keep mobile data connection on only when it\\'s in use. Available in PPP mode only"))
	o.nowrite = true
	function o.write(self, section, value)
	end

	function o.cfgvalue(self, section)
		local value = m.uci:get("network", section, "demand")
		if value then
			return "1"
		else
			return "0"
		end
	end

time = s1:option(Value, "demand", translate("No data timeout (sec)"), translate("A mobile data connection will be terminated if no data is transfered during the timeout period (default 10)"))
	time.datatype = "range(10,3600)"
	time.default = "10"

if change_band then
	local modem_firmware = util.trim(sys.exec("gsmctl -n -A 'AT+CGMR'"))
	local modem_region = nil
	local modem3 = modem_firmware:sub(5, 7)
	local modem2 = modem_firmware:sub(5, 6)
	local modem1 = modem_firmware:sub(5, 5)


	if not modem_region and modem3 and modem3 == "AUT" then
		modem_region = modem3
	end
	if not modem_region and modem2 then
		if modem2 == "EU" or modem2 == "EC" or modem2 == "AU" then
			modem_region = modem2
		end
	end
	if not modem_region and modem1 then
		if modem1 == "E" or modem1 == "J" or modem1 == "A" or modem1 == "V" or modem1 == "G" then
			modem_region = modem1
		end
	end

	s2 = m:section(NamedSection, "bands", "bands", translate("Network Frequency Bands"), translate("This is band selector option. You can't force specific band usage, you could choose it if module detects more than one band on selected network service. If all bands are unchecked any band will be used."))
	s2.addremove = false

	for i = 1, for_loop, 1 do
		local current_tab = (i == 1) and "primarytab" or "secondarytab"
		local current_sim = "sim" .. i

		if modem_region then
			s2:tab(current_tab, translate("SIM ".. i))

			auto1 = s2:taboption(current_tab, ListValue, "auto_sim" .. i,
				translate("Connection method"), translate("Specify to which network frequency bands connect to"))
				auto1:value("enable", translate("Automatic"))
				auto1:value("disable", translate("Manual"))
				auto1.default = "enable"

		end

		if modem_region == "E" then
			o = s2:taboption(current_tab, Flag, "gsm900_sim" .. i, translate("GSM900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "gsm1800_sim" .. i, translate("GSM1800"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma850_sim" .. i, translate("WCDMA 850"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma900_sim" .. i, translate("WCDMA 900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma2100_sim" .. i, translate("WCDMA 2100"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb1_sim" .. i, translate("LTE B1"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb3_sim" .. i, translate("LTE B3"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb5_sim" .. i, translate("LTE B5"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb7_sim" .. i, translate("LTE B7"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb8_sim" .. i, translate("LTE B8"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb20_sim" .. i, translate("LTE B20"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb38_sim" .. i, translate("LTE B38"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb40_sim" .. i, translate("LTE B40"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb41_sim" .. i, translate("LTE B41"))
			o:depends("auto_sim" .. i, "disable")

		elseif modem_region == "EU" then
			o = s2:taboption(current_tab, Flag, "gsm900_sim" .. i, translate("GSM900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "gsm1800_sim" .. i, translate("GSM1800"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma900_sim" .. i, translate("WCDMA 900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma2100_sim" .. i, translate("WCDMA 2100"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb1_sim" .. i, translate("LTE B1"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb3_sim" .. i, translate("LTE B3"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb7_sim" .. i, translate("LTE B7"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb8_sim" .. i, translate("LTE B8"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb20_sim" .. i, translate("LTE B20"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb28_sim" .. i, translate("LTE B28"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb38_sim" .. i, translate("LTE B38"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb40_sim" .. i, translate("LTE B40"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb41_sim" .. i, translate("LTE B41"))
			o:depends("auto_sim" .. i, "disable")

		elseif modem_region == "EC" then
			o = s2:taboption(current_tab, Flag, "gsm900_sim" .. i, translate("GSM900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "gsm1800_sim" .. i, translate("GSM1800"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma900_sim" .. i, translate("WCDMA 900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma2100_sim" .. i, translate("WCDMA 2100"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb1_sim" .. i, translate("LTE B1"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb3_sim" .. i, translate("LTE B3"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb7_sim" .. i, translate("LTE B7"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb8_sim" .. i, translate("LTE B8"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb20_sim" .. i, translate("LTE B20"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb28_sim" .. i, translate("LTE B28"))
			o:depends("auto_sim" .. i, "disable")


		elseif modem_region == "A" then
			o = s2:taboption(current_tab, Flag, "wcdma850_sim" .. i, translate("WCDMA 850"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma1700_sim" .. i, translate("WCDMA 1700"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma1900_sim" .. i, translate("WCDMA 1900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb2_sim" .. i, translate("LTE B2"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb4_sim" .. i, translate("LTE B4"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb12_sim" .. i, translate("LTE B12"))
			o:depends("auto_sim" .. i, "disable")

		elseif modem_region == "V" then
			o = s2:taboption(current_tab, Flag, "lteb4_sim" .. i, translate("LTE B4"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb13_sim" .. i, translate("LTE B13"))
			o:depends("auto_sim" .. i, "disable")

		elseif modem_region == "AU" then
			o = s2:taboption(current_tab, Flag, "gsm850_sim" .. i, translate("GSM850"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "gsm900_sim" .. i, translate("GSM900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "gsm1800_sim" .. i, translate("GSM1800"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "gsm1900_sim" .. i, translate("GSM1900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma850_sim" .. i, translate("WCDMA 850"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma900_sim" .. i, translate("WCDMA 900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma1900_sim" .. i, translate("WCDMA 1900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma2100_sim" .. i, translate("WCDMA 2100"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb1_sim" .. i, translate("LTE B1"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb2_sim" .. i, translate("LTE B2"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb3_sim" .. i, translate("LTE B3"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb4_sim" .. i, translate("LTE B4"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb5_sim" .. i, translate("LTE B5"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb7_sim" .. i, translate("LTE B7"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb8_sim" .. i, translate("LTE B8"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb28_sim" .. i, translate("LTE B28"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb40_sim" .. i, translate("LTE B40"))
			o:depends("auto_sim" .. i, "disable")

		elseif modem_region == "AUT" then
			o = s2:taboption(current_tab, Flag, "wcdma850_sim" .. i, translate("WCDMA 850"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma2100_sim" .. i, translate("WCDMA 2100"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb1_sim" .. i, translate("LTE B1"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb3_sim" .. i, translate("LTE B3"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb5_sim" .. i, translate("LTE B5"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb7_sim" .. i, translate("LTE B7"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb28_sim" .. i, translate("LTE B28"))
			o:depends("auto_sim" .. i, "disable")

		elseif modem_region == "J" then
			o = s2:taboption(current_tab, Flag, "wcdma800_sim" .. i, translate("WCDMA 800"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma900_sim" .. i, translate("WCDMA 900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma2100_sim" .. i, translate("WCDMA 2100"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb1_sim" .. i, translate("LTE B1"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb3_sim" .. i, translate("LTE B3"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb8_sim" .. i, translate("LTE B8"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb18_sim" .. i, translate("LTE B18"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb19_sim" .. i, translate("LTE B19"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb26_sim" .. i, translate("LTE B26"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb41_sim" .. i, translate("LTE B41"))
			o:depends("auto_sim" .. i, "disable")

		elseif modem_region == "G" then
			o = s2:taboption(current_tab, Flag, "gsm850_sim" .. i, translate("GSM850"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "gsm900_sim" .. i, translate("GSM900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "gsm1800_sim" .. i, translate("GSM1800"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "gsm1900_sim" .. i, translate("GSM1900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma800_sim" .. i, translate("WCDMA 800"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma850_sim" .. i, translate("WCDMA 850"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma900_sim" .. i, translate("WCDMA 900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma1700_sim" .. i, translate("WCDMA 1700"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma1900_sim" .. i, translate("WCDMA 1900"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "wcdma2100_sim" .. i, translate("WCDMA 2100"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb1_sim" .. i, translate("LTE B1"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb2_sim" .. i, translate("LTE B2"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb3_sim" .. i, translate("LTE B3"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb4_sim" .. i, translate("LTE B4"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb5_sim" .. i, translate("LTE B5"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb7_sim" .. i, translate("LTE B7"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb8_sim" .. i, translate("LTE B8"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb12_sim" .. i, translate("LTE B12"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb13_sim" .. i, translate("LTE B13"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb18_sim" .. i, translate("LTE B18"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb19_sim" .. i, translate("LTE B19"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb20_sim" .. i, translate("LTE B20"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb25_sim" .. i, translate("LTE B25"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb26_sim" .. i, translate("LTE B26"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb28_sim" .. i, translate("LTE B28"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb38_sim" .. i, translate("LTE B38"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb39_sim" .. i, translate("LTE B39"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb40_sim" .. i, translate("LTE B40"))
			o:depends("auto_sim" .. i, "disable")
			o = s2:taboption(current_tab, Flag, "lteb41_sim" .. i, translate("LTE B41"))
			o:depends("auto_sim" .. i, "disable")

		end
	end
end

if use_4G_only then
	m2 = Map("reregister")
		m2.addremove = false

	s1 = m2:section(TypedSection, "reregister", translate("Force LTE network"))
		s1.addremove = false

	o3 = s1:option(Flag, "enabled", translate("Enable"),
		translate("Try to connect to LTE network every x seconds"
		.. " (used only if service mode is set to AUTO, 4G (LTE) preferred), 2G + 4G or 3G + 4G"))
		o3.rmempty = false

	o = s1:option(Flag, "force_reregister", translate("Reregister"),
		translate("If this enabled, modem will be reregister before try to connect to LTE network."))
		o.rmempty = false

	interval = s1:option(Value, "interval", translate("Interval (sec)"),
		translate("Time in seconds between tries to connect to LTE network. Range [180 - 3600]"))
		interval.datatype = "range(180,3600)"

	o = s1:option(DummyValue, "_javascript", "")
		o.template = "admin_network/hide_force_lte"
end

function calculate_gsmbandval(sim)
	local gsmbandval = 0x0
	local gsm900 = m:formvalue("cbid.simcard.bands.gsm900_"..sim) or ""
	local gsm1800 = m:formvalue("cbid.simcard.bands.gsm1800_"..sim) or ""
	local gsm850 = m:formvalue("cbid.simcard.bands.gsm850_"..sim) or ""
	local gsm1900 = m:formvalue("cbid.simcard.bands.gsm1900_"..sim) or ""

	if gsm900 == "1" then
		gsmbandval = gsmbandval + 0x1
	end
	if gsm1800 == "1" then
		gsmbandval = gsmbandval + 0x2
	end
	if gsm850 == "1" then
		gsmbandval = gsmbandval + 0x4
	end
	if gsm1900 == "1" then
		gsmbandval = gsmbandval + 0x8
	end

	return gsmbandval
end

function calculate_wcdmabandval(sim)
	local wcdmabandval = 0x0
	local wcdma2100 = m:formvalue("cbid.simcard.bands.wcdma2100_"..sim) or ""
	local wcdma1900 = m:formvalue("cbid.simcard.bands.wcdma1900_"..sim) or ""
	local wcdma850 = m:formvalue("cbid.simcard.bands.wcdma850_"..sim) or ""
	local wcdma900 = m:formvalue("cbid.simcard.bands.wcdma900_"..sim) or ""
	local wcdma800 = m:formvalue("cbid.simcard.bands.wcdma800_"..sim) or ""
	local wcdma1700 = m:formvalue("cbid.simcard.bands.wcdma1700_"..sim) or ""

	if wcdma2100 == "1" then
		wcdmabandval = wcdmabandval + 0x10
	end
	if wcdma1900 == "1" then
		wcdmabandval = wcdmabandval + 0x20
	end
	if wcdma850 == "1" then
		wcdmabandval = wcdmabandval + 0x40
	end
	if wcdma900 == "1" then
		wcdmabandval = wcdmabandval + 0x80
	end
	if wcdma800 == "1" then
		wcdmabandval = wcdmabandval + 0x100
	end
	if wcdma1700 == "1" then
		wcdmabandval = wcdmabandval + 0x200
	end

	return wcdmabandval
end

function calculate_ltebandval(sim)
	local ltebandval = 0x0
	local lteb1 = m:formvalue("cbid.simcard.bands.lteb1_"..sim) or ""
	local lteb2 = m:formvalue("cbid.simcard.bands.lteb2_"..sim) or ""
	local lteb3 = m:formvalue("cbid.simcard.bands.lteb3_"..sim) or ""
	local lteb4 = m:formvalue("cbid.simcard.bands.lteb4_"..sim) or ""
	local lteb5 = m:formvalue("cbid.simcard.bands.lteb5_"..sim) or ""
	local lteb7 = m:formvalue("cbid.simcard.bands.lteb7_"..sim) or ""
	local lteb8 = m:formvalue("cbid.simcard.bands.lteb8_"..sim) or ""
	local lteb12 = m:formvalue("cbid.simcard.bands.lteb12_"..sim) or ""
	local lteb13 = m:formvalue("cbid.simcard.bands.lteb13_"..sim) or ""
	local lteb18 = m:formvalue("cbid.simcard.bands.lteb18_"..sim) or ""
	local lteb19 = m:formvalue("cbid.simcard.bands.lteb19_"..sim) or ""
	local lteb20 = m:formvalue("cbid.simcard.bands.lteb20_"..sim) or ""
	local lteb25 = m:formvalue("cbid.simcard.bands.lteb25_"..sim) or ""
	local lteb26 = m:formvalue("cbid.simcard.bands.lteb26_"..sim) or ""
	local lteb28 = m:formvalue("cbid.simcard.bands.lteb28_"..sim) or ""
	local lteb38 = m:formvalue("cbid.simcard.bands.lteb38_"..sim) or ""
	local lteb39 = m:formvalue("cbid.simcard.bands.lteb39_"..sim) or ""
	local lteb40 = m:formvalue("cbid.simcard.bands.lteb40_"..sim) or ""
	local lteb41 = m:formvalue("cbid.simcard.bands.lteb41_"..sim) or ""

	if lteb1 == "1" then
		ltebandval = ltebandval + 0x1
	end
	if lteb2 == "1" then
		ltebandval = ltebandval + 0x2
	end
	if lteb3 == "1" then
		ltebandval = ltebandval + 0x4
	end
	if lteb4 == "1" then
		ltebandval = ltebandval + 0x8
	end
	if lteb5 == "1" then
		ltebandval = ltebandval + 0x10
	end
	if lteb7 == "1" then
		ltebandval = ltebandval + 0x40
	end
	if lteb8 == "1" then
		ltebandval = ltebandval + 0x80
	end
	if lteb12 == "1" then
		ltebandval = ltebandval + 0x800
	end
	if lteb13 == "1" then
		ltebandval = ltebandval + 0x1000
	end
	if lteb18 == "1" then
		ltebandval = ltebandval + 0x20000
	end
	if lteb19 == "1" then
		ltebandval = ltebandval + 0x40000
	end
	if lteb20 == "1" then
		ltebandval = ltebandval + 0x80000
	end
	if lteb25 == "1" then
		ltebandval = ltebandval + 0x1000000
	end
	if lteb26 == "1" then
		ltebandval = ltebandval + 0x2000000
	end
	if lteb28 == "1" then
		ltebandval = ltebandval + 0x8000000
	end
	if lteb38 == "1" then
		ltebandval = ltebandval + tonumber("0x2000000000", 10)
	end
	if lteb39 == "1" then
		ltebandval = ltebandval + tonumber("0x4000000000", 10)
	end
	if lteb40 == "1" then
		ltebandval = ltebandval + tonumber("0x8000000000", 10)
	end
	if lteb41 == "1" then
		ltebandval = ltebandval + tonumber("0x10000000000", 10)
	end

	return dec_to_hex(ltebandval)
end

function dec_to_hex(dec)
	local b, k, out, i = 16, "0123456789ABCDEF", "", 0
	local d
	while dec > 0 do
		i = i + 1
		dec, d = math.floor(dec/b), math.mod(dec, b) + 1
		out = string.sub(k,d,d)..out
	end
	return out
end

function revert_bands(sim)
	m.uci:foreach("simcard", "bands", function(s)
		for key, value in pairs(s) do
			if key:find(sim) ~= nil and key:find("auto_sim") == nil then
				m.uci:delete("simcard", "bands", key)
				m.uci:save("simcard")
			end
		end
	end)
end

function m.on_before_save(self)

	for i = 1, for_loop, 1 do
		local current_sim = "sim" .. i

		local passthrough = m:formvalue("cbid.simcard.".. current_sim ..".passthrough_mode" .. i) or ""
		if passthrough == "static" then
			local mac_addr = m:formvalue("cbid.simcard.".. current_sim ..".mac" .. i) or ""
			if mac_addr == nil or mac_addr == "" then
				m.message = "err:MAC address can't be blank in Static DHCP mode"
				return nil
			end
		end

		local bridge = m:formvalue("cbid.simcard.".. current_sim ..".method" ..i) or ""
		local proto = m:formvalue("cbid.simcard.".. current_sim ..".proto" ..i) or ""
		if bridge == "bridge" and proto == "qmi2" then
			local bind_mac = m:formvalue("cbid.simcard.".. current_sim ..".bind_mac" ..i) or ""
			if bind_mac == nil or bind_mac == "" then
				m.message = "err:Bind to MAC address can't be blank in brigde mode"
				return nil
			end
		end

		local dialnumber = m:formvalue("cbid.simcard.".. current_sim ..".dialnumber" ..i) or ""
		if dialnumber == nil or dialnumber == "" then
			if proto == "3g" then
				m.message = "err:Dialing number can't be empty with PPP connection type"
				return nil
			end
		end
	end

	save_value = 1
end

function m.on_commit(map)
	local demand_enable = m:formvalue("cbid.simcard.ppp.demand_enable") or ""
	local demand = m:formvalue("cbid.simcard.ppp.demand") and tonumber(m:formvalue("cbid.simcard.ppp.demand")) or 10

	if demand_enable == "1" then
		m.uci:set("network", "ppp", "demand", demand)
	elseif demand_enable ~= "1" then
		m.uci:delete("network", "ppp", "demand")
	end
	m.uci:save("network")
	m.uci:commit("network")

	for i = 1, for_loop, 1 do

		local current_sim = "sim" .. i

		if not save_value == 1 then
			return
		end

		if not change_band then
			m.uci:save("simcard")
			return
		end

		local service = m:formvalue("cbid.simcard." .. current_sim ..".service") or ""
		local auto_sim = m:formvalue("cbid.simcard.bands.auto_" .. current_sim) or ""
		local gsmbandval = 0
		local wcdmabandval = 0
		local bandval = ""
		local ltebandval = ""
		local error_trigger = false
		local auto_band = "0"

		if auto_sim == "enable" then
			bandval = "ffff"
			ltebandval = "1a0000800d5"
		else
			gsmbandval = calculate_gsmbandval(current_sim)
			wcdmabandval = calculate_wcdmabandval(current_sim)
			ltebandval = calculate_ltebandval(current_sim)
			bandval = string.format("%x", gsmbandval + wcdmabandval)

			local error_message = "err: %s: In %s service mode you need to specifiy at least one "
				.. "%s band. Setting connection method to automatic."

			if gsmbandval == 0 and wcdmabandval == 0 and ltebandval == "" then
				auto_band = "1"
			elseif service == "gprs-only" and gsmbandval == 0 then
				m.message = string.format(error_message, current_sim:upper(), "2G", "2G (GSM)")
				error_trigger = true
			elseif service == "umts-only" and wcdmabandval == 0 then
				m.message = string.format(error_message, current_sim:upper(), "3G", "3G (WCDMA)")
				error_trigger = true
			elseif service == "lte-only" and ltebandval == "" then
				m.message = string.format(error_message, current_sim:upper(), "4G", "4G (LTE)")
				error_trigger = true
			elseif service == "gprs-umts" and (gsmbandval == 0 and wcdmabandval == 0) then
				m.message = string.format(error_message, current_sim:upper(), "2G + 3G",
					"2G (GMS) or 3G (WCDMA)")
				error_trigger = true
			elseif service == "gprs-lte" and (gsmbandval == 0 and ltebandval == "") then
				m.message = string.format(error_message, current_sim:upper(), "2G + 4G",
					"2G (GMS) or 4G (LTE)")
			elseif service == "umts-lte" and (wcdmabandval == 0 and ltebandval == "") then
				m.message = string.format(error_message, current_sim:upper(), "3G + 4G",
					"3G (WCDMA) or 4G (LTE)")
				error_trigger = true
			end

			if error_trigger == true then
				bandval = "ffff"
				ltebandval = "1a0000800d5"
				auto_band = "1"
				revert_bands(current_sim)
			end

			local warning_message = "wrn: %s: Using %s bands with %s only service mode may cause "
				.. "network connectivity to become unstable"

			if not error_trigger and service == "gprs-only"
			and (wcdmabandval ~= 0 or ltebandval ~= "") then
				m.message = string.format(warning_message, current_sim:upper(), "3G (WCDMA) or 4G (LTE)",
					"2G (GSM)")
			elseif not error_trigger and service == "umts-only"
			and (gsmbandval ~= 0 or ltebandval ~= "") then
				m.message = string.format(warning_message, current_sim:upper(), "2G (GSM) or 4G (LTE)",
					"3G (WCDMA)")
			elseif not error_trigger and service == "lte-only"
			and (gsmbandval ~= 0 or wcdmabandval ~= 0) then
				m.message = string.format(warning_message, current_sim:upper(), "2G (GSM) or 3G (WCDMA)",
					"4G (LTE)")
			elseif not error_trigger and service == "gprs-utms" and (ltebandval ~= "") then
				m.message = string.format(warning_message, current_sim:upper(), "4G (LTE)",
					"2G (GSM) + 3G (WCDMA)")
			elseif not error_trigger and service == "gprs-lte" and (wcdmabandval ~= 0) then
				m.message = string.format(warning_message, current_sim:upper(), "3G (WCDMA)",
					"2G (GSM) + 4G (LTE)")
			elseif not error_trigger and service == "umts-lte" and (gsmbandval ~= 0) then
				m.message = string.format(warning_message, current_sim:upper(), "2G (GSM)",
					"3G (WCDMA) + 4G (LTE)")
			end

			if bandval == "0" then
				bandval = "ffff"
			end
			if ltebandval == "" then
				ltebandval = "1a0000800d5"
			end
		end

		if auto_band == "1" then
			m.uci:set("simcard", "bands", "auto_" .. current_sim, "enable")
		end
		m.uci:set("simcard", current_sim, "bandval", bandval)
		m.uci:set("simcard", current_sim, "ltebandval", ltebandval)
		m.uci:save("simcard")
	end
end

if use_4G_only then
	m2.on_commit = function()
		local sim1_service = m.uci:get("simcard", "sim1", "service") or nil
		local sim2_service = m.uci:get("simcard", "sim2", "service") or nil
		local save = false

		if sim1_service == "lte-only" or sim1_service == "auto" or sim2_service == "lte-only"
			or sim2_service == "auto" or sim1_service == "gprs-lte" or sim2_service == "gprs-lte"
			or sim1_service == "umts-lte" or sim2_service == "umts-lte" then
			save = true
		end

		if save == false then
			m2.uci:set("reregister", "reregister", "enabled", "0")
			m2.uci:commit("reregister")
		end
	end
end

if use_4G_only then
	return m, m2
else
	return m
end
