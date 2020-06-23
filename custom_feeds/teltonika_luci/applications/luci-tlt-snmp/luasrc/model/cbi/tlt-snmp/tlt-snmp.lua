local m, agent, sys,  o, port, remote, deathtrap = false, enable, com, comname, snmp_ver, butt

local uci = luci.model.uci.cursor()
local fw = require "luci.model.firewall"
local fs = require "nixio.fs"
local sys = require "luci.sys"
fw.init(uci)

local __define__rule_name = "SNMP_WAN_Access"
local _define_snmp_cfg = "snmpd"
------
-- DBG
------
local function cecho(string)
	luci.sys.call("echo \"" .. string .. "\" >> /tmp/log.log")
end

m = Map("snmpd", translate("SNMP Configuration"))
m:chain("firewall")

agent = m:section(TypedSection, "agent", translate("SNMP Service Settings"))
agent.addremove = false
agent.anonymous = true

mib = agent:option(Button, "_download")
mib.title      = translate("MIB file")
mib.inputtitle = translate("Download")
mib.inputstyle = "apply"
mib.timeload = true
mib.onclick = true
mib.template = "admin_system/dwbutton"

-----------------
-- enable/disable
-----------------
o = agent:option(Flag, "enabled", translate("Enable SNMP service"), translate("Run SNMP (Simple Network Management Protocol) service on system\\'s startup"))
o.forcewrite = true
o.rmempty = false
-----------------------
-- enable remote access
-----------------------
remote = agent:option(Flag, "remoteAccess", translate("Enable remote access"), translate("Open port in firewall so that SNMP (Simple Network Management Protocol) service may be reached from WAN"))
remote.forcewrite = true
remote.rmempty = false

-------
-- port
-------
port = agent:option(Value, "portNumber", translate("Port"), translate("SNMP (Simple Network Management Protocol) service\\'s port"))
port.default = "161"
port.datatype = "port"

function port.validate(self, value, section)
	def = m.uci:get("snmpd", "teltonika_auth_service", "portNumber")
-- 	luci.sys.call("echo \"def [".. def .."]\" \"value [".. value .."]\"  >>/tmp/aaa")
	if def ~= value then
		return value
	else
-- 		m.message = translate("This port using for monitoring. Please choose another one!")
		return nil, translate("This port is used for monitoring. Please choose another one!")
-- 		return m.message
	end
end

--
-- community
--
com = agent:option(ListValue, "_community", translate("Community"), translate("The SNMP (Simple Network Management Protocol) Community is an ID that allows access to a router\\'s SNMP data"))
com:value("public", translate("Public"))
com:value("private", translate("Private"))
com:value("custom", translate("Custom"))
com.default = "public"

comname = agent:option(Value, "_community_name", translate("Community name"), translate("Set custom name to access SNMP"))
comname:depends("_community", "custom")
comname.default = "custom"

src = agent:option(Value, "_src", translate("IP or Network"), translate("192.168.1.1 or 192.168.1.0/24"))
src:depends("_community", "private")
src.default = "127.0.0.1"
src.datatype = "ip4addr"

-- sys = m:section(TypedSection, "system", translate("SNMP Configuration Settings"))
-- sys.addremove = false
-- sys.anonymous = true
-- 
-- o = sys:option(Value, "sysLocation", translate("Location"))
-- o = sys:option(Value, "sysContact", translate("Contact"))
-- o = sys:option(Value, "sysName", translate("Name"))

loc = agent:option(Value, "sysLocation", translate("Location"), translate("Trap named sysLocation"))
	function loc.cfgvalue(self, section)
		return luci.sys.exec("uci get snmpd.@system[0].sysLocation")
	end
	function loc.write(self, section, value)
		luci.sys.call("uci set snmpd.@system[0].sysLocation="..value.."; uci commit snmpd")
	end  
	  
con = agent:option(Value, "sysContact", translate("Contact"), translate("Trap named sysContact"))
	function con.cfgvalue(self, section)
		return luci.sys.exec("uci get snmpd.@system[0].sysContact")
	end
	function con.write(self, section, value)
		luci.sys.call("uci set snmpd.@system[0].sysContact="..value.."; uci commit snmpd")
	end
	
nam = agent:option(Value, "sysName", translate("Name"), translate("Trap named sysName"))
	function nam.cfgvalue(self, section)
		return luci.sys.exec("uci get snmpd.@system[0].sysName")
	end
	function nam.write(self, section, value)
		luci.sys.call("uci set snmpd.@system[0].sysName="..value.."; uci commit snmpd")
	end

snmp_ver = agent:option(ListValue, "version", translate("SNMP version"), translate("The SNMP version to be used"))
snmp_ver:value("v1/v2", translate("v1/v2"))
snmp_ver:value("v1/v2/v3", translate("v1/v2/v3"))
snmp_ver:value("v3", translate("v3"))
snmp_ver.default = "v1/v2"

username = agent:option(Value, "user_name", translate("Username"), translate("Set username to access SNMP"))
username:depends("version", "v1/v2/v3")
username:depends("version", "v3")
username.default = "username"

auth_type = agent:option(ListValue, "auth_type", translate("Authentication type"), translate("Set authentication type to use with SNMPv3"))
auth_type:depends("version", "v1/v2/v3")
auth_type:depends("version", "v3")
auth_type:value("SHA", translate("SHA"))
auth_type:value("MD5", translate("MD5"))
auth_type.default = "SHA"


auth_pass = agent:option(Value, "auth_pass", translate("Authentication passphrase"), translate("Set authentication passpharse to generate key for SNMPv3"))
auth_pass:depends("version", "v1/v2/v3")
auth_pass:depends("version", "v3")
auth_pass.default = "passphrase"
auth_pass.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',8)"

encryption_type = agent:option(ListValue, "encryption_type", translate("Encryption type"), translate("Set encryption type to use with SNMPv3"))
encryption_type:depends("version", "v1/v2/v3")
encryption_type:depends("version", "v3")
encryption_type:value("AES", translate("AES"))
encryption_type:value("DES", translate("DES"))
encryption_type.default = "AES"

encryption_pass = agent:option(Value, "encryption_pass", translate("Encryption passphrase"), translate("Set encryption passpharse to generate key for SNMPv3"))
encryption_pass:depends("version", "v1/v2/v3")
encryption_pass:depends("version", "v3")
encryption_pass.default = "passphrase"
encryption_pass.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',8)"

function o.write(self, section, value)
	Value.write(self, section, value)

	----------------------------------------------------------------------------
	-- community option
	----------------------------------------------------------------------------
	local stateNow = com:formvalue(section)
	local statePreviuos
	local nameNow, namePreviuos
	local needUpdate = false
	local commonName
	local snmp_version = snmp_ver:formvalue(section)
	local snmp_version_prev

	uci:foreach(_define_snmp_cfg, "agent",
		function(s)
			statePreviuos = s._community
			namePreviuos = s._community_name
			srcPreviuos = s._src
			snmp_version_prev = s.version
		end)

	-- check if there are changes
	if stateNow ~= statePreviuos then 
		needUpdate = true 
	end
	if stateNow == "custom" then
		nameNow = comname:formvalue(section)
		if nameNow ~= namePreviuos then needUpdate = true end
	end
	
	if stateNow == "private" then
		srcNow = src:formvalue(section)
		if srcNow ~= srcPreviuos then needUpdate = true end
	end

	if snmp_version ~= snmp_version_prev then
		needUpdate = true
	end

	if needUpdate then
		if statePreviuos ~= "custom" then commonName = statePreviuos
		else commonName = namePreviuos end
		
		-- delete old sections
		uci:delete(_define_snmp_cfg, commonName)
		uci:delete(_define_snmp_cfg, commonName .. "_v1")
		uci:delete(_define_snmp_cfg, commonName .. "_v2c")
		uci:delete(_define_snmp_cfg, commonName .. "_usm")
		uci:delete(_define_snmp_cfg, commonName .. "_access")
		
		-- create new sections
		if stateNow == "public" then
			uci:set(_define_snmp_cfg, stateNow, "com2sec") -- new section
			uci:set(_define_snmp_cfg, stateNow, "secname", "ro")
			uci:set(_define_snmp_cfg, stateNow, "source", "default")
			uci:set(_define_snmp_cfg, stateNow, "community", stateNow)
			
			uci:set(_define_snmp_cfg, stateNow .. "_v1", "group") -- new section
			uci:set(_define_snmp_cfg, stateNow .. "_v1", "group", stateNow)
			uci:set(_define_snmp_cfg, stateNow .. "_v1", "version", "v1")
			uci:set(_define_snmp_cfg, stateNow .. "_v1", "secname", "ro")
			
			uci:set(_define_snmp_cfg, stateNow .. "_v2c", "group") -- new section
			uci:set(_define_snmp_cfg, stateNow .. "_v2c", "group", stateNow)
			uci:set(_define_snmp_cfg, stateNow .. "_v2c", "version", "v2c")
			uci:set(_define_snmp_cfg, stateNow .. "_v2c", "secname", "ro")

			uci:set(_define_snmp_cfg, stateNow .. "_usm", "group") -- new section
			uci:set(_define_snmp_cfg, stateNow .. "_usm", "group", stateNow)
			uci:set(_define_snmp_cfg, stateNow .. "_usm", "version", "usm")
			uci:set(_define_snmp_cfg, stateNow .. "_usm", "secname", "ro")
		
			uci:set(_define_snmp_cfg, stateNow .. "_access", "access") -- new section
			uci:set(_define_snmp_cfg, stateNow .. "_access", "group", stateNow)
			uci:set(_define_snmp_cfg, stateNow .. "_access", "context", "none")
			uci:set(_define_snmp_cfg, stateNow .. "_access", "version", "any")
			if snmp_version == "v3" then
				uci:set(_define_snmp_cfg, stateNow .. "_access", "level", "priv")
			else
				uci:set(_define_snmp_cfg, stateNow .. "_access", "level", "noauth")
			end
			uci:set(_define_snmp_cfg, stateNow .. "_access", "prefix", "exact")
			uci:set(_define_snmp_cfg, stateNow .. "_access", "read", "all")
			uci:set(_define_snmp_cfg, stateNow .. "_access", "write", "none")
			uci:set(_define_snmp_cfg, stateNow .. "_access", "notify", "none")
		elseif stateNow == "private" then
			uci:set(_define_snmp_cfg, stateNow, "com2sec") -- new section
			uci:set(_define_snmp_cfg, stateNow, "secname", "rw")
			uci:set(_define_snmp_cfg, stateNow, "source", srcNow)
			uci:set(_define_snmp_cfg, stateNow, "community", stateNow)
			
			uci:set(_define_snmp_cfg, stateNow .. "_v1", "group") -- new section
			uci:set(_define_snmp_cfg, stateNow .. "_v1", "group", stateNow)
			uci:set(_define_snmp_cfg, stateNow .. "_v1", "version", "v1")
			uci:set(_define_snmp_cfg, stateNow .. "_v1", "secname", "rw")
			
			uci:set(_define_snmp_cfg, stateNow .. "_v2c", "group") -- new section
			uci:set(_define_snmp_cfg, stateNow .. "_v2c", "group", stateNow)
			uci:set(_define_snmp_cfg, stateNow .. "_v2c", "version", "v2c")
			uci:set(_define_snmp_cfg, stateNow .. "_v2c", "secname", "rw")

			uci:set(_define_snmp_cfg, stateNow .. "_usm", "group") -- new section
			uci:set(_define_snmp_cfg, stateNow .. "_usm", "group", stateNow)
			uci:set(_define_snmp_cfg, stateNow .. "_usm", "version", "usm")
			uci:set(_define_snmp_cfg, stateNow .. "_usm", "secname", "rw")
		
			uci:set(_define_snmp_cfg, stateNow .. "_access", "access") -- new section
			uci:set(_define_snmp_cfg, stateNow .. "_access", "group", stateNow)
			uci:set(_define_snmp_cfg, stateNow .. "_access", "context", "none")
			uci:set(_define_snmp_cfg, stateNow .. "_access", "version", "any")
			if snmp_version == "v3" then
				uci:set(_define_snmp_cfg, stateNow .. "_access", "level", "priv")
			else
				uci:set(_define_snmp_cfg, stateNow .. "_access", "level", "noauth")
			end
			uci:set(_define_snmp_cfg, stateNow .. "_access", "prefix", "exact")
			uci:set(_define_snmp_cfg, stateNow .. "_access", "read", "all")
			uci:set(_define_snmp_cfg, stateNow .. "_access", "write", "all")
			uci:set(_define_snmp_cfg, stateNow .. "_access", "notify", "all")
		elseif stateNow == "custom" then
			uci:set(_define_snmp_cfg, nameNow, "com2sec") -- new section
			uci:set(_define_snmp_cfg, nameNow, "secname", "rw")
			uci:set(_define_snmp_cfg, nameNow, "source", "default")
			uci:set(_define_snmp_cfg, nameNow, "community", nameNow)
			
			uci:set(_define_snmp_cfg, nameNow .. "_v1", "group") -- new section
			uci:set(_define_snmp_cfg, nameNow .. "_v1", "group", nameNow)
			uci:set(_define_snmp_cfg, nameNow .. "_v1", "version", "v1")
			uci:set(_define_snmp_cfg, nameNow .. "_v1", "secname", "rw")
			
			uci:set(_define_snmp_cfg, nameNow .. "_v2c", "group") -- new section
			uci:set(_define_snmp_cfg, nameNow .. "_v2c", "group", nameNow)
			uci:set(_define_snmp_cfg, nameNow .. "_v2c", "version", "v2c")
			uci:set(_define_snmp_cfg, nameNow .. "_v2c", "secname", "rw")

			uci:set(_define_snmp_cfg, nameNow .. "_usm", "group") -- new section
			uci:set(_define_snmp_cfg, nameNow .. "_usm", "group", nameNow)
			uci:set(_define_snmp_cfg, nameNow .. "_usm", "version", "usm")
			uci:set(_define_snmp_cfg, nameNow .. "_usm", "secname", "rw")
		
			uci:set(_define_snmp_cfg, nameNow .. "_access", "access") -- new section
			uci:set(_define_snmp_cfg, nameNow .. "_access", "group", nameNow)
			uci:set(_define_snmp_cfg, nameNow .. "_access", "context", "none")
			uci:set(_define_snmp_cfg, nameNow .. "_access", "version", "any")
			if snmp_version == "v3" then
				uci:set(_define_snmp_cfg, nameNow .. "_access", "level", "priv")
			else
				uci:set(_define_snmp_cfg, nameNow .. "_access", "level", "noauth")
			end
			uci:set(_define_snmp_cfg, nameNow .. "_access", "prefix", "exact")
			uci:set(_define_snmp_cfg, nameNow .. "_access", "read", "all")
			uci:set(_define_snmp_cfg, nameNow .. "_access", "write", "all")
			uci:set(_define_snmp_cfg, nameNow .. "_access", "notify", "all")
		end	
		uci:save(_define_snmp_cfg)
	end	
		------------------------------------------------------------------------
		-- other options
		------------------------------------------------------------------------
		local remoteEnable = remote:formvalue(section)
		local openPort = port:formvalue(section)
		local needsUpdate = false
		local remoteEnableFix
		
		local fwRuleInstName
		local fwRuleEnabled
		local fwRulePort
		local fwRuleFound = false
		

		if not openPort or openPort == "" then
			m.message = translate("err: Please specify SNMP port!")
			return
		end
		
		-- Double execution prevention
		if not deathtrap then deathtrap = true else return end
		
		-- scan existing rules
		uci:foreach("firewall", "rule", function(s)
			if s.name == __define__rule_name then
				fwRuleInstName = s[".name"]
				fwRuleEnabled = s.enable
				fwRulePort = s.dest_port
				fwRuleFound = true
			end
		end)
		
		-- update values if rule exists
		if fwRuleFound then
			-- fix incompatibility
			if remoteEnable == "1" then remoteEnableFix = "" else remoteEnableFix = "0" end
			
			if openPort ~= fwRulePort then
				uci:set("firewall", fwRuleInstName, "dest_port", openPort)
				needsUpdate = true
			end	
			if remoteEnableFix ~= fwRuleEnabled then
				if remoteEnable == "1" then
					uci:delete("firewall", fwRuleInstName, "enabled")
					needsUpdate = true
				elseif remoteEnable == "0" or remoteEnable == nil then
					uci:set("firewall", fwRuleInstName, "enabled", "0")
					needsUpdate = true
				end
			end
		end
		
		
		
		if not fwRuleFound then
			local wanZone = fw:get_zone("wan")
			if not wanZone then
				m.message = translate("err: Error: could not add firewall rule!")
				return
			end
			
			-- fix incompatibility issue
			local enableFlagFix = ""
			if remoteEnable == "0" or remoteEnable == nil then enableFlagFix = "0" end
			 
			local options = {
				target 		= "ACCEPT",
				proto 		= "udp",
				dest_port 	= openPort,
				name 		= __define__rule_name,
				enabled		= enableFlagFix
			}
			wanZone:add_rule(options)
			needsUpdate = true
		end		
		
		if needsUpdate == true then
			uci:save("firewall")
			uci:commit("firewall")
		end
		
		-- duplicate port entry. This value is used by snmpd
		uci:set("snmpd", section, "agentaddress", "UDP:" .. openPort)
		uci:save(_define_snmp_cfg)
		uci:commit(_define_snmp_cfg)
end

if m:formvalue("cbid.snmpd.cfg018054._download") then
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

			return 0
		end

		if fileExists("/usr/opt/snmp/mibs/", "TLT-MIB.txt") == 1 then
			luci.http.redirect(luci.dispatcher.build_url("admin/services/snmp/mib_download"))
		end
end

return m
