
require "teltonika_lua_functions"
local uci = require "luci.model.uci".cursor()
local bus = require "ubus"
local _ubus = bus.connect()

local moduleType = uci:get("system", "module", "type")
local moduleVidPid =  uci:get("system", "module", "vid") .. ":" .. uci:get("system", "module", "pid")
local m, s, o, apn_id, apn_list, info
local modulsevice = "3G"
local modulsevice2 = "0"
local product_name = uci:get("hwinfo", "hwinfo", "mnf_code")
_ubus:call("file", "exec", { command="/usr/sbin/operctl", params={"-f"} })

function debug(string)
		os.execute("logger " .. string)
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

if moduleVidPid == "12D1:1573" or moduleVidPid == "12D1:15C1" or moduleVidPid == "12D1:15C3" then
	modulsevice = "LTE"
elseif moduleVidPid == "1BC7:1201" then
	modulsevice = "TelitLTE"
	modulsevice2 = "2"
elseif moduleVidPid == "1BC7:0036" then
	modulsevice = "TelitLTE_V2"
	modulsevice2 = "2"
elseif moduleVidPid == "1199:68C0" then
	modulsevice = "SieraLTE"
elseif moduleVidPid == "05C6:9215" then
	modulsevice = "QuectelLTE"
	modulsevice2 = "2"
elseif moduleVidPid == "2C7C:0125" then
	modulsevice = "QuectelLTE_EC25"
	modulsevice2 = "2"
end

--ismtys del telit modemu kurie nepalaiko 2g prefered ir 3g prefered pasirinkimo
if moduleVidPid == "1BC7:0021" then --Telit He910d
	modulsevice2 = "1"
end

if moduleType == "3g_ppp" then
	m = Map("simcard", translate("Mobile Configuration"),
		translate("Next, let's configure your mobile settings so you can start using internet right away."))
	m.wizStep = 2
	m.addremove = false

	s = m:section(NamedSection, "sim1", "", translate("Mobile Configuration (SIM1)"))
	s.addremove = false

	o = s:option(Flag, "auto_apn", translate("Auto APN"))

	if not uci:get("simcard", "sim1", "force_apn") and not uci:get("simcard", "sim1", "apn") and
		not m:formvalue("cbid.simcard.sim1.force_apn") and not m:formvalue("cbid.simcard.sim1.apn") then
		o = s:option(DummyValue, "_auto_apn_set", translate(" "))
			o:depends("auto_apn", "1")
			o.value = (uci:get("network", "ppp", "apn") and translate("Provided APN: ") .. uci:get("network", "ppp", "apn") or
							translate("Connection is or will be established without using APN"))
	end

	info = _ubus:call("network.interface.ppp", "status", { })
	if info then
		apn_id = info.data.apn_id or 0
		apn_list = info.data.apn_list
	end

	o = s:option(ListValue, "force_apn", translate("APN"), translate("APN (Access Point Name) is configurable network identifier used by a mobile device when connecting to a carrier"))
		o.placeholder = "apn"
		o:depends("auto_apn", "")

		if apn_list and next(apn_list) then
			for i, apn in pairs(apn_list) do
				o:value(apn.id, apn.carrier.." ("..apn.apn..")")
			end
		end
		o:value("-1", "-- custom --")

	o = s:option(Value, "apn", translate("Custom APN"), translate("APN entered by the user"))
		o:depends("force_apn", "-1")

	o = s:option(Value, "mtu", translate("MTU"), translate("The size (in bytes or octets) of the largest protocol data unit that the layer can pass onwards"))
		o.default = "1500"
		o.datatype = "integer"

	o = s:option(ListValue, "auth_mode", translate("Authentication method"), translate("Authentication method that your carrier uses to authenticate new connections on its network"))
		o:value("chap", translate("CHAP"))
		o:value("pap", translate("PAP"))
		o:value("none", translate("None"))
		o.default = "none"
		o:depends("force_apn", "-1")

	o = s:option(Value, "username", translate("Username"), translate("Type in your username"))
		o:depends("auth_mode", "chap")
		o:depends("auth_mode", "pap")

	o = s:option(Value, "password", translate("Password"), translate("Type in your password"))
		o:depends("auth_mode", "chap")
		o:depends("auth_mode", "pap")
		o.noautocomplete = true
		o.password = true

	o = s:option(Value, "pincode", translate("PIN number"), translate("SIM card PIN (Personal Identification Number) is a secret numeric password shared between a user and a system that can be used to authenticate the user"))
		o.datatype = "lengthvalidation(4,12,'^[0-9]+$')"

	o = s:option(Value, "mtu", translate("MTU"), translate("The size (in bytes or octets) of the largest protocol data unit that the layer can pass onwards"))
		o.default = "1500"
		o.datatype = "integer"

	o = s:option(ListValue, "service", translate("Service mode"), translate("Your network preference. If your local mobile network supports GSM (2G) and UMTS (3G) you can specify to which network you prefer to connect to"))

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

	o1 = s:option(Flag, "shw3g", translate("Show mobile info at login page"), translate("Show operator and signal strenght at login page"))
		o1.rmempty = false

		function o1.cfgvalue(...)
			return m.uci:get("teltonika", "sys", "shw3g")
		end

		function o1.write(self, section, value)
			m.uci:set("teltonika", "sys", "shw3g", value)
			m.uci:save("teltonika")
			m.uci:commit("teltonika")
		end

else
	m = Map("network_3g", translatef("Step - %s", modulservice),
			translatef("Next, let's configure your %s settings so you can start using internet right away.", modulservice))
	m.wizStep = 2

	--[[
	config custom_interface '3g'
		option pin 'kazkas'
		option apn 'kazkas'
		option user 'kazkas'
		option password 'kazkas'
		option auth_mode 'chap' ARBA 'pap' (jei nerandu nieko ar kazka kita, laikau kad auth nenaudojama)
		option net_mode 'gsm' ARBA 'umts' ARBA 'auto' (prefered tinklas. jei nerandu nieko arba kazka kita laikau kad auto)
		option data_mode 'enabled' ARBA 'disabled' (ar leisti siusti duomenis. jei nera nieko ar kazkas kitas, laikau kad enabled)
	]]
	m.addremove = false

	s = m:section(NamedSection, "3g", "custom_interface", translatef(" %s Configuration", modulservice));
	s.addremove = false

	o = s:option(Value, "apn", translate("APN"), translate("APN (Access Point Name) is configurable network identifier used by a mobile device when connecting to a carrier"))

	o = s:option(Value, "pin", translate("PIN number"), translate("SIM card PIN (Personal Identification Number) is a secret numeric password shared between a user and a system that can be used to authenticate the user"))
	o.datatype = "range(0,9999)"

	auth = s:option(ListValue, "auth_mode", translatef(" %s authentication method", modulservice), translate("Authentication method that your carrier uses to authenticate new connections on its network"))

	auth:value("chap", translate("CHAP"))
	auth:value("pap", translate("PAP"))
	auth:value("none", translate("none"))
	auth.default = "none"

	o = s:option(Value, "user", translate("Username"), translate("Type in your username"))
	o:depends("auth_mode", "chap")
	o:depends("auth_mode", "pap")

	o = s:option(Value, "password", translate("Password"), translate("Type in your password"))
	o:depends("auth_mode", "chap")
	o:depends("auth_mode", "pap")
	o.noautocomplete = true
	o.password = true

	o = s:option(ListValue, "net_mode", translate("Prefered network"), translate("Select network that you prefer"))

	o:value("gsm", translate("2G"))
	o:value("umts", translate("3G"))
	o:value("auto", translate("auto"))
	o.default = "auto"

	o1 = s:option(Flag, "shw3g", translate("Show mobile info at login page"), translate("Show operator and signal strenght at login page"))
	o1.rmempty = false

	function o1.cfgvalue(...)
		return m.uci:get("teltonika", "sys", "shw3g")
	end

	function o1.write(self, section, value)
		m.uci:set("teltonika", "sys", "shw3g", value)
		m.uci:save("teltonika")
		m.uci:commit("teltonika")
	end

	--[[o = s:option(Flag, "data_mode", translate("Data mode"))

	o.enabled = "enabled"
	o.disabled = "disabled"]]
end
function m.on_after_save()
	if m:formvalue("cbi.wizard.next") then

		m.uci:commit("simcard")
--kadangi jau po save tai reikia rankiniu budu perleist inis skripta
		luci.sys.call("/etc/init.d/operctl restart")
		luci.sys.call("/etc/init.d/sim_conf_switch restart >/dev/null")
		luci.sys.call("ifup ppp >/dev/null")
		luci.http.redirect(luci.dispatcher.build_url("admin/system/wizard/step-lan"))
	end
end
if m:formvalue("cbi.wizard.skip") then
	luci.http.redirect(luci.dispatcher.build_url("/admin/status/overview"))
end
return m
