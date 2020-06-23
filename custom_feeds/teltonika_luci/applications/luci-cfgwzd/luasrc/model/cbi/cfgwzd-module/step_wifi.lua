local nw = require "luci.model.network"
local fs = require "nixio.fs"

arg[1] = arg[1] or ""

arg[1] = "radio0.network1"

m = Map("wireless", "Step - Wireless",
	translate("Now let's configure your wireless radio. (Note: if you are currently connecting via wireless and you change parameters, like SSID, encryption, etc. your connection will be dropped and you will have to reconnect with a new set of parameters.)"))

m:chain("firewall")
function m.on_after_save()
	if m:formvalue("cbi.wizard.next") then
		m.uci:commit("wireless")
		luci.sys.call("wifi up")
		luci.http.redirect(luci.dispatcher.build_url("admin/system/wizard/step-rms"))
	end
end

--[[m.apply_on_parse = true]]

m.wizStep = 4
local ifsection

function m.on_commit(map)
	local wnet = nw:get_wifinet(arg[1])
	if ifsection and wnet then
		ifsection.section = wnet.sid
		--m.title = luci.util.pcdata(wnet:get_i18n())
		m.title = "Step - Wireless"
	end

	check = luci.http.formvalue("cbid.wireless.radio0.enable") or "0"
	if check then
		if check == "1" then	--//Disable NAT=1 means masq=0
			m.uci:delete("wireless", wnet.sid, "disabled")
			m.uci:set("wireless", wnet.sid, "user_enable", "1")
		else
			m.uci:set("wireless", wnet.sid, "disabled", "1")
			m.uci:set("wireless", wnet.sid, "user_enable", "0")
		end
			m.uci:save("wireless")
	end
-- 	m.uci:commit("wireless")
end

nw.init(m.uci)

local wnet = nw:get_wifinet(arg[1])
local wdev = wnet and wnet:get_device()

-- redirect to overview page if network does not exist anymore (e.g. after a revert)
if not wnet or not wdev then
	luci.http.redirect(luci.dispatcher.build_url("admin/network/wireless"))
	return
end

local iw = luci.sys.wifi.getiwinfo(arg[1])
local hw_modes = iw.hwmodelist or { }

s = m:section(NamedSection, wnet.sid, "wifi-device", translate("WiFi Configuration"))
s.addremove = false


no_nat = s:option(Flag, "enable", translate("Enable wireless"), translate("Do not forget to save before togging the wireless radio on and off"))
	no_nat.rmempty = false
function no_nat.cfgvalue(self, section)
	value = m.uci:get("wireless", wnet.sid, "disabled")
	if value == "1" then
		cval = "0"
	else
		cval = "1"
	end
	return cval
end
function no_nat.write(self, section, value)
	if value == "1" then	--//Disable NAT=1 means masq=0
		m.uci:delete("wireless", wnet.sid, "disabled")
		m.uci:set("wireless", wnet.sid, "user_enable", "1")
	else
		m.uci:set("wireless", wnet.sid, "disabled", "1")
		m.uci:set("wireless", wnet.sid, "user_enable", "0")
	end
	m.uci:save("wireless")
end


ssid = s:option(Value, "ssid", translate("SSID"), translate("Your wireless network identification string"))
ssid.rmempty = false
ssid.datatype = "lengthvalidation(0,32)"

function ssid.cfgvalue(self, section)
	cval = m.uci:get("wireless", wnet.sid, "ssid")
	return cval
end
function ssid.write(self, section, value)
	m.uci:set("wireless", wnet.sid, "ssid", value)
	m.uci:save("wireless")
end
local hwtype = wdev:get("type")
local htcaps = wdev:get("ht_capab") and true or false

-- NanoFoo
local nsantenna = wdev:get("antenna")

------------------- MAC80211 Device ------------------

	mode = s:option(ListValue, "hwmode", translate("Mode"), translate("Different modes provide different throughput and security options"))
	mode:value("auto", translate("Auto"))
	if hw_modes.b then mode:value("11b", "802.11b") end
	if hw_modes.g then mode:value("11g", "802.11g") end
	if hw_modes.a then mode:value("11a", "802.11a") end

	if htcaps then
		if hw_modes.g and hw_modes.n then mode:value("11ng", "802.11g+n") end
		if hw_modes.a and hw_modes.n then mode:value("11na", "802.11a+n") end
	end
function mode.cfgvalue(self, section)
	local mval = nil
	mval = m.uci:get("wireless", wdev:name() , "hwmode")
	if mval == nil then
		mval = ""
	end
	return mval
end
function mode.write(self, section, value)
	if value == "auto" then
		m.uci:delete("wireless",  wdev:name() , "hwmode")
	else
		m.uci:set("wireless",  wdev:name() , "hwmode", value)
	end
	m.uci:save("wireless")
end


ch = s:option(ListValue, "channel", translate("Channel"), translate("Your wireless radio is forced to work in selected channel in order to maintain the connection"))
ch:value("auto", translate("Auto"))
for _, f in ipairs(iw and iw.freqlist or luci.sys.wifi.channels()) do
	if not f.restricted then
		ch:value(f.channel, "%i (%.3f GHz)" % { f.channel, f.mhz / 1000 })
	end
end
function ch.cfgvalue(self, section)
	cval = m.uci:get("wireless", wdev:name() , "channel")
	return cval
end
function ch.write(self, section, value)
	m.uci:set("wireless",  wdev:name() , "channel", value)
	m.uci:save("wireless")
end


ifsection = s
s.addremove = false
s.anonymous = true
s.defaults.device = wdev:name()
mode = "ap"
network = "lan"

------------------- WiFI-Encryption -------------------

encr = s:option(ListValue, "encryption", translate("Encryption"), translate("Specifies what method of encryption will be used for authorization verification "))
encr.override_values = true
encr.override_depends = true
--encr:depends({mode="ap"})
--encr:depends({mode="sta"})
--encr:depends({mode="adhoc"})
--encr:depends({mode="ahdemo"})
--encr:depends({mode="ap-wds"})
--encr:depends({mode="sta-wds"})
--encr:depends({mode="mesh"})

cipher = s:option(ListValue, "cipher", translate("Cipher"), translate("In cryptography, a cipher (or cypher) is an algorithm for performing encryption or decryption"))
cipher:depends({encryption="wpa"})
cipher:depends({encryption="wpa2"})
cipher:depends({encryption="psk"})
cipher:depends({encryption="psk2"})
cipher:depends({encryption="wpa-mixed"})
cipher:depends({encryption="psk-mixed"})
cipher:value("auto", translate("Auto"))
cipher:value("ccmp", translate("Force CCMP (AES)"))
cipher:value("tkip", translate("Force TKIP"))
cipher:value("tkip+ccmp", translate("Force TKIP and CCMP (AES)"))

function encr.cfgvalue(self, section)
	local v = tostring(ListValue.cfgvalue(self, section))
	if v and v:match("%+") then
		return (v:gsub("%+.+$", ""))
	end
	return v
end

function encr.write(self, section, value)
	local e = tostring(encr:formvalue(section))
	local c = tostring(cipher:formvalue(section))
	if value == "wpa" or value == "wpa2"  then
		self.map.uci:delete("wireless", section, "key")
	end
	if e and (c == "tkip" or c == "ccmp" or c == "tkip+ccmp") then
		e = e .. "+" .. c
	end
	self.map:set(section, "encryption", e)
end

function cipher.cfgvalue(self, section)
	local v = tostring(ListValue.cfgvalue(encr, section))
	if v and v:match("%+") then
		v = v:gsub("^[^%+]+%+", "")
		if v == "aes" then v = "ccmp"
		elseif v == "tkip+aes" then v = "tkip+ccmp"
		elseif v == "aes+tkip" then v = "tkip+ccmp"
		elseif v == "ccmp+tkip" then v = "tkip+ccmp"
		end
	end
	return v
end

function cipher.write(self, section)
	return encr:write(section)
end


encr:value("none", "No encryption")

if hwtype == "atheros" or hwtype == "mac80211" or hwtype == "prism2" then
	local supplicant = fs.access("/usr/sbin/wpa_supplicant")
	local hostapd = fs.access("/usr/sbin/hostapd")

	-- Probe EAP support
	local has_ap_eap  = (os.execute("hostapd -veap >/dev/null 2>/dev/null") == 0)
	local has_sta_eap = (os.execute("wpa_supplicant -veap >/dev/null 2>/dev/null") == 0)

	if hostapd and supplicant then
		encr:value("psk", "WPA-PSK")
		encr:value("psk2", "WPA2-PSK")
		encr:value("psk-mixed", "WPA-PSK/WPA2-PSK Mixed Mode")
		if has_ap_eap and has_sta_eap then
			encr:value("wpa", "WPA-EAP")
			encr:value("wpa2", "WPA2-EAP")
		end
	elseif hostapd and not supplicant then
		encr:value("psk", "WPA-PSK")
		encr:value("psk2", "WPA2-PSK")
		encr:value("psk-mixed", "WPA-PSK/WPA2-PSK Mixed Mode")
		if has_ap_eap then
			encr:value("wpa", "WPA-EAP")
			encr:value("wpa2", "WPA2-EAP")
		end
		encr.description = translate(
			"WPA-Encryption requires wpa_supplicant (for client mode) or hostapd (for AP " ..
			"and ad-hoc mode) to be installed."
		)
	elseif not hostapd and supplicant then
		encr:value("psk", "WPA-PSK", {mode="sta"}, {mode="sta-wds"})
		encr:value("psk2", "WPA2-PSK", {mode="sta"}, {mode="sta-wds"})
		encr:value("psk-mixed", "WPA-PSK/WPA2-PSK Mixed Mode", {mode="sta"}, {mode="sta-wds"})
		if has_sta_eap then
			encr:value("wpa", "WPA-EAP", {mode="sta"}, {mode="sta-wds"})
			encr:value("wpa2", "WPA2-EAP", {mode="sta"}, {mode="sta-wds"})
		end
		encr.description = translate(
			"WPA-Encryption requires wpa_supplicant (for client mode) or hostapd (for AP " ..
			"and ad-hoc mode) to be installed."
		)
	else
		encr.description = translate(
			"WPA-Encryption requires wpa_supplicant (for client mode) or hostapd (for AP " ..
			"and ad-hoc mode) to be installed."
		)
	end
elseif hwtype == "broadcom" then
	encr:value("psk", "WPA-PSK")
	encr:value("psk2", "WPA2-PSK")
	encr:value("psk+psk2", "WPA-PSK/WPA2-PSK Mixed Mode")
end

wpakey = s:option(Value, "_wpa_key", translate("Key"), translate("Specifies encrypted string. Minimum length - 8, maximum length - 64."))
wpakey:depends("encryption", "psk")
wpakey:depends("encryption", "psk2")
wpakey:depends("encryption", "psk+psk2")
wpakey:depends("encryption", "psk-mixed")
wpakey.datatype = "wpakey"
--  wpakey.rmempty = true
wpakey.default = "12345678"
wpakey.password = true

function wpakey.validate(self, value, section)
	if #value > 0 then
		return value
	else
		return nil, translate("The value is invalid because WPA key can not be empty.")
	end
end

wpakey.cfgvalue = function(self, section, value)
	local key = m.uci:get("wireless", section, "key")
	if key == "1" or key == "2" or key == "3" or key == "4" then
		return nil
	end
	return key
end

wpakey.write = function(self, section, value)
	self.map.uci:set("wireless", section, "key", value)
	self.map.uci:delete("wireless", section, "key1")
end

local cl = iw and iw.countrylist

if cl and #cl > 0 then
	cc = s:option(ListValue, "country", translate("Country Code"), translate("Use ISO/IEC 3166 alpha2 country codes."))
	cc.default = tostring(iw and iw.country or "00")
	for _, c in ipairs(cl) do
		cc:value(c.alpha2, "%s - %s" %{ c.alpha2, c.name })
	end
	function cc.cfgvalue(self, section)
		local ccval = nil
		ccval = m.uci:get("wireless", wdev:name() , "country")
		return ccval
	end
	function cc.write(self, section, value)
		m.uci:set("wireless",  wdev:name() , "country", value)
		m.uci:save("wireless")
	end
else
	s:option(Value, "country", translate("Country Code"), translate("Use ISO/IEC 3166 alpha2 country codes."))
end

if m:formvalue("cbi.wizard.skip") then
	luci.http.redirect(luci.dispatcher.build_url("/admin/status/overview"))
end
return m
