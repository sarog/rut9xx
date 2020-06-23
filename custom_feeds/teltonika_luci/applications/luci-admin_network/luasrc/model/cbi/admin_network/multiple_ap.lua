local m, sc, sc1, o


m = Map("multi_wifi", translate("Multiple Access Points configuration"), translate(""))
m.addremove = false

function is_checked()
	local checked = ""
	m.uci:foreach("multi_wifi", "wifi-iface", function(s)
		if s.enabled and s.enabled == "1" then
			checked = "checked"
		end
	end)

	return checked
end

sc = m:section(NamedSection, "general", "multi_wifi", translate("General settings"))

o = sc:option(Flag, "enabled", translate("Enable"), translate("Enable Multiple AP"))
o.rmempty = false

o = sc:option(Value, "scan_time", translate("Scan Time (sec)"), translate("Time between scans of available access points (min. 30sec.)"))
o.datatype = "min(30)"
o.rmempty = false

o = sc:option(Value, "block_time", translate("Blocking Time (min)"), translate("Time for blocking access points after retry counts exceed (min. 1 minute)"))
o.datatype = "min(1)"
o.rmempty = false

function o.cfgvalue(self, section)
	local value = m.uci:get("multi_wifi", section, "block_time")
	if value and tonumber(value) then
		value = value/60
	end
	return value
end

function o.write(self, section, value)
	if value and #value > 0 and tonumber(value) then
		local time = value*60
		return Value.write(self, section, time)
	else
		m.message = translate("err: Incorrect Blocking time value!")
		return nil
	end
end

FileUpload.size = "100000"
FileUpload.sizetext = translate("Selected file is too large, max size is 100 KB")
FileUpload.sizetextempty = translate("Selected file is empty")
file_to_upload = sc:option( FileUpload, "access_points", translate("AP list"), translate("Upload a file with many access points (Max file size allowed is 100KB). <br> " ..
		"The file\\'s input text format should be \"name: value\" in each line. <br>" ..
		"Every AP must have <strong>ssid</strong> option e.g. \"ssid: NEW_AP\" <br>" ..
		"Every new AP starts with <strong>ssid</strong> option. <br> " ..
		"Priorities will be set by what order they are written in the file. <br>" ..
		"Optional values are: <br>" ..
		"<strong>enable</strong> e.g. \"enable: 1\" (1 - for enabled, 0 - for disabled) <br>" ..
		"<strong>encryption</strong> e.g. \"encryption: psk2\" (available options: none, psk, psk2, psk-mixed) <br> " ..
		"<strong>cipher</strong> e.g. \"cipher: auto\" (available options: auto, ccmp, tkip, tkip+ccmp) <br> " ..
		"<strong>key</strong> e.g. \"key: NEW_AP_KEY\" <br> " ..
		"<strong>retry</strong> e.g. \"retry: 3\""))

sc1 = m:section(TypedSection, "wifi-iface")
sc1.addremove = true
sc1.anonymous = true
sc1.template = "cbi/tblsection"
sc1.sortable  = true
sc1.novaluetext = translate("There are no access points added yet")

o = sc1:option(Flag, "enabled", translate("Enable"), translate("Check to enable this AP"))
o.rmempty = false
o.default = "1"
o.width = "10%"
o.useCustomHTML = true
o.customHTML = "<div id=\"select_all_sms\"><input " .. is_checked() .. " type='checkbox' onclick=\"select_all(this, 'cbi-input-checkbox');\"></div>"

o = sc1:option(Value, "ssid", translate("SSID"), translate("SSID of access point"))
o.datatype = "string_not_empty"

encr = sc1:option(ListValue, "encryption", translate("Encryption"), translate("Method of encryption that will be used for users authorization"))
encr:value("none", translate("No encryption"))

local supplicant = nixio.fs.access("/usr/sbin/wpa_supplicant")
local hostapd = nixio.fs.access("/usr/sbin/hostapd")
if hostapd and supplicant then
	encr:value("psk", translate("WPA-PSK"))
	encr:value("psk2", translate("WPA2-PSK"))
	encr:value("psk-mixed", translate("WPA-PSK/WPA2-PSK"))
end

cipher = sc1:option(ListValue, "cipher", translate("Cipher"), translate("In cryptography, a cipher (or cypher) is an algorithm for performing encryption or decryption"))
cipher:depends({encryption="psk"})
cipher:depends({encryption="psk2"})
cipher:depends({encryption="wpa-mixed"})
cipher:depends({encryption="psk-mixed"})
cipher:value("auto", translate("Auto"))
cipher:value("ccmp", translate("Force CCMP"))
cipher:value("tkip", translate("Force TKIP"))
cipher:value("tkip+ccmp", translate("Force TKIP and CCMP"))

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

wpakey = sc1:option(Value, "key", translate("Key"))
wpakey.template = "admin_network/password_value"
wpakey:depends("encryption", "psk")
wpakey:depends("encryption", "psk2")
wpakey:depends("encryption", "psk-mixed")
wpakey.datatype = "wpakey"
wpakey.password = true
wpakey.noautocomplete = true

o = sc1:option(ListValue, "retry", translate("Retry"), translate("Retry count if connection fails"))
o:value("1")
o:value("2")
o:value("3")
o:value("4")
o:value("5")
o:value("6")
o:value("7")
o:value("8")
o:value("9")
o:value("10")

function m.on_before_apply(self)
	local number = 1

	m.uci:foreach("multi_wifi", "wifi-iface", function(s)
		m.uci:set("multi_wifi", s[".name"], "priority", number)
		number = number + 1
	end)

	m.uci:commit("multi_wifi")
end

function m.on_after_all(self)
	local new_cfg
	local number = 0
	local uploaded = m.uci:get("multi_wifi", "general", "access_points") or ""
	if uploaded and uploaded ~= "" and nixio.fs.access(uploaded) then

		m.uci:foreach("multi_wifi", "wifi-iface", function(s)
			if tonumber(s.priority) and tonumber(s.priority) > number then
				number = tonumber(s.priority)
			end
		end)


		local f = io.open(uploaded, "r")
		local list_ap = f:read("*all")
		f:close()
		list_ap = list_ap:gsub("\r\n", "\n")
		for name, value in list_ap:gmatch("([%w%.%-%+_]+)%:%s+(.-)\n") do
			if name then
				if name == "ssid" then
					new_cfg = m.uci:add("multi_wifi", "wifi-iface")
					m.uci:set("multi_wifi", new_cfg, "ssid", value)
				elseif name == "enable" and new_cfg then
					m.uci:set("multi_wifi", new_cfg, "enabled", value)
				elseif name == "cipher" and new_cfg then
					m.uci:set("multi_wifi", new_cfg, "cipher", value)
				elseif name == "encryption" and new_cfg then
					m.uci:set("multi_wifi", new_cfg, "encryption", value)
				elseif name == "key" and new_cfg then
					m.uci:set("multi_wifi", new_cfg, "key", value)
				elseif name == "retry" and new_cfg then
					m.uci:set("multi_wifi", new_cfg, "retry", value)
				end
			end
		end

		m.uci:foreach("multi_wifi", "wifi-iface", function(s)
			if s.encryption and s.cipher and s.cipher ~= "auto" then
				m.uci:set("multi_wifi", s[".name"], "encryption", s.encryption.."+"..s.cipher)
			end
			if s.cipher then
				m.uci:delete("multi_wifi", s[".name"], "cipher")
			end
			if not s.encryption then
				m.uci:set("multi_wifi", s[".name"], "encryption", "none")
			end
			if not s.enabled then
				m.uci:set("multi_wifi", s[".name"], "enabled", "0")
			end
			if not s.retry then
				m.uci:set("multi_wifi", s[".name"], "retry", "3")
			end
			if not s.priority then
				m.uci:set("multi_wifi", s[".name"], "priority", tostring(number+1))
				number = number + 1
			end
		end)
		m.uci:delete("multi_wifi", "general", "access_points")
		nixio.fs.remove(uploaded)
		m.uci:commit("multi_wifi")
	end
end

return m
