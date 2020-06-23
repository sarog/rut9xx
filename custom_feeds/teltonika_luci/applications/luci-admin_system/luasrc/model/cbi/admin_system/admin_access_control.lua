--[[
Teltonika R&D. ver 0.1
]]--


local fs = require "nixio.fs"
local fw = require "luci.model.firewall"
local http = require "luci.http"
require("luci.fs")
require("luci.config")
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()

local logDir, o, needReboot = false
local deathTrap = { }
local pbridge = false

function symbol_escape(line)
	symbols = {"`","&","!","\"","%$","\\","'","?",";","*",">"}
	if line then
		line = line.gsub(line, "\\", "")
		for a = 1, #symbols, 1 do
			line = line.gsub(line, symbols[a], "\\" .. symbols[a])
		end
	end
	return line
end

local ipaddr = symbol_escape(http.getenv("REMOTE_ADDR"))
local port = http.getenv('SERVER_PORT') or ""
local https_port = luci.util.trim(luci.sys.exec("uci -q get uhttpd.main.listen_https | awk -F ':' '{print $2}'")) or "443"
local ipaddr_eth = luci.util.trim(luci.sys.exec(". /lib/teltonika-functions.sh; check_this_ip " .. ipaddr)) or ""
if port == https_port then
	port = "HTTPS"
else
	port = "HTTP"
end

met=utl.trim(luci.sys.exec("uci get -q network.ppp.method")) or ""
if met == "pbridge" then
	ppp_enable=utl.trim(luci.sys.exec("uci get -q network.ppp.enabled")) or "1"
	if ppp_enable ~= "0" then
		pbridge = true
	end
end

if fs.access("/etc/config/dropbear") then
-- m = Map("system", translate("Administration properties"),
-- 	translate("Changes the administration password, log level and provides SSH access control."))

	m2 = Map("dropbear", translate("Access Control"), translate(""))
	m2:chain("firewall")
	fw.init(m2.uci)

	s = m2:section(TypedSection, "dropbear", translate("SSH"))
	s.anonymous = true
	s.addremove = false

	dummy = s:option(DummyValue, "dummy_value_one", translate(""))
	dummy.default = translate("Enabling remote SSH access makes your device reachable from WAN, this might pose a security risk, especially if you are using a weak or default user password!")

	p = s:option(Flag, "enable", translate("Enable SSH access"), translate("Check box to enable SSH access functionality"))
	p.rmempty = false
	p.enabled = "1"
	p.disabled = "0"

	o = s:option(Flag, "_sshWanAccess", translate("Remote SSH access"), translate("If check box is selected user can access the router via SSH from the outside (WAN)"))
	o.rmempty = false
	o.enabled = "1"
	o.disabled = "0"


	function o.write(self, section)
		local fval = self:formvalue(section)
		local fvalPort = pt:formvalue(section)
		local dropBearInstName

		-- stop this function being called twice
		if not deathTrap[1] then deathTrap[1] = true
		else return end

		-- fix some firewall rules incompatibility issues
		if not fval then
			fval = "0"
		else
			fval = ""
		end

		local fwRedirect = "nil"
		local fwRuleInstName = "nil"
		local needsPortUpdate = false
		m2.uci:foreach("firewall", "rule", function(s)
			if s.name == "Enable_SSH_WAN" then
				fwRuleInstName = s[".name"]
				if s.dest_port ~= fvalPort then
					needsPortUpdate = true
				end
				if s.enabled ~= fval then
					needsPortUpdate = true
				end
			end
		end)

		m2.uci:foreach("firewall", "redirect", function(s)
			if s.name == "Enable_SSH_WAN_PASSTHROUGH" then
				fwRedirect = s[".name"]
			end
		end)
		if fwRedirect ~= "nil" then
			if pbridge then
				m2.uci:set("firewall", "E_SSH_W_P", "enabled", fval)
			end
			m2.uci:set("firewall", "E_SSH_W_P", "src_dport", fvalPort)
		else
			local options = {
				target 		= "DNAT",
				src			= "wan",
				dest		= "lan",
				proto 		= "tcp",
				name 		= "Enable_SSH_WAN_PASSTHROUGH",
				dest_ip		= "127.0.0.1",
				reflection	= "0",
				src_dport 	= fvalPort,
-- 				enabled		= fval
				enabled		= "0"
			}
			uci:section("firewall", "redirect","E_SSH_W_P",options)
			uci:save("firewall")
		end

		if needsPortUpdate == true then
			m2.uci:set("firewall", fwRuleInstName, "dest_port", fvalPort)
			m2.uci:set("firewall", fwRuleInstName, "enabled", fval)
			m2.uci:save("firewall")
		end

		if fwRuleInstName == "nil" then
			local wanZone = fw:get_zone("wan")
			if not wanZone then
				m.message = translate("err: Error: could not add firewall rule!")
				return
			end
			local options = {
				target 		= "ACCEPT",
				proto 		= "tcp udp",
				dest_port 	= fvalPort,
				name 		= "Enable_SSH_WAN",
				enabled		= fval
			}
			wanZone:add_rule(options)
			m2.uci:save("firewall")
		end
		--m2.uci:apply("firewall")
		--m2.uci.commit("firewall")
	end

	pt = s:option(Value, "Port", translate("Port"),
	translate("Port to listen for SSH access."))
	pt.datatype = "port"
	pt.default = 22

	function o.cfgvalue(self, section)
		local fwRuleEn = false

		m2.uci:foreach("firewall", "rule", function(s)
			if s.name == "Enable_SSH_WAN" and s.enabled ~= "0" then
				fwRuleEn = true
			end
		end)

		if fwRuleEn then
			return self.enabled
		else
			return self.disabled
		end
	end
end

if fs.access("/etc/config/uhttpd") then
	m3 = Map("uhttpd", "", "")
	m3:chain("firewall")
	fw.init(m3.uci)

	s = m3:section(NamedSection, "main", "uhttpd", translate("WebUI"))

	dummy = s:option(DummyValue, "dummy_value_one", translate(""))
	dummy.default = translate("Enabling remote HTTP access or remote HTTPS access makes your device reachable from WAN, this might pose a security risk, especially if you are using a weak or default user password! Also, if you want to use HTTPS to reach the router, you should first download the uhttpd certificate and add it to your trusted CA list.")

	enb = s:option(Flag, "enablehttp", translate("Enable HTTP access"), translate("Check box to enable HTTP access functionality"))
	enb.rmempty = false
	enb.confirm = {"0", "If this option will be disabled then you will not be to able to reach WebUI, are you sure?" }
	enb.default = "1"

	enb = s:option(Flag, "enablehttps", translate("Enable HTTPS access"), translate("Check box to enable HTTPS access functionality"))
	enb.rmempty = false
	enb.default = "1"

	redirect_https = s:option(Flag, "redirect_https", translate("Redirect to HTTPS"), translate("Force HTTPS by redirecting HTTP to HTTPS"))
	redirect_https.rmempty = false

	o = s:option(Flag, "_httpWanAccess", translate("Enable remote HTTP access"), translate("If check box is selected user can acces the router via the HTTP WEB interface from outside (WAN)"))
-- 	o:visdeps("enablehttp" , "1")
	o.rmempty = false
	if ipaddr_eth == "WAN" and port == "HTTP" then
		o.confirm = {"0", "If this option will be disabled then you will not be to able to reach WebUI, are you sure?" }
	end


	prt = s:option(Value, "listen_http", translate("Port"), translate("Specify a port number for routers web management via HTTP protocol"))
	prt.datatype = "port"
	prt.rmempty = false
-- 	prt:visdeps("enablehttp" , "1")

	function prt.cfgvalue(self, section)
		local cport = AbstractValue.cfgvalue(self, section)
		if cport then
			return cport:gsub("(%d+.%d+.%d+.%d+:)","")
		end
	end

	function prt.write(self, section, value)
		m3.uci:set("firewall","service_HTTP","dest_port", value)
		m3.uci:save("firewall")
		AbstractValue.write(self, section, "0.0.0.0:"..value)
	end

	function o.write(self, section)
		local fval = self:formvalue(section)
		local fvalPort = prt:formvalue(section)
		local enbHttp = enb:formvalue(section)
		local dropBearInstName
		if fval then
			m3.uci:set("uhttpd", "main", "_httpWanAccess", fval)
		else
			m3.uci:set("uhttpd", "main", "_httpWanAccess", "0")
		end



		-- stop this function being called twice
		if not deathTrap[2] then deathTrap[2] = true
		else return end

		-- fix some firewall rules incompatibility issues
		if not fval or not enbHttp then
			fval = "0"
		else
			fval = ""
		end

		local fwRedirect = "nil"
		local fwRuleInstName = "nil"
		local needsPortUpdate = false
		m3.uci:foreach("firewall", "rule", function(s)
			if s.name == "Enable_HTTP_WAN" then
				fwRuleInstName = s[".name"]
				if s.dest_port ~= fvalPort then
					needsPortUpdate = true
				end
				if s.enabled ~= fval then
					needsPortUpdate = true
				end
			end
		end)

		m3.uci:foreach("firewall", "redirect", function(s)
			if s.name == "Enable_HTTP_WAN_PASSTHROUGH" then
				fwRedirect = s[".name"]
			end
		end)
		if fwRedirect ~= "nil" then
			if pbridge then
				m3.uci:set("firewall", "E_HTTP_W_P", "enabled", fval)
			end
			m3.uci:set("firewall", "E_HTTP_W_P", "src_dport", fvalPort)
		else
			local options = {
				target 		= "DNAT",
				src			= "wan",
				dest		= "lan",
				proto 		= "tcp",
				name 		= "Enable_HTTP_WAN_PASSTHROUGH",
				dest_ip		= "127.0.0.1",
				reflection	= "0",
				src_dport 	= fvalPort,
-- 				enabled		= fval
				enabled		= "0"
			}
			uci:section("firewall", "redirect","E_HTTP_W_P",options)
			uci:save("firewall")
		end

		if needsPortUpdate == true then
			m3.uci:set("firewall", fwRuleInstName, "dest_port", fvalPort)
			m3.uci:set("firewall", fwRuleInstName, "enabled", fval)
			m3.uci:save("firewall")
		end

		if fwRuleInstName == "nil" then
			local wanZone = fw:get_zone("wan")
			if not wanZone then
				m.message = translate("err: Error: could not add firewall rule!")
				return
			end
			local options = {
				target 		= "ACCEPT",
				proto 		= "tcp udp",
				dest_port 	= fvalPort,
				name 		= "Enable_HTTP_WAN",
				enabled		= fval
			}
			wanZone:add_rule(options)
			m3.uci:save("firewall")
		end
		--m3.uci:apply("firewall")
		--m3.uci.commit("firewall")
	end

-- 	function o.cfgvalue(self, section)
-- 		local fwRuleEn = false
--
-- 		m3.uci:foreach("firewall", "rule", function(s)
-- 			if s.name == "Enable_HTTP_WAN" and s.enabled ~= "0" then
-- 				fwRuleEn = true
-- 			end
-- 		end)
--
-- 		if fwRuleEn then
-- 			return self.enabled
-- 		else
-- 			return self.disabled
-- 		end
-- 	end

	o = s:option(Flag, "_httpsWanAccess", translate("Enable remote HTTPS access"), translate("If check box is selected users can access the router via the HTTPS WEB interace from the outside (WAN)"))
	o.rmempty = false
	if ipaddr_eth == "WAN" and port == "HTTPS" then
		o.confirm = {"0", "If this option will be disabled then you will not be to able to reach WebUI, are you sure?" }
	end

	prt_https = s:option(Value, "listen_https", translate("Port"), translate("Specify a port number for routers web management via HTTPS protocol"))
	prt_https.datatype = "port"
	prt_https.rmempty = false


	function prt_https.cfgvalue(self, section)
		local cport = AbstractValue.cfgvalue(self, section)
		if cport then
			return cport:gsub("(%d+.%d+.%d+.%d+:)","")
		end
	end

	function prt_https.write(self, section, value)
		m3.uci:set("firewall","service_HTTPS","dest_port", value)
		m3.uci:save("firewall")
		AbstractValue.write(self, section, "0.0.0.0:"..value)
	end

	function o.write(self, section)
		local fval = self:formvalue(section)
		local fvalPort = prt_https:formvalue(section)
		local dropBearInstName

		if fval then
			m3.uci:set("uhttpd", "main", "_httpsWanAccess", fval)
		else
			m3.uci:set("uhttpd", "main", "_httpsWanAccess", "0")
		end

		-- stop this function being called twice
		if not deathTrap[3] then deathTrap[3] = true
		else return end

		-- fix some firewall rules incompatibility issues
		if not fval then
			fval = "0"
		else
			fval = ""
		end

		local fwRedirect = "nil"
		local fwRuleInstName = "nil"
		local needsPortUpdate = false
		m3.uci:foreach("firewall", "rule", function(s)
			if s.name == "Enable_HTTPS_WAN" then
				fwRuleInstName = s[".name"]
				if s.dest_port ~= fvalPort then
					needsPortUpdate = true
				end
				if s.enabled ~= fval then
					needsPortUpdate = true
				end
			end
		end)

		m3.uci:foreach("firewall", "redirect", function(s)
			if s.name == "Enable_HTTP_WAN_PASSTHROUGH" then
				fwRedirect = s[".name"]
			end
		end)
		if fwRedirect ~= "nil" then
			if pbridge then
				m3.uci:set("firewall", "E_HTTPS_W_P", "enabled", fval)
			end
			m3.uci:set("firewall", "E_HTTPS_W_P", "src_dport", fvalPort)
		else
			local options = {
				target 		= "DNAT",
				src			= "wan",
				dest		= "lan",
				proto 		= "tcp",
				name 		= "Enable_HTTPS_WAN_PASSTHROUGH",
				dest_ip		= "127.0.0.1",
				reflection	= "0",
				src_dport 	= fvalPort,
-- 				enabled		= fval
				enabled		= "0"
			}
			uci:section("firewall", "redirect","E_HTTPS_W_P",options)
			uci:save("firewall")
		end

		if needsPortUpdate == true then
			m3.uci:set("firewall", fwRuleInstName, "dest_port", fvalPort)
			m3.uci:set("firewall", fwRuleInstName, "enabled", fval)
			m3.uci:save("firewall")
		end

		if fwRuleInstName == "nil" then
			local wanZone = fw:get_zone("wan")
			if not wanZone then
				m.message = translate("err: Error: could not add firewall rule!")
				return
			end
			local options = {
				target 		= "ACCEPT",
				proto 		= "tcp udp",
				dest_port 	= fvalPort,
				name 		= "Enable_HTTPS_WAN",
				enabled		= fval
			}
			wanZone:add_rule(options)
			m3.uci:save("firewall")
		end
		--m3.uci:apply("firewall")
		--m3.uci.commit("firewall")
	end

	function o.cfgvalue(self, section)
		local fwRuleEn = false

		m3.uci:foreach("firewall", "rule", function(s)
			if s.name == "Enable_HTTPS_WAN" and s.enabled ~= "0" then
				fwRuleEn = true
			end
		end)

		if fwRuleEn then
			return self.enabled
		else
			return self.disabled
		end
	end

	local uhttpd_crt = s:option(Button, "_download_uhttpd_crt")
	uhttpd_crt.title      = translate("uhttpd certificate")
	uhttpd_crt.inputtitle = translate("Download")
	uhttpd_crt.inputstyle = "apply"
	uhttpd_crt.timeload = true
	uhttpd_crt.onclick = true
	uhttpd_crt.template = "admin_system/dwbutton"

	if m3:formvalue("cbid.uhttpd.main._download_uhttpd_crt") then
		luci.http.redirect(luci.dispatcher.build_url("admin/system/uhttpdcrt"))
	end

end

	json = s:option(Flag, "ubus_prefix", translate("Enable JSON RPC"), translate("If check box is selected users can access to ubus over HTTP"))
	json.enabled = "/ubus"
	json.rmempty = true

	rfc = s:option(Flag, "rfc1918_filter", translate("RFC1918 Filter"), translate("Address allocation for private internets filter"))
	rfc.rmempty = false

m4 = Map("cli", "", translate(""))
m4:chain("firewall")
fw.init(m4.uci)
local ut = require "luci.util"
local sys = require "luci.sys"
sc = m4:section(NamedSection, "status","status", translate("CLI"))

dummy = sc:option(DummyValue, "dummy_value_one", translate(""))
dummy.default = translate("Enabling remote CLI access makes your device reachable from WAN, this might pose a security risk, especially if you are using a weak or default user password!")

cli = sc:option(Flag, "enable", translate("Enable CLI"), translate(""))
rm_cli = sc:option(Flag, "_cliWanAccess", translate("Enable remote CLI"), translate(""))
rm_cli.rmempty = false
rm_cli.enabled = "1"
rm_cli.disabled = "0"


function rm_cli.cfgvalue(self, section)
	local fwRuleEn = false

	m4.uci:foreach("firewall", "rule", function(s)
		if s.name == "Enable_CLI_WAN" and s.enabled ~= "0" then
			fwRuleEn = true
		end
	end)

	if fwRuleEn then
		return self.enabled
	else
		return self.disabled
	end
end

function rm_cli.write(self, section)
	local rm_cli_val = self:formvalue(section)
	local cli_port = pt_cli:formvalue(section)
	-- fix some firewall rules incompatibility issues
	if not rm_cli_val then
		rm_cli_val = "0"
	else
		rm_cli_val = ""
	end

	local fwRedirect = "nil"
	local fwRuleInstName = "nil"
	local needsPortUpdate = false
	m4.uci:foreach("firewall", "rule", function(s)
		if s.name == "Enable_CLI_WAN" then
			fwRuleInstName = s[".name"]
			if s.dest_port ~= cli_port then
				needsPortUpdate = true
			end
			if s.enabled ~= rm_cli_val then
				needsPortUpdate = true
			end
		end
	end)

	m4.uci:foreach("firewall", "redirect", function(s)
		if s.name == "Enable_CLI_WAN_PASSTHROUGH" then
			fwRedirect = s[".name"]
		end
	end)
	if fwRedirect ~= "nil" then
		if pbridge then
			m4.uci:set("firewall", "E_CLI_W_P", "enabled", rm_cli_val)
		end
		m4.uci:set("firewall", "E_CLI_W_P", "src_dport", cli_port)
	else
		local options = {
			target 		= "DNAT",
			src		= "wan",
			dest		= "lan",
			proto 		= "tcp",
			name 		= "Enable_CLI_WAN_PASSTHROUGH",
			dest_ip		= "127.0.0.1",
			reflection	= "0",
			src_dport 	= cli_port,
-- 			enabled		= rm_cli_val
			enabled		= "0"
		}
		uci:section("firewall", "redirect","E_CLI_W_P",options)
		uci:save("firewall")
	end

	if needsPortUpdate == true then
		m4.uci:set("firewall", fwRuleInstName, "dest_port", cli_port)
		m4.uci:set("firewall", fwRuleInstName, "enabled", rm_cli_val)
		m4.uci:save("firewall")
	end

end



pt_cli = sc:option(Value, "port", translate("Port Range"), translate("Port range to listen for cli access."))
pt_cli.datatype = "portrange"
pt_cli.rmempty = false

limit_cli = sc:option(Value, "shell_limit", translate("Shell limit"), translate("Maximum number of active CLI shell instances."))
limit_cli.datatype = "range(1,10)"
limit_cli.default = "5"
limit_cli.rmempty = false

function m3.on_after_commit(self)

	local http_enabled = luci.http.formvalue("cbid.uhttpd.main._httpWanAccess")
	if http_enabled then
		http_enabled = ut.trim(http_enabled)
	else
		http_enabled = "0"
	end

	local https_enabled = luci.http.formvalue("cbid.uhttpd.main._httpsWanAccess")
	if https_enabled then
		https_enabled = ut.trim(https_enabled)
	else
		https_enabled = "0"
	end

	local dmz_enabled = sys.exec("uci get firewall.DMZ.enabled")
	if dmz_enabled then
		dmz_enabled = ut.trim(dmz_enabled)
	else
		dmz_enabled = "0"
	end

	local http_port = luci.http.formvalue("cbid.uhttpd.main.listen_http")
	if http_port then
		http_port = ut.trim(http_port)
	else
		http_port = "80"
	end

	local https_port = luci.http.formvalue("cbid.uhttpd.main.listen_https")
	if https_port then
		https_port = ut.trim(https_port)
	else
		https_port = "443"
	end

	local lan_ip = sys.exec("uci get network.lan.ipaddr")
	if lan_ip then
		lan_ip = ut.trim(lan_ip)
	end

	m3.uci:foreach("firewall", "redirect", function(s)
		if s["name"] == "tlt_allow_remote_http_through_DMZ" or s["name"] == "tlt_allow_remote_https_through_DMZ" then
			sys.exec("uci delete firewall.".. s[".name"])
		end
	end)

	if dmz_enabled == "1" and http_enabled == "1" then
		local config_value = ut.trim(sys.exec("uci add firewall redirect"))

		sys.exec("uci set firewall." ..config_value.. ".target='DNAT'")
		sys.exec("uci set firewall." ..config_value.. ".src='wan'")
		sys.exec("uci set firewall." ..config_value.. ".dest='lan'")
		sys.exec("uci set firewall." ..config_value.. ".proto='tcp'")
		sys.exec("uci set firewall." ..config_value.. ".src_dport='" ..http_port.. "'")
		sys.exec("uci set firewall." ..config_value.. ".dest_ip='".. lan_ip .."'")
		sys.exec("uci set firewall." ..config_value.. ".dest_port='"..http_port.."'")
		sys.exec("uci set firewall." ..config_value.. ".name='tlt_allow_remote_http_through_DMZ'")
		sys.exec("uci set firewall." ..config_value.. ".enabled='1'")
	end

	if dmz_enabled == "1" and https_enabled == "1" then
		local config_value = ut.trim(sys.exec("uci add firewall redirect"))

		sys.exec("uci set firewall." ..config_value.. ".target='DNAT'")
		sys.exec("uci set firewall." ..config_value.. ".src='wan'")
		sys.exec("uci set firewall." ..config_value.. ".dest='lan'")
		sys.exec("uci set firewall." ..config_value.. ".proto='tcp'")
		sys.exec("uci set firewall." ..config_value.. ".src_dport='" ..https_port.. "'")
		sys.exec("uci set firewall." ..config_value.. ".dest_ip='".. lan_ip .."'")
		sys.exec("uci set firewall." ..config_value.. ".dest_port='"..https_port.."'")
		sys.exec("uci set firewall." ..config_value.. ".name='tlt_allow_remote_https_through_DMZ'")
		sys.exec("uci set firewall." ..config_value.. ".enabled='1'")
	end

	sys.exec("uci commit firewall")

end

return m2, m3, m4
