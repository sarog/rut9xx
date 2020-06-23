-- FIXME: pakeitimus butina atlikti ir default config faile custom_feeds/teltonika_luci/modules/admin-full/root/etc/config/wimax_gct
local uci  = require "luci.model.uci".cursor()

YES_FLAG = "y"
NO_FLAG = "n"

local m, s, d, d

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

m = Map("wimax_gct", translate("WiMAX Configuration"), translate("Here you can configure your WiMAX settings."))
m.addremove = false

s = m:section(NamedSection, "wimax_gct", "wimax", translate("WiMAX settings"));
s.addremove = false

o = s:option(Flag, "auth_pkm_enable", translate("PKM Enable"))
o.rmempty = false
o.enabled = YES_FLAG
o.disabled = NO_FLAG

o = s:option(ListValue, "eap_type", translate("EAP Type"))
o:value("TLS", translate("TLS"))
o:value("TTLS-CHAP", translate("TTLS-CHAP"))
o:value("TTLS-MSCHAPV2", translate("TTLS-MSCHAPV2"))

o = s:option(Value, "eap_tls_userid", translate("User ID"))
o:depends("eap_type", "TTLS-CHAP")
o:depends("eap_type", "TTLS-MSCHAPV2")

o = s:option(Value, "eap_tls_userpasswd", translate("User password"))
o.password = true
o:depends("eap_type", "TTLS-CHAP")
o:depends("eap_type", "TTLS-MSCHAPV2")

o = s:option(Value, "eap_tls_anonyid", translate("Anonymous ID"))

o = s:option(Value, "eap_tls_pri_passwd", translate("Private password"))
o.password = true

o = s:option(Value, "eap_tls_fragsize", translate("Length of fragment"))

o = s:option(Flag, "eap_tls_sessionticket_disable", translate("Disable EAP TLS session ticket"))
o.rmempty = false
o.enabled = YES_FLAG
o.disabled = NO_FLAG

o = s:option(Flag, "eap_tls_dev_cert_null", translate("Device certificate NULL"))
o.rmempty = false
o.enabled = YES_FLAG
o.disabled = NO_FLAG

o = s:option(Flag, "eap_tls_ca_cert_null", translate("CA certificate NULL"))
o.rmempty = false
o.enabled = YES_FLAG
o.disabled = NO_FLAG

o = s:option(Flag, "eap_tls_delimiter_enable", translate("Use @ delimiter"))
o.rmempty = false
o.enabled = YES_FLAG
o.disabled = NO_FLAG

	-------------------------
	-- Default values
	-- FIXME: pakeitimus butina atlikti ir default config faile custom_feeds/teltonika_luci/modules/admin-full/root/etc/config/wimax_gct
	-------------------------
function o.write(self, section, value)
	local Size = m:formvalue("cbid.wimax_gct.wimax_gct.eap_tls_fragsize")
	if Size == "" or Size == nil then
		m.message = translate("err: Length of fragment is incorrect!")
		return nil
	end
	if not Size:match("^%d+") then
		m.message = translate("err: Length of fragment is incorrect!")
		return nil
	else
		m.message = translate("scs: Settings will take effect after router reboot.")
	end
	
	Flag.write(self, section, value)
	-- [common]
	m.uci:set("wimax_gct", "wimax_gct", "log_path", 						"./sdklog")
	m.uci:set("wimax_gct", "wimax_gct", "log_level", 						"1")
	m.uci:set("wimax_gct", "wimax_gct", "eap_log_enable", 					YES_FLAG)
	m.uci:set("wimax_gct", "wimax_gct", "embedded_eap_enable", 				YES_FLAG)
	m.uci:set("wimax_gct", "wimax_gct", "oma_dm_enable", 					YES_FLAG)
	m.uci:set("wimax_gct", "wimax_gct", "nonvolatile_dir", 					"./storage/")
	m.uci:set("wimax_gct", "wimax_gct", "run_script_file", 					" ")
	m.uci:set("wimax_gct", "wimax_gct", "auto_connect_enable", 				YES_FLAG)
	m.uci:set("wimax_gct", "wimax_gct", "auto_connect_retry_count", 		"10")
	m.uci:set("wimax_gct", "wimax_gct", "auto_select_profile_index", 		"0")
	m.uci:set("wimax_gct", "wimax_gct", "unknown_net_auto_connect_enable", 	NO_FLAG)
	m.uci:set("wimax_gct", "wimax_gct", "ip_allocation_timeout_sec", 		"30")
	m.uci:set("wimax_gct", "wimax_gct", "disconnct_on_ip_failure", 			YES_FLAG)
	-- [device-default]
	m.uci:set("wimax_gct", "wimax_gct", "eap_tls_use_nvram_info", 			NO_FLAG)
	m.uci:set("wimax_gct", "wimax_gct", "eap_tls_use_nvram_cert", 			YES_FLAG)
	
	m.uci:save("wimax_gct")
	m.uci:commit("wimax_gct")
end

d = m:section(NamedSection, "wimax_cert", "wimax", translate("WiMAX certificates"));
d.addremove = false

o = d:option(FileUpload, "xml", translate("OMA-XML"))
o = d:option(FileUpload, "dcert", translate("Device Cert"))
o = d:option(FileUpload, "sroot", translate("Server Root CA"))
o = d:option(FileUpload, "subca1", translate("Sub CA #1"))
o = d:option(FileUpload, "subca2", translate("Sub CA #2"))
o = d:option(FileUpload, "combca", translate("Combined CA"))
--[[
dl = d:option(Button, "_download_certs")
dl.title = translate("Backup certificates")
dl.inputtitle = translate("Download")
dl.inputstyle = "apply"

if m:formvalue("cbid.wimax_gct.wimax_cert._download_certs") then
	local backup_cmd  = "cat /tmp/mano2 - 2>/dev/null"
	local reader = ltn12_popen(backup_cmd)
	luci.http.header('Content-Disposition', 'attachment; filename="backup-%s-%s.tar.gz"' % {
		luci.sys.hostname(), os.date("%Y-%m-%d")})
	luci.http.prepare_content("application/x-targz")
	luci.ltn12.pump.all(reader, luci.http.write)
end]]

return m
