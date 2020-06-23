--[[ NOTE: kadangi idetas TAP pasirinkimas, tai atsirado tokie pakeitimai:
+ tap atsiranda tik esant tls
+ jei serveris:
	+ 'dev' = 'tap'
	+ 'server' keiciasi i 'server_bridge' = 'nogw'
	+ atsiranda checkbox 'duplicate_cn'
	+ dingsta IP ir netmask
+ jei clientas
	+ 'dev' = 'tap'
]]
local sys = require("luci.sys")
local ipc = require("luci.ip")
local uci = require("luci.model.uci").cursor()
local utl = require "luci.util"
local dsp = require "luci.dispatcher"
local nixio = require "nixio"
-- local mix = require("luci.mtask")


local VPN_INST, TMODE
local VPN_INST_UNHEXED

local function cecho(string)
	sys.call("echo \"vpn: " .. string .. "\" >> /tmp/log.log")
end

function hex_to_string(str)
    return (str:gsub('..', function (cc)
        return string.char(tonumber(cc, 16))
    end))
end

if arg[1] then
	VPN_INST = arg[1]
	name_is_hexed = uci:get("openvpn", VPN_INST, "name_is_hexed") or "0"
	if name_is_hexed == "1" then
		VPN_INST_UNHEXED = hex_to_string(VPN_INST)
	else
		VPN_INST_UNHEXED = VPN_INST
	end

else
	--print("[Openvpn.cbasic] Fatal Err: Pass openvpn instance failed")
	--Shoud redirect back to overview
	return nil
end

local mode, o

function  split_by_word(config, section, option, order)
	local uci_l = require "luci.model.uci".cursor()
	local values={}
	if config and section and option and order then
		local val = uci_l:get(config, section, option)
		if val then
			for v in val:gmatch("[%w+%.]+") do
				table.insert(values, v)
			end
		end
	end

	return values[order]
end

function split(pString, pPattern)
	local Table = {}
	local fpat = "(.-)" .. pPattern
	local last_end = 1
	local s, e, cap = pString:find(fpat, 1)
	while s do
		if s ~= 1 or cap ~= "" then
			table.insert(Table,cap)
		end
		last_end = e+1
		s, e, cap = pString:find(fpat, last_end)
	end
	if last_end <= #pString then
		cap = pString:sub(last_end)
		table.insert(Table, cap)
	end
	return Table
end

local m = Map("openvpn", translatef("OpenVPN Instance: %s", VPN_INST_UNHEXED:gsub("^%l", string.upper)), "")
m.redirect = dsp.build_url("admin/services/vpn/openvpn-tlt/")

if VPN_INST then
	TMODE = VPN_INST_UNHEXED:match("%l+")
	TNAME = VPN_INST_UNHEXED:match("_.*")

	if TMODE == "client" then
		m.spec_dir = "/etc/openvpn/"
	elseif TMODE == "server" then
		m.spec_dir = "/etc/easy-rsa/keys/"
	else
		--print("[Openvpn.cbasic] Fatal Err: get mode failed")
		return
	end
end


local s = m:section( NamedSection, VPN_INST, "openvpn", translate("Main Settings"), "")

--[[#########################Same settings for server/client ##########################################]]--

local enable_custom = s:option(Flag, "enable_custom", translate("Enable OpenVPN config from file"),
	translate("Enable custom OpenVPN configuration from file"))

o = s:option( Flag, "enable", translate("Enable"), translate("Enable current configuration"))
	o.forcewrite = true
	o.rmempty = false

local upload_files = s:option(Flag, "upload_files", translate("Upload OpenVPN authentication files"),
		translate("Upload OpenVPN authentication files, which will be automatically included in configuration"))
	upload_files:depends({enable_custom = "1"})

local temp = s:option(DummyValue, "_temp", translate(""), translate(""))
	temp.rawhtml = true
	temp.default = "<div style='padding-left: 20%; padding-bottom: 20px;'>"
		.. "Uploaded certificate file locations will be automatically added into configuration."
		.. "</div>"
	temp:depends({enable_custom = "1", upload_files = "1"})

local custom_config = s:option(FileUpload, "config", translate("OpenVPN configuration file"),
	translate("Warning! This will overwrite your current configuration."))
	custom_config:depends("enable_custom", "1")

	function custom_config.parse(self, section, novld)
		local fvalue = self:formvalue(section) or nil
		local enable_custom = m:formvalue("cbid.openvpn." .. section .. ".enable_custom") or "0"

		if enable_custom == "1" and (fvalue == nil or fvalue == "") then
			local _, val_err = self:validate(nil, section)
			self:add_error(section, "missing", val_err)
		end

		AbstractValue.parse(self, section, novld)
	end

	function custom_config:validate(value)
		if value == nil then
			m.message = "err: OpenVPN configuration file is empty!"
			return nil
		end

		local val = value
		local command = "cat " .. val .. " | sed \"s/[;#].*$//\"" .. "| grep \'"
			.. (TMODE == "server" and "^[[:space:]]*client[[:space:]]*$"
			or "^[[:space:]]*server[[:space:]]") .. "\'"
		if val and sys.exec(command) ~= "" then
			m.message = "err: Bad configuration file. Currently configuring a "
				.. (TMODE or "(unknown)") .. " instance."
			failure = true
			return nil
		end
		return value
	end

local dev_type, value, num, num1

if TMODE == "client" then
	dev_type = s:option(ListValue, "dev", translate("TUN/TAP"), translate("Virtual VPN interface type"))
	--MANO dev_type:depends("_auth", "tls")
	dev_type.default = "tun"
	dev_type:value("tun", translate("TUN (tunnel)"))
	dev_type:depends({enable_custom = ""})

	value = m.uci:get("openvpn", VPN_INST, "dev") or ""
	num = utl.trim(sys.exec(" cat /etc/config/openvpn | grep -c \"option dev 'tap'\""))
	num1 = utl.trim(sys.exec(" cat /etc/config/openvpn | grep -c \"option dev tap\""))

	if value == "tap" then
		dev_type:value("tap", translate("TAP (bridged)"))
	elseif tonumber(num) == 0 and tonumber(num1) == 0 then
		dev_type:value("tap", translate("TAP (bridged)"))
	end
else
	dev_type = s:option(Value, "dev", translate("TUN/TAP"), translate("Virtual VPN interface type"))
	dev_type.template = "openvpn/tun_tap"
	dev_type:depends({enable_custom = ""})
end

function dev_type.cfgvalue(self, section)
	local value = m.uci:get("openvpn", section, "dev")
	if value and "tun" == value:match("%l+") then
		value = "tun"
	else
		value = "tap"
	end
	return value
end

function dev_type.write(self, section, value)
	if "tun" == value then
		if TMODE == "server" then
			m.uci:set("openvpn", VPN_INST, "dev", value.."_s"..TNAME)
		else
			m.uci:set("openvpn", VPN_INST, "dev", value.."_c"..TNAME)
		end
	else
		m.uci:set("openvpn", VPN_INST, "dev", "tap")
	end
	m.uci:save("openvpn")
	m.uci:commit("openvpn")
end

function o.write(self, section, value)
	if value == self.enabled then
		m.uci:set("openvpn", VPN_INST, "enable", "1")
		m.uci:save("openvpn")
	else
		m.uci:set("openvpn", VPN_INST, "enable", "0")
		m.uci:save("openvpn")
	end

	-- turint galvoje, kad kai ne tls, dev nera
	local devType = dev_type:formvalue(section) or nil

	if not devType then
		m.uci:set("openvpn", VPN_INST, "dev", "tun")
		m.uci:save("openvpn")
	end

	local auth = tostring(m:formvalue("cbid.openvpn." .. VPN_INST .. "._auth")) or ""

	if TMODE == "server" and devType == "tap" and
		(auth == "tls" or auth == "tls/pass" or auth == "pass")
	then
		m.uci:delete("openvpn", VPN_INST, "server")
		m.uci:set("openvpn", VPN_INST, "server_bridge", "nogw")
		m.uci:set("openvpn", VPN_INST, "mode", "server")
		m.uci:set("openvpn", VPN_INST, "tls-server", "1")
		m.uci:save("openvpn")
	else
		m.uci:delete("openvpn", VPN_INST, "server_bridge")
		m.uci:delete("openvpn", VPN_INST, "mode")
		m.uci:delete("openvpn", VPN_INST, "tls-server")
		m.uci:save("openvpn")
	end

	m.uci:commit("openvpn")
end

if VPN_INST then
	TMODE = VPN_INST_UNHEXED:match("%l+")
	if TMODE == "client" then
		local proto = s:option( ListValue, "proto", translate("Protocol"),
			translate("A transport protocol used for connection. "
			.. "You can choose here between TCP and UDP"))
		proto.default = "udp"
		proto:value("udp", translate("UDP"))
		proto:value("tcp-client", translate("TCP"))
		proto:depends({enable_custom = ""})
	elseif TMODE == "server" then
		local proto = s:option( ListValue, "proto", translate("Protocol"),
			translate("A transport protocol used for connection. "
			.. "You can choose here between TCP and UDP"))
		proto.default = "udp"
		proto:value("udp", translate("UDP"))
		proto:value("tcp-server", translate("TCP"))
		proto:depends({enable_custom = ""})
	end
end

o = s:option(Value, "port", translate("Port"), translate("TCP/UDP port for both local and remote "
	.. "endpoint. Make sure that this port is open in firewall"))
	o.default = "1194"
	o.datatype = "port"
	--o.rmempty = false
	o:depends({enable_custom = ""})

function o.write(self, section, value)
	m.uci:set("openvpn", VPN_INST, "port", value)
	m.uci:save("openvpn")
	local fwRuleInstName = "nil"
	local needsPortUpdate = false

	m.uci:foreach("firewall", "rule", function(s)
		if s.name == "Allow-vpn-traffic" then
			fwRuleInstName = s[".name"]
			m.uci:set("firewall", fwRuleInstName, "dest_port", value)
			m.uci:save("firewall")
			m.uci.commit("firewall")
		end
	end)
end

lzo = s:option( Flag, "comp_lzo", "LZO", translate("Use fast LZO compression. With LZO compression your VPN connection will generate less network traffic") )
lzo:depends({enable_custom = ""})

function lzo.write(self, section, value)
	local val = (value == "1") and "yes" or "no"
	m.uci:set("openvpn", section, "comp_lzo", "yes")
	m.uci:save("openvpn")
	m.uci.commit("openvpn")
end

function lzo.cfgvalue(self, section)
	local value = m.uci:get("openvpn", section, "comp_lzo")
	return (value == "yes") and "1" or "0"
end

local auth
if TMODE == "client" then
	auth = s:option(ListValue,"_auth", translate("Authentication"),
		translate("Authentication mode used to secure data session"))
		auth.default = "tls"
		auth:value("skey", translate("Static key"))
		auth:value("tls", translate("TLS"))
		auth:value("tls/pass", translate("TLS/Password"))
		auth:value("pass", translate("Password"))
		auth.nowrite = true
		auth:depends({upload_files = "1"})
		auth:depends({enable_custom = ""})
else
	auth = s:option(Value,"_auth", translate("Authentication"),
		translate("Authentication mode used to secure data session") )
	auth.template = "openvpn/auth"
	auth:depends({upload_files = "1"})
	auth:depends({enable_custom = ""})
end

local cipher = s:option(ListValue, "cipher", translate("Encryption"), translate("Packet encryption algorithm (cipher)"))
cipher.default = "BF-CBC"
cipher:value("none", translate("none"))
cipher:value("DES-CBC", translate("DES-CBC 64"))
cipher:value("RC2-CBC", translate("RC2-CBC 128"))
cipher:value("DES-EDE-CBC", translate("DES-EDE-CBC 128"))
cipher:value("DES-EDE3-CBC", translate("DES-EDE3-CBC 192"))
cipher:value("DESX-CBC", translate("DESX-CBC 192"))
cipher:value("BF-CBC", translate("BF-CBC 128 (default)"))
cipher:value("RC2-40-CBC", translate("RC2-40-CBC 40"))
cipher:value("CAST5-CBC", translate("CAST5-CBC 128"))
cipher:value("RC2-64-CBC", translate("RC2-64-CBC 64"))
cipher:value("AES-128-CFB", translate("AES-128-CFB 128"), {_auth = "tls", enable_custom = ""}, {_auth = "tls/pass", enable_custom = ""})
cipher:value("AES-128-CFB1", translate("AES-128-CFB1 128"), {_auth = "tls", enable_custom = ""}, {_auth = "tls/pass", enable_custom = ""})
cipher:value("AES-128-CFB8", translate("AES-128-CFB8 128"), {_auth = "tls", enable_custom = ""}, {_auth = "tls/pass", enable_custom = ""})
cipher:value("AES-128-OFB", translate("AES-128-OFB 128"), {_auth = "tls", enable_custom = ""}, {_auth = "tls/pass", enable_custom = ""})
cipher:value("AES-128-CBC", translate("AES-128-CBC 128"))
cipher:value("AES-128-GCM", translate("AES-128-GCM 128"), {_auth = "tls", enable_custom = ""}, {_auth = "tls/pass", enable_custom = ""})
cipher:value("AES-192-CFB", translate("AES-192-CFB 192"), {_auth = "tls", enable_custom = ""}, {_auth = "tls/pass", enable_custom = ""})
cipher:value("AES-192-CFB1", translate("AES-192-CFB1 192"), {_auth = "tls", enable_custom = ""}, {_auth = "tls/pass", enable_custom = ""})
cipher:value("AES-192-CFB8", translate("AES-192-CFB8 192"), {_auth = "tls", enable_custom = ""}, {_auth = "tls/pass", enable_custom = ""})
cipher:value("AES-192-OFB", translate("AES-192-OFB 192"), {_auth = "tls", enable_custom = ""}, {_auth = "tls/pass", enable_custom = ""})
cipher:value("AES-192-CBC", translate("AES-192-CBC 192"))
cipher:value("AES-192-GCM", translate("AES-192-GCM 192"), {_auth = "tls", enable_custom = ""}, {_auth = "tls/pass", enable_custom = ""})
cipher:value("AES-256-CFB", translate("AES-256-CFB 256"), {_auth = "tls", enable_custom = ""}, {_auth = "tls/pass", enable_custom = ""})
cipher:value("AES-256-CFB1", translate("AES-256-CFB1 256"), {_auth = "tls", enable_custom = ""}, {_auth = "tls/pass", enable_custom = ""})
cipher:value("AES-256-CFB8", translate("AES-256-CFB8 256"), {_auth = "tls", enable_custom = ""}, {_auth = "tls/pass", enable_custom = ""})
cipher:value("AES-256-OFB", translate("AES-256-OFB 256"), {_auth = "tls", enable_custom = ""}, {_auth = "tls/pass", enable_custom = ""})
cipher:value("AES-256-CBC", translate("AES-256-CBC 256"))
cipher:value("AES-256-GCM", translate("AES-256-GCM 256"), {_auth = "tls", enable_custom = ""}, {_auth = "tls/pass", enable_custom = ""})
cipher:depends({enable_custom = ""})

local tls_cipher = s:option(ListValue, "_tls_cipher", translate("TLS cipher"),
	translate("Packet encryption algorithm (cipher)"))
	tls_cipher:value("all", translate("All"))
	tls_cipher:value("dhe_rsa", translate("DHE + RSA"))
	tls_cipher:value("custom", translate("Custom"))
	tls_cipher:depends({_auth="tls", enable_custom = ""})
	tls_cipher:depends({_auth="tls/pass", enable_custom = ""})
	tls_cipher:depends({_auth="pass", enable_custom = ""})

local tls_cipher2 = s:option(DynamicList, "_tls_cipher2", translate("Allowed TLS ciphers"))
	tls_cipher2:depends({_tls_cipher="custom", enable_custom = ""})
function tls_cipher2.cfgvalue(self, section)
	local val = self.map:get(section, "tls_cipher")
	if val ~= nil and val ~= "" then
		t=split(val,"\:")
	else
		t={""}
	end
	return t
end

function tls_cipher2.write(self, section)
end

if TMODE == "client" then

o = s:option( Value,"remote", translate("Remote host/IP address"), translate("IP address or domain name of OpenVPN server"))
	--o.datatype = "ipaddr"
	--o.rmempty = false
	o:depends({enable_custom = ""})

o = s:option( Value, "resolv_retry" ,translate("Resolve retry"), translate("Try to resolve server hostname for x seconds before giving up"))
	o.forcewrite = true
	o:depends({enable_custom = ""})

function o.cfgvalue(self, section)
	local val = self.map:get(section, self.option)
	return val and val or "infinite"
end

function o.write(self, section, value)
	if not value then
		value = "infinite"
	end
	AbstractValue.write(self, section, value)
end

-- 	o = s:option( Flag, "nobind", translate("No Bind"), translate("Do not bind to local address and port") )
-- 	o:depends({dev="tun", _auth="tls"})
-- 	o:depends({dev="tap", _auth="tls"})

keep = s:option( Value, "keepalive", translate("Keep alive"), translate("Try to keep a connection alive. Two values are required: ping_interval and ping_restart, e.g. 10 120") )
	keep:depends({enable_custom = ""})

function keep:validate(Value)
	local failure
	if Value == "" then return Value end
	if not Value:match("%d+ %d+") then
		m.message = translate("Keep alive value is not correct! Must be two integer values, e.g. 10 120")
		failure = true
		return nil
	end

	return Value

end



--[[#########################Client settings depdends on tls only############################]]--
-- 	o = s:option( Flag, "client", translate("Client") )
-- 	o:depends({_auth="tls"})

--[[#########################Client settings depdends on tun and skey############################]]--
--[[ Ifconfig option ]]--
local ifconf_local = s:option( Value, "_ifconfig" ,translate("Local tunnel endpoint IP"), translate("IP address of virtual local network interface") )
	ifconf_local:depends({dev="tun", _auth="skey", enable_custom = ""})
	--MANO ifconf_local:depends({_auth="skey"})
	ifconf_local.placeholder = "172.16.0.2"
	ifconf_local.datatype = "ip4addr"
-- 	ifconf_local.rmempty = false

function ifconf_local.write() end

function ifconf_local.cfgvalue(self, section)
	return split_by_word(self.config, section, "ifconfig", 1)
end

local ifconf_remote = s:option( Value, "ifconfig", translate("Remote tunnel endpoint IP"), translate("IP address of virtual remote network interface") )
	ifconf_remote:depends({dev="tun", _auth="skey", enable_custom = ""})
	--MANO ifconf_remote:depends({_auth="skey"})
	ifconf_remote.forcewrite = true
	ifconf_remote.placeholder = "172.16.0.1"
	ifconf_remote.datatype = "ip4addr"
-- 	ifconf_remote.rmempty = false

function ifconf_remote.write(self,section, ip2)
	local ip1 = ifconf_local:formvalue(section)

	if ip1 and ip2 then
		AbstractValue.write(self, section, ip1.." "..ip2)
	end
end

function ifconf_remote.cfgvalue(self, section)
	return split_by_word(self.config, section, "ifconfig", 2)
end


--[[ Route option ]]--
local route_ip = s:option( Value, "_route" ,translate("Remote network IP address"), translate("IP address of remote LAN network"))
	route_ip:depends({dev = "tun", enable_custom = ""})
	--MANO route_ip:depends({_auth="skey"})
	route_ip.datatype = "ipaddr"

function ifconf_local.write() end

function route_ip.cfgvalue(self, section)
	return split_by_word(self.config, section, "route", 1)
end

local route_mask = s:option( Value, "route", translate("Remote network IP netmask"), translate("Subnet mask of remote LAN network"))
	route_mask:depends({dev = "tun", enable_custom = ""})
	--MANO route_mask:depends({_auth="skey"})
	route_mask.forcewrite = true
	route_mask.placeholder = "255.255.255.0"
	route_mask.datatype = "ip4addr"

function route_mask.cfgvalue(self, section)
	return split_by_word(self.config, section, "route", 2)
end

function route_mask.write(self, section, remote_mask)
	local remote_ip = route_ip:formvalue(section)

	if remote_ip and remote_mask then
		AbstractValue.write(self, section, remote_ip.." "..remote_mask)
	end
end

function route_mask:validate(Value)
	local txtip = tostring(m:formvalue("cbid.openvpn." .. VPN_INST .. "._route"))
	--local remote_ip = route_ip:formvalue(section)
	local lan_ip = utl.trim(m.uci:get("network", "lan", "ipaddr"))
	local networkip = utl.trim(luci.util.exec("ipcalc.sh ".. txtip .." ".. Value .." |grep NETWORK= | cut -d'=' -f2 | tr -d ''"))
	local broadcast = utl.trim(luci.util.exec("ipcalc.sh ".. txtip .." ".. Value .." |grep BROADCAST= | cut -d'=' -f2 | tr -d ''"))
	networkip = networkip:match("[%w%.]+")
	broadcast = broadcast:match("[%w%.]+")
	if txtip == networkip then
		if lan_ip > networkip and lan_ip < broadcast then
				m.message = translatef("err: Remote network IP includes router LAN, choose other")
				return nil
			else
				return Value
			end
		else
			m.message = translatef("err: To match specified netmask, remote network IP address should be %s", networkip);
			return nil
		end
		return Value
	end
	--[[#########################Client settings depdends on tap and tls############################]]--
	o = s:option( DummyValue, "user")

	function o:render() end

	function o:parse(section)
		--local dev_val  = dev_type:formvalue(section)
		local dev_val  = "tun"
		local auth_val = auth:formvalue(section)

		if (auth_val=="tls" or auth_val == "tls/pass" or auth_val == "pass") and dev_val=="tap" then
			AbstractValue.write(self, section, "root")
			self.section.changed = true
		else
			AbstractValue.remove(self, section)
			self.section.changed = true
		end
	end

	--[[#########################Client settings depdends on tun and tls############################]]--
-- 	o = s:option(DummyValue, "push")
-- 	function o:render() end
--
-- 	function o:parse(section)
-- 		local dev_val  = dev_type:formvalue(section)
-- 		local auth_val = auth:formvalue(section)
--
-- 		if auth_val=="tls" and dev_val=="tun" then
-- 			local net_ip   = uci:get("network", "lan", "ipaddr")
-- 			local net_mask = uci:get("network", "lan", "netmask")
-- 			if net_ip and net_mask then
-- 				local cidr_inst = ipc.IPv4(net_ip, net_mask)
-- 				if cidr_inst then
-- 					local ip = cidr_inst:network()
-- 					local mask = cidr_inst:mask()
-- 					if ip and mask then
-- 						if AbstractValue.write(self, section, "route "..ip:string().." "..mask:string()) then
-- 							self.section.changed = true
-- 						end
-- 					end
-- 				end
-- 			end
-- 		else
-- 			AbstractValue.remove(self, section)
-- 			self.section.changed = true
-- 		end
-- 	end

end


if TMODE == "server" then

-- 	o = s:option( DummyValue, "_keepalive_static")
-- 	o:depends({dev="tun"})
-- 	function o:render() end
-- 	function o:write(self,section, value)
-- 			m.uci:set("openvpn", VPN_INST, "keepalive", "10 120")
-- 			m.uci:save("openvpn")
-- 	end

	--[[#########################Server settings depdends on tls only############################]]--
	o = s:option( Flag, "client_to_client", translate("Client to client"), translate("Allow client-to-client traffic") )
		o:depends({ _auth="tls", enable_custom = ""})
		o:depends({ _auth="tls/pass", enable_custom = ""})
		o:depends({ _auth="pass", enable_custom = ""})

	--o = s:option( Flag, "duplicate_cn", translate("Allow duplicate certificates") )
	--o:depends({ _auth="tls"})

 	o = s:option( Value, "_keepalive_tls", translate("Keep alive"), translate("Try to keep a connection alive. Two values are required:"
			.. " ping_interval and ping_restart, e.g. 10 120. These values will be pushed to all connected clients") )
		o:depends({_auth="tls", enable_custom = ""})
		o:depends({_auth="tls/pass", enable_custom = ""})
		o:depends({_auth="pass", enable_custom = ""})
		--MANO
		o:depends({_auth="skey", dev="tap", enable_custom = ""})

	function o.write(self,section, value)
		m.uci:set("openvpn", VPN_INST, "keepalive", value)
		m.uci:save("openvpn")
	end
	function o.cfgvalue(self, section)
		return self.map:get(section, "keepalive")
	end
-- 	o = s:option( DummyValue, "ifconfig_pool_persist")
-- 	function o:render() end
--
-- 	function o:parse(section)
-- 		local auth_val = auth:formvalue(section)
--
-- 		if auth_val=="tls" then
-- 			AbstractValue.write(self, section, "/tmp/ipp.txt")
-- 			self.map:set(section, "status", "/tmp/openvpn-status.log")
-- 			self.section.changed = true
-- 		else
-- 			AbstractValue.remove(self, section)
-- 			self.map:del(section, "status")
-- 			self.section.changed = true
-- 		end
-- 	end


	--[[#########################Server settings depdends on tun and skey############################]]--
	--[[ Ifconfig option ]]--
	local ifconf_local = s:option( Value, "_ifconfig" ,translate("Local tunnel endpoint IP"), translate("IP address of virtual local network interface"))
		ifconf_local:depends({dev="tun", _auth="skey", enable_custom = ""})
		--MANO ifconf_local:depends({_auth="skey"})
		ifconf_local.placeholder = "172.16.0.1"
		ifconf_local.datatype = "ip4addr"
		--ifconf_local.rmempty = false

	function ifconf_local.write() end

	function ifconf_local.cfgvalue(self, section)
		return split_by_word(self.config, section, "ifconfig", 1)
	end

	local ifconf_remote = s:option( Value, "ifconfig", translate("Remote tunnel endpoint IP"), translate("IP address of virtual remote network interface"))
		ifconf_remote:depends({dev="tun", _auth="skey", enable_custom = ""})
		--MANO ifconf_remote:depends({_auth="skey"})
		ifconf_remote.forcewrite = true
		ifconf_remote.placeholder = "172.16.0.2"
		ifconf_remote.datatype = "ip4addr"
		--ifconf_remote.rmempty = false

	function ifconf_remote.write(self,section, ip2)
		local ip1 = ifconf_local:formvalue(section)

		if ip1 and ip2 then
			AbstractValue.write(self, section, ip1.." "..ip2)
		end
	end

	function ifconf_remote.cfgvalue(self, section)
		return split_by_word(self.config, section, "ifconfig", 2)
	end

	--serveriui sito lyg nereikia
	-- [[ resolv_retry option ]]--
	--o = s:option( Value, "resolv_retry", translate("Resolve Retry"), translate("Sets time in seconds to try resolve server hostname periodically"))
	---- o:depends({dev="tun", _auth="skey"})
	--o:depends({_auth="skey"})
	--o.forcewrite = true

	-- function o.cfgvalue(self, section)
		-- local val = self.map:get(section, self.option)
		-- return val and val or "infinite"
	-- end

	-- function o.write(self, section, value)
		-- if not value then
			-- value = "infinite"
		-- end
		-- AbstractValue.write(self, section, value)
	-- end


	--[[ Route option ]]--
	local sroute_ip = s:option( Value, "_route" ,translate("Remote network IP address"), translate("IP address of remote LAN network") )
		sroute_ip:depends({dev="tun", _auth="skey", enable_custom = ""})
		sroute_ip.datatype = "ipaddr"
		--MANO sroute_ip:depends({_auth="skey"})

	function sroute_ip.cfgvalue(self, section)
		return split_by_word(self.config, section, "route", 1)
	end

	local sroute_mask = s:option( Value, "route", translate("Remote network netmask"), translate("Subnet mask of remote LAN network"))
		sroute_mask:depends({dev="tun", _auth="skey", enable_custom = ""})
		--MANO sroute_mask:depends({_auth="skey"})
		sroute_mask.forcewrite = true
		sroute_mask.placeholder = "255.255.255.0"
		sroute_mask.datatype = "ipaddr"

	function sroute_mask.cfgvalue(self, section)
		return split_by_word(self.config, section, "route", 2)
	end

	function sroute_mask.write(self, section, remote_mask)
		local remote_ip = sroute_ip:formvalue(section)
		if remote_ip and remote_mask then
			AbstractValue.write(self, section, remote_ip.." "..remote_mask)
		end
	end

	function sroute_mask:validate(Value)
		local txtip = tostring(m:formvalue("cbid.openvpn." .. VPN_INST .. "._route"))
		--local remote_ip = route_ip:formvalue(section)
		local lan_ip = utl.trim(m.uci:get("network", "lan", "ipaddr"))
		local networkip = utl.trim(luci.util.exec("ipcalc.sh ".. txtip .." ".. Value .." |grep NETWORK= | cut -d'=' -f2 | tr -d ''"))
		local broadcast = utl.trim(luci.util.exec("ipcalc.sh ".. txtip .." ".. Value .." |grep BROADCAST= | cut -d'=' -f2 | tr -d ''"))
		networkip = networkip:match("[%w%.]+")
		broadcast = broadcast:match("[%w%.]+")
		if txtip == networkip then
			if lan_ip > networkip and lan_ip < broadcast then
				m.message = translatef("err: Remote network IP includes router LAN, choose other")
				return nil
			else
				return Value
			end
		else
			m.message = translatef("err: To match specified netmask, remote network IP address should be %s", networkip);
			return nil
		end
		return Value
	end

	--[[#################Server settings depdends on tap and tls################]]--
	function o:parse(section)
		local dev_val  = dev_type:formvalue(section)
		local auth_val = auth:formvalue(section)

		if (auth_val=="tls" or auth_val=="tls/pass" or auth_val=="pass") and dev_val=="tap" then
			m.uci:set("openvpn", VPN_INST, "mode", "server")
			m.uci:set("openvpn", VPN_INST, "tls-server", "1")
		else
			m.uci:delete("openvpn", VPN_INST, "mode")
			 m.uci:delete("openvpn", VPN_INST, "tls-server")
		end

		if auth_val=="tls" or auth_val=="tls/pass" or auth_val=="pass" then
			m.uci:delete("openvpn", VPN_INST, "ifconfig")
		end
		m.uci:save("openvpn")
		m.uci:commit("openvpn")
	end

	--[[ server_bridge option ]]--
-- 	local sbridge_from = s:option( Value, "_server_bridge_from", translate("IP address pool for clients, from") )
-- 	sbridge_from:depends({dev="tap", _auth="tls"})
-- 	sbridge_from.datatype = "ipaddr"
-- 	sbridge_from.nowrite = true
--
--
-- 	function sbridge_from.cfgvalue(self, section)
-- 		return split_by_word(self.config, section, "server_bridge", 3)
-- 	end


-- 	local sbridge_to = s:option( Value, "server_bridge", translate("IP address pool for clients, to") )
-- 	sbridge_to:depends({dev="tap", _auth="tls"})
-- 	sbridge_to.forcewrite = true
-- 	sbridge_to.datatype = "ipaddr"
--
-- 	function sbridge_to.write(self, section, br_to)
-- 			local net_ip   = uci:get("network", "lan", "ipaddr")
-- 			local net_mask = uci:get("network", "lan", "netmask")
-- 			local br_from = sbridge_from:formvalue(section)
--
-- 			if net_ip and net_mask and br_from and br_to then
-- 				local s_bridge = net_ip.." "..net_mask.." "..br_from.." "..br_to
-- 				AbstractValue.write(self, section, s_bridge)
-- 			end
-- 	end
--
-- 	function sbridge_to.cfgvalue(self, section)
-- 		return split_by_word(self.config, section, "server_bridge", 4)
-- 	end

	--[[#################Server settings depdends on tun and tls################]]--
	--[[ server option ]]--
	local serv_ip = s:option( Value, "_server", translate("Virtual network IP address"), translate("IP address used for virtual network"))
		--serv_ip:depends({dev="tun", _auth="tls"})
		serv_ip:depends({_auth="tls", dev="tun", enable_custom = ""})
		serv_ip:depends({_auth="tls/pass", dev="tun", enable_custom = ""})
		serv_ip:depends({_auth="pass", dev="tun", enable_custom = ""})
		serv_ip.placeholder = "172.16.1.0"
		serv_ip.datatype = "ip4addr"

	function serv_ip.write() end

	function serv_ip.cfgvalue(self, section)
		return split_by_word(self.config, section, "server", 1)
	end


	local serv_mask = s:option( Value, "server" ,translate("Virtual network netmask"), translate("Subnet mask used for virtual network"))
	--serv_mask:depends({dev="tun", _auth="tls"})
		serv_mask:depends({_auth="tls", dev="tun", enable_custom = ""})
		serv_mask:depends({_auth="tls/pass", dev="tun", enable_custom = ""})
		serv_mask:depends({_auth="pass", dev="tun", enable_custom = ""})
		serv_mask.forcewrite = true
		serv_mask.placeholder = "255.255.255.0"
		serv_mask.datatype = "ipaddr"

	function serv_mask.cfgvalue(self, section)
		return split_by_word(self.config, section, "server", 2)
	end

	function serv_mask.write(self, section, serv_mask)
		local sip = serv_ip:formvalue(section)

		if sip and serv_mask then
			AbstractValue.write(self, section, sip .. " ".. serv_mask)
		end
	end

	function serv_mask:validate(Value)
		local txtip = tostring(m:formvalue("cbid.openvpn." .. VPN_INST .. "._server"))
		--local remote_ip = route_ip:formvalue(section)
		local networkip = tostring(luci.util.exec("ipcalc.sh ".. txtip .." ".. Value .." |grep NETWORK= | cut -d'=' -f2 | tr -d ''"))
		networkip = networkip:match("[%w%.]+")
		if txtip == networkip then
			return Value
		else
			m.message = translatef("err: To match specified netmask, virtual network IP address should be %s", networkip);
			return nil
		end
		return Value
	end

	local serv_push = s:option(DynamicList, "push", translate("Push option"), translate("Push a configuration option back to the client for remote execution"))
		serv_push:depends({_auth="tls", enable_custom = ""})
		serv_push:depends({_auth="tls/pass", enable_custom = ""})
		serv_push:depends({_auth="pass", enable_custom = ""})
		serv_push.placeholder = "route 192.168.1.0 255.255.255.0"

if TMODE == "server" then
	local multi_cl = s:option(Flag, "duplicate_cn", translate("Allow duplicate certificates"), translate("All clients can have same certificates"))
	--MANO multi_cl:depends("dev", "tap")
		multi_cl:depends({_auth="tls", enable_custom = ""})
		multi_cl:depends({_auth="tls/pass", enable_custom = ""})
		multi_cl:depends({_auth="pass", enable_custom = ""})
end
	--[[ push option ]]--
-- 	o = s:option(DummyValue, "push")
-- 	function o:render() end
--
-- 	function o:parse(section)
-- 		local dev_val  = dev_type:formvalue(section)
-- 		local auth_val = auth:formvalue(section)
--
-- 		if auth_val=="tls" and dev_val=="tun" then
-- 			local net_ip   = uci:get("network", "lan", "ipaddr")
-- 			local net_mask = uci:get("network", "lan", "netmask")
-- 			if net_ip and net_mask then
-- 				local cidr_inst = ipc.IPv4(net_ip, net_mask)
-- 				if cidr_inst then
-- 					local ip = cidr_inst:network()
-- 					local mask = cidr_inst:mask()
-- 					if ip and mask then
-- 						if AbstractValue.write(self, section, "route "..ip:string().." "..mask:string()) then
-- 							self.section.changed = true
-- 						end
-- 					end
-- 				end
-- 			end
-- 		else
-- 			AbstractValue.remove(self, section)
-- 			self.section.changed = true
-- 		end
-- 	end

	--[[ client_config_dir option ]]--
-- 	o = s:option( DummyValue, "client_config_dir")
-- 	function o:render() end
--
-- 	function o:parse(section)
-- 		--local dev_val  = dev_type:formvalue(section)
-- 		local dev_val  = "tun"
-- 		local auth_val = auth:formvalue(section)
--
-- 		--no need for selective client configuration if we do not provide
-- 		-- webUI to configure them.
-- 		AbstractValue.remove(self, section)
-- 		self.section.changed = true
--
-- 		if auth_val=="tls" and dev_val=="tun" then
-- 			AbstractValue.write(self, section, "/etc/openvpn/ccd/")
-- 			self.section.changed = false
-- 		else
-- 			AbstractValue.remove(self, section)
-- 			self.section.changed = true
-- 		end
-- 	end
end

auth_alg = s:option( ListValue, "auth", translate("HMAC authentication algorithm"), translate(""))
	auth_alg:depends({_auth="tls", enable_custom = ""})
	auth_alg:depends({_auth="pass", enable_custom = ""})
	auth_alg:depends({_auth="tls/pass", enable_custom = ""})
	auth_alg.default = "sha1"
	auth_alg:value("none", translate("None"))
	auth_alg:value("md5", translate("MD5"))
	auth_alg:value("sha1", translate("SHA1 (default)"))
	auth_alg:value("sha256", translate("SHA256"))
	auth_alg:value("sha384", translate("SHA384"))
	auth_alg:value("sha512", translate("SHA512"))

tls_auth = s:option( ListValue, "_tls_auth", "Additional HMAC authentication", translate("Add an additional layer of HMAC authentication on top of the TLS control channel to protect against DoS attacks.") )
	tls_auth:value("none", translate("None"))
	tls_auth:value("tls-auth", translate("Authentication only (tls-auth)"))
	tls_auth:value("tls-crypt", translate("Authentication and encryption (tls-crypt)"))
	tls_auth.default = "none"
	tls_auth:depends({_auth="tls"})
	tls_auth:depends({_auth="pass"})
	tls_auth:depends({_auth="tls/pass"})
	tls_auth.nowrite = true

function tls_auth:validate(value)
	local file
	if value == "tls-auth" then
		file = m:formvalue("cbid.openvpn." .. VPN_INST .. ".tls_auth")
		if not file or file == "" or file:len() < 1 then
			m.message = translate("err: No HMAC authentication key file selected")
			return nil
		end
	elseif value == "tls-crypt" then
		file = m:formvalue("cbid.openvpn." .. VPN_INST .. ".tls_crypt")
		if not file or file == "" or file:len() < 1 then
			m.message = translate("err: No HMAC authentication key file selected")
			return nil
		end
	end
	return value
end

tls_key = s:option( FileUpload, "tls_auth", translate("HMAC authentication key"), translate(""))
	tls_key:depends({_tls_auth="tls-auth"})

function tls_key.cfgvalue(self, section)
	local val = self.map:get(section, "tls_auth")
	if val ~= nil and val ~= "" then
		t=split(val," ")
	else
		t={""}
	end
	return t[1]
end

function tls_key.write(self, section, value)
	local s = auth_key_direction:formvalue(section)
	if s == "none" then
		AbstractValue.write(self, section, value)
	else
		AbstractValue.write(self, section, value .. " ".. s)
	end
end

tls_crypt_key = s:option( FileUpload, "tls_crypt", translate("HMAC authentication key"), translate(""))
	tls_crypt_key:depends({_tls_auth="tls-crypt"})

auth_key_direction = s:option( ListValue, "auth_key_direction", translate("HMAC key direction"), translate("HMAC authentication key direction value is arbitrary and must be opposite between communicating parties (or omitted entirely)"))
	auth_key_direction:depends({_tls_auth="tls-auth"})
	auth_key_direction.default = "1"
	auth_key_direction:value("0", translate("0"))
	auth_key_direction:value("1", translate("1"))
	auth_key_direction:value("none", translate("None"))

function auth_key_direction.write(self, section, value)
	AbstractValue.write(self, section, value)

	local auth_key_direction = self.map:get(section, "auth_key_direction")
	local tls_auth = self.map:get(section, "tls_auth")

	if tls_auth ~= nil and tls_auth ~= "" and auth_key_direction ~= nil and auth_key_direction ~= "" then
		tls_auth = split(tls_auth," ")
		tls_auth = tls_auth[1]
		if auth_key_direction ~= "none" then
			tls_auth = tls_auth .. " " .. auth_key_direction
		end
		m.uci:set("openvpn", VPN_INST, "tls_auth", tls_auth)
	end
end

if TMODE == "client" then
	--~ Nebenaudojama nuo 2,4 openvpn versijos
	--~ o = s:option( Value, "max_routes", translate("Max routes"), translate("Allow a maximum number of routes to be pulled from an OpenVPN server."))
	--~ o.datatype = "range(1,1000)"
	--~ o.default = "100"

	o = s:option( Value, "user", translate("User name"), translate("VPN client user name"))
		o:depends({_auth="pass"})
		o:depends({_auth="tls/pass"})
		o.datatype = "username_validate"

		function o.cfgvalue(self, section)
			local userval  = utl.trim(sys.exec("head -n 1 /etc/openvpn/auth_"..VPN_INST.."")) or ""
			return userval
		end
		function o.write(self, section)
		end

	o = s:option( Value, "pass", translate("Password"), translate("VPN client password. Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:;[]"))
		o.password = true
		o:depends({_auth="pass"})
		o:depends({_auth="tls/pass"})
		o.datatype = "password"

		function o.cfgvalue(self, section)
			local passval = utl.trim(sys.exec("head -n 2 /etc/openvpn/auth_"..VPN_INST.." | tail -n 1 2>/dev/null")) or ""
			return passval
		end
		function o.write(self, section)
		end

	FileUpload.size = "64000"
	FileUpload.sizetext = translate("Selected file is too large, max 64 KiB")
	FileUpload.sizetextempty = translate("Selected file is empty")

	extra = s:option(DynamicList, "_extra", "Extra options", "Enter any additional options to be added to the OpenVPN configuration.")
		extra:depends({enable_custom = ""})

	o = s:option(Flag, "use_pkcs", translate("Use PKCS #12 format"), translate("Use PKCS #12 archive file format to bundle all the members of a chain of trust"))
		o:depends({_auth="tls"})
		o:depends({_auth="tls/pass"})
		o:depends({_auth="pass"})

	o = s:option(Value, "askpass", translate("PKCS #12 passphrase"), translate("Passphrase to decrypt PKCS #12 certificates."))
		o.password = true
		o:depends({_auth="pass", use_pkcs="1"})
		o:depends({_auth="tls", use_pkcs="1"})
		o:depends({_auth="tls/pass", use_pkcs="1"})

	o.write = function(self, section, value)
		local filename = nil
		if section and type(section) == "string" then
			filename = "/etc/openvpn/openvpn-" .. section .. "-pkcs12.pass"
		end
		if filename then
			nixio.fs.writefile(filename, value)
			return AbstractValue.write(self, section, filename)
		end
	end

	o.cfgvalue = function(self, section)
		local filename = nil
		if section and type(section) == "string" then
			filename = "/etc/openvpn/openvpn-" .. section .. "-pkcs12.pass"
		end
		if filename then
			return nixio.fs.readfile(filename) or ""
		end
		return ""
	end

	o.remove = function(self, section)
		local filename = nil
		if section and type(section) == "string" then
			filename = "/etc/openvpn/openvpn-" .. section .. "-pkcs12.pass"
		end
		if filename then
			nixio.fs.writefile(filename, "")
			return AbstractValue.remove(self, section)
		end
	end

	o = s:option( FileUpload, "ca", translate("Certificate authority"), translate("The digital certificate verifies the ownership of a public key by the named subject of the certificate"))
		o:depends({_auth="pass", use_pkcs=""})
		o:depends({_auth="tls", use_pkcs=""})
		o:depends({_auth="tls/pass", use_pkcs=""})

	o = s:option( FileUpload, "cert", translate("Client certificate"), translate("Identify a client or a user, authenticating the client to the server and establishing precisely who they are"))
		o:depends({_auth="pass", use_pkcs=""})
		o:depends({_auth="tls", use_pkcs=""})
		o:depends({_auth="tls/pass", use_pkcs=""})

	o = s:option( FileUpload, "key", translate("Client key"), translate("It has been generated for the same purpose as client certificate"))
		o:depends({_auth="pass", use_pkcs=""})
		o:depends({_auth="tls", use_pkcs=""})
		o:depends({_auth="tls/pass", use_pkcs=""})

	o = s:option(FileUpload, "pkcs12", translate("PKCS #12 certificate chain"), translate(""))
		o:depends({_auth="pass", use_pkcs="1"})
		o:depends({_auth="tls", use_pkcs="1"})
		o:depends({_auth="tls/pass", use_pkcs="1"})

	o = s:option( FileUpload, "secret", translate("Static pre-shared key"), translate("Pre-shared key (PSK) is a shared secret which was previously shared between the two parties using some secure channel before it needs to be used"))
		o:depends({_auth="skey"})

	dec_key = s:option( Value, "decrypt", translate("Private key decryption password (optional)"), translate("Decrypt private key with password.(Optional). Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
		dec_key.noautocomplete = true
		dec_key.password = true
		dec_key.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~.-]+$',0)"
		dec_key:depends({_auth="tls", enable_custom = "", use_pkcs = ""})
		dec_key:depends({_auth="tls/pass", enable_custom = "", use_pkcs = ""})

else
	FileUpload.size = "64000"
	FileUpload.sizetext = translate("Selected file is too large, max 64 KiB")
	FileUpload.sizetextempty = translate("Selected file is empty")

	o = s:option(FileUpload, "userpass", translate("Usernames & Passwords"),
		translate("File containing usernames and passwords against which "
		.. "the server can authenticate clients. Each username and password "
		.. "pair should be placed on a single line and separated by a space."))
	o:depends({_auth="tls/pass"})
	o:depends({_auth="pass"})

	o = s:option(Flag, "use_pkcs", translate("Use PKCS #12 format"), translate("Use PKCS #12 archive file format to bundle all the members of a chain of trust"))
	o:depends({_auth="tls"})
	o:depends({_auth="tls/pass"})
	o:depends({_auth="pass"})

	o = s:option(Value, "askpass", translate("PKCS #12 passphrase"), translate("Passphrase to decrypt PKCS #12 certificates."))
		o.password = true
		o:depends({_auth="pass", use_pkcs="1"})
		o:depends({_auth="tls", use_pkcs="1"})
		o:depends({_auth="tls/pass", use_pkcs="1"})

	o.write = function(self, section, value)
		local filename = nil
		if section and type(section) == "string" then
			filename = "/etc/openvpn/openvpn-" .. section .. "-pkcs12.pass"
		end
		if filename then
			nixio.fs.writefile(filename, value)
			return AbstractValue.write(self, section, filename)
		end
	end

	o.cfgvalue = function(self, section)
		local filename = nil
		if section and type(section) == "string" then
			filename = "/etc/openvpn/openvpn-" .. section .. "-pkcs12.pass"
		end
		if filename then
			return nixio.fs.readfile(filename) or ""
		end
		return ""
	end

	o.remove = function(self, section)
		local filename = nil
		if section and type(section) == "string" then
			filename = "/etc/openvpn/openvpn-" .. section .. "-pkcs12.pass"
		end
		if filename then
			nixio.fs.writefile(filename, "")
			return AbstractValue.remove(self, section)
		end
	end

	o = s:option( FileUpload, "ca", translate("Certificate authority"), translate("The digital certificate verifies the ownership of a public key by the named subject of the certificate"))
	o:depends({_auth="tls", use_pkcs=""})
	o:depends({_auth="tls/pass", use_pkcs=""})
	o:depends({_auth="pass", use_pkcs=""})

	o = s:option( FileUpload, "cert", translate("Server certificate"), translate("Certificate servers validate or certify keys as part of a public key infrastructure"))
	o:depends({_auth="tls", use_pkcs=""})
	o:depends({_auth="tls/pass", use_pkcs=""})
	o:depends({_auth="pass", use_pkcs=""})

	o = s:option( FileUpload, "key", translate("Server key"), translate("It has been generated for the same purpose as server certificate"))
	o:depends({_auth="tls", use_pkcs=""})
	o:depends({_auth="tls/pass", use_pkcs=""})
	o:depends({_auth="pass", use_pkcs=""})

	o = s:option(FileUpload, "pkcs12", translate("PKCS #12 certificate chain"), translate(""))
	o:depends({_auth="pass", use_pkcs="1"})
	o:depends({_auth="tls", use_pkcs="1"})
	o:depends({_auth="tls/pass", use_pkcs="1"})

	o = s:option( FileUpload, "dh", translate("Diffie Hellman parameters"), translate("Diffie-Hellman key exchange is a specific method of exchanging cryptographic keys"))
	o:depends({_auth="tls"})
	o:depends({_auth="tls/pass"})
	o:depends({_auth="pass"})

	o = s:option( FileUpload, "crl_verify", translate("CRL file (optional)"), translate("Revoking a certificate means to invalidate a previously signed certificate so that it can no longer be used for authentication purposes. Upload a .pem revocation file"))
	o:depends({_auth="tls"})
	o:depends({_auth="tls/pass"})
	o:depends({_auth="pass"})

	o = s:option( FileUpload, "secret", translate("Static pre-shared key"), translate("Pre-shared key (PSK) is a shared secret which was previously shared between the two parties using some secure channel before it needs to be used") )
	o:depends({_auth="skey"})

	local enable_ccd = s:option(Flag, "enable_ccd", translate("Enable manual ccd upload"),
		translate("Enable manual upload of client-config-dir files"))
	enable_ccd:depends({_auth="tls", enable_custom = "", dev="tun"})
	enable_ccd:depends({_auth="tls/pass", enable_custom = "", dev="tun"})

	o = s:option(FileUpload, "ccd_archive", translate("Client-config-dir files"),
		translate("Upload client-config-dir files wrapped up in .tar.gz."
		.. " Files will be extracted in \"/etc/openvpn/ccd\""))
	o:depends({enable_ccd="1", enable_custom = ""})

end

function getLanIp()
	local ipadresas = m.uci:get("network", "lan", "ipaddr")
	local network= {}
	for elem in ipadresas:gmatch("%d+") do
		network[#network + 1] = elem
	end
	ipadresas = network[1].."."..network[2].."."..network[3]..".0"
	return ipadresas
end

if TMODE == "server" then

	s = m:section(TypedSection, "client", translate("TLS Clients"),
		translate("Here you can add your VPN clients so that they may be reachable from the server."))
	s.template = "cbi/tblsection"
	s.addremove = true
	s.anonymous = true
	s.sectionhead = false

	-- Fix for annonymous section deletion in sequence
	function s.remove(self, novld)
		TypedSection.remove(self, novld)
		self.map.uci:commit("openvpn")
	end

	-- Filter out all the sections except for those
	-- which do not have sname field or their sname field
	-- is set to the current OpenVPN configuration name
	local lan_ip = getLanIp()
	local remote_net

	function s.cfgsections(self)
		local sections = {}
		self.map.uci:foreach(self.map.config, self.sectiontype,
			function (section)
				remote_net = luci.http.formvalue("cbid.openvpn."..section[".name"]..".pip")
				if remote_net == lan_ip then
					m.message = translate("err: Bad private network, "
						.. "please enter another one")
				end
				if self:checkscope(section[".name"]) then
					if section.sname == VPN_INST or not section.sname then
						table.insert(sections, section[".name"])
					end
				end
			end)
		return sections
	end

	o = s:option(Value, "sname", "", translate("With what OpenVPN instance should this entry "
		.. "be associated with. It is automatically set to your current OpenVPN instance" ))
	o:depends({enable_custom = ""})
	o.hidden_field = true

	function o.cfgvalue(self, section)
		local value
		value = self.map:get(section, self.option)
		if not value then
			self.map:set(section, self.option, VPN_INST)
			return VPN_INST
		else
			return value
		end
	end

	o = s:option(Value, "ept_name", translate("Endpoint name"),
		translate("Your endpoint name. E.g. MyHomeComputer"))
	o:depends({enable_custom = ""})
	o.maxWidth = "110px"

	o = s:option(Value, "cn", translate("Common name (CN)"),
		translate("Client certificate CN field. E.g. name.surname@domain.com" ))
	o:depends({enable_custom = ""})

	o = s:option(Value, "lip", translate("Virtual Local Endpoint"), translate("E.g. 10.8.1.10" ))
	o.datatype = "ip4addr"
	o:depends({enable_custom = ""})
	o.maxWidth = "110px"

	o = s:option(Value, "rip", translate("Virtual Remote Endpoint"), translate("E.g. 10.8.1.9" ))
	o.datatype = "ip4addr"
	o:depends({enable_custom = ""})
	o.maxWidth = "110px"

	o = s:option(Value, "pip", translate("Private network"),
		translate("The IP of the private <b>NETWORK</b>. E.g. 192.168.1.0" ))
	o.datatype = "remote_net"
	o:depends({enable_custom = ""})
	o.maxWidth = "110px"

	o = s:option(Value, "pnm", translate("Private netmask"),
		translate("The subnet mask of the private network. E.g. 255.255.255.0" ))
	o.datatype = "ip4addr"
	o:depends({enable_custom = ""})
	o.maxWidth = "110px"

end

function m.on_parse(self)
	--cecho("Entering on parse.")

	local _auth

	--cecho("cbid.openvpn." .. VPN_INST .. "._auth");

	_auth = m:formvalue("cbid.openvpn." .. VPN_INST .. "._auth")

	if not _auth then
		return
	end

	--cecho("_auth: [" .. (_auth or "null") .. "]")

	m.uci:delete("openvpn", VPN_INST, "client")
	m.uci:delete("openvpn", VPN_INST, "client_config_dir")
	m.uci:delete("openvpn", VPN_INST, "auth_user_pass")
	m.uci:delete("openvpn", VPN_INST, "auth_user_pass_verify")
	m.uci:delete("openvpn", VPN_INST, "script_security")
	m.uci:delete("openvpn", VPN_INST, "client_cert_not_required")

	if _auth == "tls" then
		if TMODE == "client" then
			m.uci:set("openvpn", VPN_INST, "client", "1")
		elseif TMODE == "server" then
			m.uci:set("openvpn", VPN_INST, "client_config_dir", "/etc/openvpn/ccd")
		end
	elseif _auth == "pass" or _auth == "tls/pass" then
		if TMODE == "client" then
			m.uci:set("openvpn", VPN_INST, "client", "1")
			m.uci:set("openvpn", VPN_INST, "auth_user_pass", "/etc/openvpn/auth_"..VPN_INST.."")
		else
			m.uci:set("openvpn", VPN_INST, "auth_user_pass_verify", "/etc/openvpn/auth-pam.sh via-file")
			m.uci:set("openvpn", VPN_INST, "script_security", "2")
			m.uci:set("openvpn", VPN_INST, "client_config_dir", "/etc/openvpn/ccd")
			if _auth == "pass" then
				m.uci:set("openvpn", VPN_INST, "client_cert_not_required", "1")
			end
		end
	end
	--m.uci:save("openvpn")
	--m.uci:commit("openvpn")
	--cecho("Exiting on parse.")
end

function m.on_commit(map)

	local auth = m:formvalue("cbid.openvpn." .. VPN_INST .. "._auth")
	if TMODE == "server" then
		local dev = m:formvalue("cbid.openvpn." .. VPN_INST .. ".dev")

		if (auth ~= "skey" or dev ~= "tun") then
			local keepalive = m:formvalue("cbid.openvpn." .. VPN_INST .. "._keepalive_tls") or nil
			if keepalive ~= nil then
				m.uci:set("openvpn", VPN_INST, "keepalive", keepalive)
				m.uci:save("openvpn")
			end
		end
	end

	if auth == "tls" then
		local _tls_cipher_value = m:formvalue("cbid.openvpn." .. VPN_INST .. "._tls_cipher") or "all"
		if _tls_cipher_value=="dhe_rsa" then
			list="TLS-DHE-RSA-WITH-AES-256-GCM-SHA384:TLS-DHE-RSA-WITH-AES-256-CBC-SHA:TLS-DHE-RSA-WITH-CAMELLIA-256-CBC-SHA:TLS-DHE-RSA-WITH-3DES-EDE-CBC-SHA:TLS-DHE-RSA-WITH-AES-128-GCM-SHA256:TLS-DHE-RSA-WITH-AES-128-CBC-SHA:TLS-DHE-RSA-WITH-SEED-CBC-SHA:TLS-DHE-RSA-WITH-CAMELLIA-128-CBC-SHA:TLS-DHE-RSA-WITH-DES-CBC-SHA"
		elseif _tls_cipher_value=="all" then
			list=""
		elseif _tls_cipher_value=="custom" then
			local value = m:formvalue("cbid.openvpn." .. VPN_INST .. "._tls_cipher2")
			if #value > 1 and value[1] ~= nil and value[1] ~= "" then
				list=""
				for i = 1, #value do
					if value[i]~="" then
						if list == "" then
							list=value[i]
						else
							list=list..":"..value[i]
						end
					end
				end
			else
				list=value
			end
		end
		--sys.call("uci set -q openvpn.".. VPN_INST..".tls_cipher=\""..list.."\" ")
		--sys.call("uci commit openvpn")
		m.uci:set("openvpn", VPN_INST, "tls_cipher", list)
		m.uci:commit("openvpn")
	else
		--sys.call("uci set -q openvpn.".. VPN_INST..".tls_cipher=\"\" ")
		m.uci:set("openvpn", VPN_INST, "tls_cipher", "")
	end


	local vpnEnable = m:formvalue("cbid.openvpn." .. VPN_INST .. ".enable")
	-- it's not PID, this function returns fail or success (shell var $? = 0 or 1)
	local vpnPIDstatus = sys.call("pidof openvpn > /dev/null")
	local vpnRunning = false

	if vpnPIDstatus == 0 then
		vpnRunning = true
	end

	if TMODE == "client" and (auth == "tls/pass" or auth == "pass") then
		local username = m:formvalue("cbid.openvpn." .. VPN_INST .. ".user") or nil
		local password = m:formvalue("cbid.openvpn." .. VPN_INST .. ".pass") or nil
		if username and password then
			sys.exec("echo -e \"" .. username .. "\n" .. password .. "\" > "
				.. "/etc/openvpn/auth_" .. VPN_INST .. "")
		end
	elseif TMODE == "server" and (auth == "tls/pass" or auth == "pass") then
		local auth_file = m.uci:get("openvpn", VPN_INST, "userpass") or nil
		if auth_file then
			nixio.fs.copy(auth_file, "/etc/openvpn/auth_" .. VPN_INST)
		end
	else
		sys.exec("rm /etc/openvpn/auth_" .. VPN_INST .. " 2>/dev/null >/dev/null")
	end

	--~ key encryption verification
	if TMODE == "client" then
		local key = m:formvalue("cbid.openvpn." .. VPN_INST .. ".decrypt") or ""
		local encrypted = utl.trim(sys.exec("grep -q 'ENCRYPTED PRIVATE KEY' /lib/uci/upload/cbid.openvpn.".. VPN_INST ..".key; echo $?"))

		if encrypted == "0" and key == "" then
			m.message = translatef("err: Client key is encrypted, please enter decryption password!")
			m.redirect = nil
			m.uci:set("openvpn", VPN_INST, "key_encrypted", "1")
		end

		if key ~= "" then
			sys.call("openssl rsa -in /lib/uci/upload/cbid.openvpn.".. VPN_INST ..".key -out /lib/uci/upload/cbid.openvpn.".. VPN_INST ..".key -passin pass:"..key)
			m.uci:set("openvpn", VPN_INST, "key_encrypted", "0")
			m.uci:delete("openvpn", VPN_INST, "decrypt")
		end

	end
	---------------end---------------

	if vpnEnable then
		--Delete all usr_enable from openvpn config
		m.uci:foreach("openvpn", "openvpn", function(s)
			local usr_enable = s.usr_enable or ""
			local name = s[".name"]:split("_")
			if TMODE == name[1] then
				if usr_enable == "1" then
					open_vpn = s[".name"]
					m.uci:delete("openvpn", open_vpn, "usr_enable")
				end
			end
		end)
		m.uci:save("openvpn")
		m.uci.commit("openvpn")

		if vpnRunning then
			sys.call("/etc/init.d/openvpn restart > /dev/null")
			sys.call("/etc/init.d/firewall restart >/dev/null")
		else
			sys.call("/etc/init.d/openvpn start > /dev/null")
			sys.call("/etc/init.d/firewall restart >/dev/null")
		end
		local archive = m.uci:get("openvpn", VPN_INST, "ccd_archive") or nil
		if archive then
			sys.exec("rm " .. archive)
		end
		m.uci:delete("openvpn", VPN_INST, "ccd_archive")
	else
		sys.call("/etc/init.d/openvpn stop > /dev/null")
	end
end

return m
