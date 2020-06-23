 --[[
LuCI - Lua Configuration Interface

Copyright 2009 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: wifi_add.lua 7794 2011-10-26 21:54:51Z jow $
]]--

require "teltonika_lua_functions"
local fs   = require "nixio.fs"
local nw   = require "luci.model.network"
local fw   = require "luci.model.firewall"
local uci  = require "luci.model.uci".cursor()
local http = require "luci.http"
function wpakey(val)
	if #val == 64 then
		return (val:match("^[a-fA-F0-9]+$") ~= nil)
	else
		return (#val >= 8) and (#val <= 63)
	end
end

local WPA_PSK = 0
local WPA_EAP = 0
local wanNetName = get_wan_section("type", "wifi") or "wan"
local iw = luci.sys.wifi.getiwinfo(http.formvalue("device"))
local has_firewall = fs.access("/etc/config/firewall")
if not iw then
	luci.http.redirect(luci.dispatcher.build_url("admin/network/wireless"))
	return
end
stationSSID = http.formvalue("join")

m = SimpleForm("network", translatef("Join Network: \"%s\"", stationSSID ))
m.cancel = translate("Back to scan results")
m.reset = false
m.template = "admin_network/simpleform_wifi"
m.redirect = luci.dispatcher.build_url("admin/network/wireless")
function m.on_cancel()
	luci.http.redirect(luci.dispatcher.build_url("admin/network/wireless_join"))
end

nw.init(uci)
fw.init(uci)

m.hidden = {
	device      = http.formvalue("device"),
	join        = stationSSID,
	channel     = http.formvalue("channel"),
	mode        = http.formvalue("mode"),
	bssid       = http.formvalue("bssid"),
	wpa_suites	= http.formvalue("wpa_suites"),
	wpa_version = http.formvalue("wpa_version")
}
--if iw and iw.mbssid_support then
--[[if iw then
	replace = m:field(Flag, "replace", translate("Replace wireless configuration"),
		translate("An additional network will be created if you leave this unchecked."))

	function replace.cfgvalue() return "1" end
else
	replace = m:field(DummyValue, "replace", translate("Replace wireless configuration"))
	replace.default = translate("The hardware is not multi-SSID capable and existing " ..
		"configuration will be replaced if you proceed.")

	function replace.formvalue() return "1" end
end]]

if m.hidden.wpa_suites == "802.1X" then
	WPA_EAP = 1
	eap_type = m:field(Value, "eap_type", translate("EAP Protocol"), translate("Defines the EAP protocol to use"))
	eap_type:value("peap", translate("EAP-PEAP"))
	eap_type:value("ttls", translate("EAP-TTLS"))
	eap_type:value("tls", translate("EAP-TLS"))

	auth = m:field(Value, "auth", translate("Authentication Parameters"), translate("Authentication Parameters"))
	auth:value("auth=MSCHAPV2", translate("MSCHAPV2"))
	auth:value("auth=PAP", translate("PAP"))
	auth:depends({eap_type="peap"})
	auth:depends({eap_type="ttls"})

	identity = m:field(Value, "identity", translate("Identity"), translate("Identity to use"))
	password = m:field(Value, "password", translate("Password"), translate("Password to use"))

	FileUpload.size = "262144"
        FileUpload.sizetext = translate("Selected file is too large, max 256 KiB")
        FileUpload.sizetextempty = translate("Selected file is empty")

	ca_cert = m:field(FileUpload, "ca_cert", translate("CA Certificate"), translate("CA Certificate"))
	client_cert = m:field(FileUpload, "client_cert", translate("Client Certificate"), translate("Client Certificate"))
	priv_key = m:field(FileUpload, "priv_key", translate("Private Key"), translate("Private Key"))
	priv_key_pwd = m:field(Value, "priv_key_pwd", translate("Private Key Password"), translate("Private Key Password"))
elseif (tonumber(m.hidden.wpa_version) or 0) > 0 and
	(m.hidden.wpa_suites == "PSK" or m.hidden.wpa_suites == "PSK2")
then
	WPA_PSK = 1
	if #m.hidden.join == 0 then
		ssidname = m:field(Value, "ssid", translate("SSID"),
		translate("Your wireless network\\'s identification string."))
		ssidname.datatype = "lengthvalidation(0,32)"
	end

	key = m:field(Value, "key", translate("WPA passphrase"),
		translate("Specify the secret encryption key here."))

	key.password = true
	key.datatype = "wpakey"
	--m.hidden.wpa_suite = (tonumber(http.formvalue("wpa_version")) or 0) >= 2 and "psk2" or "psk"
else
	WPA_PSK = 1
	text = m:field(DummyValue, translate("No key required\" style=\'width: 415px"),
	              translate("The access point has no password protection."))
	key = m:field(Value, "key")
		key.hidden_field = true
		key.default = "12345678" --default reiksme yra butina, nes kitaip nesuvygdoma funkcija validate
end


--[[newnet = m:field(Value, "_netname_new", translate("Name of the new network"),
	translate("The allowed characters are: <code>A-Z</code>, <code>a-z</code>, " ..
		"<code>0-9</code> and <code>_</code>"
	))

newnet.default = m.hidden.mode == "Ad-Hoc" and "mesh" or "wwan"
newnet.datatype = "uciname"]]

--[[if has_firewall then
	fwzone = m:field(Value, "_fwzone",
		translate("Create / Assign firewall-zone"),
		translate("Choose the firewall zone you want to assign to this interface. Select <em>unspecified</em> to remove the interface from the associated zone or fill out the <em>create</em> field to define a new zone and attach the interface to it."))

	fwzone.template = "cbi/firewall_zonelist"
	fwzone.default = m.hidden.mode == "Ad-Hoc" and "mesh" or "wan"
end]]

if WPA_EAP == 1 then
function eap_type.validate(self, value, section)
		local signal_threshold
		if has_firewall then
			local zval  = wanNetName
			zone = fw:get_zone(zval)
		end
		local wdev = nw:get_wifidev(m.hidden.device)
		wdev:set("disabled", false)
		wdev:set("channel", m.hidden.channel)

		local _, wnet
		for _, wnet in ipairs(wdev:get_wifinets()) do
			if wnet:mode() == "sta" then
				signal_threshold = wnet:get("signal_threshold")
				wdev:del_wifinet(wnet)
			end
		end

		local wconf = {
			device  = m.hidden.device,
			ssid    = ssidname and ssidname:formvalue(section) or m.hidden.join,
			mode    = (m.hidden.mode == "Ad-Hoc" and "adhoc" or "sta"),
			network	= wanNetName,
			user_enable = "1",
			scan_sleep = "10",
			signal_threshold = signal_threshold or ""
		}

		wconf.encryption = (tonumber(m.hidden.wpa_version) or 0) >= 2 and "wpa2" or "wpa"
		wconf.eap_type = eap_type and eap_type:formvalue(section) or ""
		wconf.auth = auth and auth:formvalue(section) or ""
		wconf.identity = identity and identity:formvalue(section) or ""
		wconf.password = password and password:formvalue(section) or ""

		FileUpload.size = "262144"
	        FileUpload.sizetext = translate("Selected file is too large, max 256 KiB")
	        FileUpload.sizetextempty = translate("Selected file is empty")

		wconf.ca_cert = ca_cert and ca_cert:formvalue(section) or ""
		wconf.client_cert = client_cert and client_cert:formvalue(section) or ""
		wconf.priv_key = priv_key and priv_key:formvalue(section) or ""
		wconf.priv_key_pwd = priv_key_pwd and priv_key_pwd:formvalue(section) or ""


		if wconf.mode == "adhoc" then
			wconf.bssid = m.hidden.bssid
		end
		wconf.network = wanNetName
		local wnet = wdev:add_wifinet(wconf)
		if wnet then
			if zone then
				fw:del_network(wanNetName)
				zone:add_network(wanNetName)
			end
			uci:save("wireless")
			uci:save("network")
			uci:save("firewall")
			uci:commit("wireless")
			self.map.apply_changes = true

			return value
		end
end
end

if WPA_PSK == 1 then
function key.validate(self, value, section)
	local signal_threshold
	if wpakey(value) then
		local net, zone
		if has_firewall then
			--local zval  = fwzone:formvalue(section)
			local zval  = wanNetName
			zone = fw:get_zone(zval)
		--[[if not zone and zval == '-' then
				zval = m:formvalue(fwzone:cbid(section) .. ".newzone")
			if zval and #zval > 0 then
				zone = fw:add_zone(zval)
			end
		end]]
		end
		local wdev = nw:get_wifidev(m.hidden.device)
		wdev:set("disabled", false)
		wdev:set("channel", m.hidden.channel)

	--if replace:formvalue(section) then
			local _, wnet
			for _, wnet in ipairs(wdev:get_wifinets()) do
				if wnet:mode() == "sta" then
					signal_threshold = wnet:get("signal_threshold")
					wdev:del_wifinet(wnet)
				end
			end
	--end

		local wconf = {
			device  = m.hidden.device,
			ssid    = ssidname and ssidname:formvalue(section) or m.hidden.join,
			mode    = (m.hidden.mode == "Ad-Hoc" and "adhoc" or "sta"),
			network	= wanNetName,
			user_enable = "1",
			scan_sleep = "10",
			signal_threshold = signal_threshold or ""
		}

		if m.hidden.wpa_suites == "802.1X" then
			eap_type = s:taboption("encryption", Value, "eap_type", translate("EAP Protocol"), translate("Defines the EAP protocol to use"))
			auth = s:taboption("encryption", Value, "auth", translate("Authentication parameters"), translate("For example: auth=MSCHAPV2"))
			identity = s:taboption("encryption", Value, "identity", translate("Identity"), translate("Identity to use"))
			password = s:taboption("encryption", Value, "password", translate("Password"), translate("Password to use"))

			FileUpload.size = "262144"
		        FileUpload.sizetext = translate("Selected file is too large, max 256 KiB")
		        FileUpload.sizetextempty = translate("Selected file is empty")

			ca_cert = s:taboption("encryption", FileUpload, "ca_cert", translate("CA Certificate"), translate("CA Certificate"))
			client_cert = s:taboption("encryption", FileUpload, "client_cert", translate("Client Certificate"), translate("Client Certificate"))
			priv_key = s:taboption("encryption", FileUpload, "priv_key", translate("Private Key"), translate("Private Key"))
			priv_key_pwd = s:taboption("encryption", Value, "priv_key_pwd", translate("Private Key Password"), translate("Private Key Password"))

		elseif (tonumber(m.hidden.wpa_version) or 0) > 0 then
			wconf.encryption = (tonumber(m.hidden.wpa_version) or 0) >= 2 and "psk2" or "psk"
			wconf.key        = key and key:formvalue(section) or ""
		else
			wconf.encryption = "none"
		end

		if wconf.mode == "adhoc" then
			wconf.bssid = m.hidden.bssid
		end
	--[[
	local value = self:formvalue(section)
	net = nw:add_network(value, { proto = "dhcp" })]]
	--[[if not net then
		self.error = { [section] = "missing" }
	else]]
		--wconf.network = net:name()
			wconf.network = wanNetName
			local wnet = wdev:add_wifinet(wconf)
			if wnet then
				if zone then
				--[[fw:del_network(net:name())
				zone:add_network(net:name())]]
					fw:del_network(wanNetName)
					zone:add_network(wanNetName)
				end
				uci:save("wireless")
				uci:save("network")
				uci:save("firewall")
				uci:commit("wireless")
				self.map.apply_changes = true

				return value
			end
		else
			return false
		end
end
end

--[[if has_firewall then
	function fwzone.cfgvalue(self, section)
		self.iface = section
		local z = fw:get_zone_by_network(section)
		return z and z:name()
	end
end]]



return m
