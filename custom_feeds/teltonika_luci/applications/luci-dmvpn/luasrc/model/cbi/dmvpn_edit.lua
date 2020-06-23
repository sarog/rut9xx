
local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()
local util = require ("luci.util")

local section_name

if arg[1] then
	section_name = arg[1]
else
	luci.http.redirect(luci.dispatcher.build_url("admin", "services", "vpn", "dmvpn"))
end

-----------------------------------------------------------------------------------------
------------------------------- Global DMVPN --------------------------------------------
-----------------------------------------------------------------------------------------

local dmvpn_map = Map("dmvpn", translate("DMVPN Configuration"), translate(""))
	dmvpn_map.addremove = false
	dmvpn_map.redirect = luci.dispatcher.build_url("admin", "services", "vpn", "dmvpn")

local dmvpn = dmvpn_map:section(NamedSection, section_name, "dmvpn", translate("DMVPN Parameters Configuration"))

local en = dmvpn:option(Flag, "enabled", translate("Enabled"), translate("Enables DMVPN client"))

	  function en.on_enable(self, section)
		  local IPsecESP = dmvpn_map.uci:get("firewall", "IPsecESP", "enabled") or "1"
		  local IPsecNAT = dmvpn_map.uci:get("firewall", "IPsecNAT", "enabled") or "1"
		  local IPsecIKE = dmvpn_map.uci:get("firewall", "IPsecIKE", "enabled") or "1"

		  if IPsecESP ~= "1" or IPsecNAT ~= "1" or IPsecIKE ~= "1" then
			dmvpn_map.uci:set("firewall", "IPsecESP", "enabled", "1")
			dmvpn_map.uci:set("firewall", "IPsecNAT", "enabled", "1")
			dmvpn_map.uci:set("firewall", "IPsecIKE", "enabled", "1")
			dmvpn_map.uci:commit("firewall")
		  end
	  end

	  function en.on_disable(self, section)
		  local IPsecESP = dmvpn_map.uci:get("firewall", "IPsecESP", "enabled") or "1"
		  local IPsecNAT = dmvpn_map.uci:get("firewall", "IPsecNAT", "enabled") or "1"
		  local IPsecIKE = dmvpn_map.uci:get("firewall", "IPsecIKE", "enabled") or "1"

		  if IPsecESP ~= "0" or IPsecNAT ~= "0" or IPsecIKE ~= "0" then
			  local ipsec_enabled = false
			  dmvpn_map.uci:foreach("strongswan", "conn", function(sec)
				  if sec.enabled and sec.enabled == "1" and string.match(sec[".name"], "dmvpn") then
					  ipsec_enabled = true
				  end
			  end)
			  if not ipsec_enabled then
				dmvpn_map.uci:set("firewall", "IPsecESP", "enabled", "0")
				dmvpn_map.uci:set("firewall", "IPsecNAT", "enabled", "0")
				dmvpn_map.uci:set("firewall", "IPsecIKE", "enabled", "0")
				dmvpn_map.uci:commit("firewall")
			  end
		  end
	  end

local config_mode = dmvpn:option(ListValue, "config_mode", translate("Working mode"), translate("Select mode in which this device is going to function as (Spoke or Hub)"))
	config_mode.default = "spoke"
	config_mode:value("spoke", translate("Spoke"))
	config_mode:value("hub", translate("Hub"))

local hub = dmvpn:option(Value, "hub_address", translate("Hub Address"), translate("IP address of DMVPN HUB"))
	hub:depends("config_mode", "spoke")
	hub.datatype = "host"


-----------------------------------------------------------------------------------------
------------------------------- GRE tunnel configration ---------------------------------
-----------------------------------------------------------------------------------------

local interfaces = {
	{ifname="3g-ppp", genName="Mobile", type="3G"},
	{ifname="eth2", genName="Mobile", type="3G"},
	{ifname="usb0", genName="WiMAX", type="WiMAX"},
	{ifname="eth1", genName="Wired", type="vlan"},
	{ifname="wlan0", genName="WiFi", type="wifi"},
	{ifname="none", genName="Mobile bridged", type="3G"},
	{ifname="wwan0", genName="Mobile", type="3G"},
	{ifname="wm0", genName="WiMAX", type="WiMAX"},
	{ifname="wwan-usb0", genName="Mobile USB", type="3G"},
}

local gre_map = Map("network")
	gre_map.addremove = false

local gre = gre_map:section(NamedSection, section_name, "interface", translate("GRE Parameters Configuration"))
	gre.addremove = false

local local_ipaddr = gre:option(Value, "ipaddr_tunlink", translate("Tunnel source"),
	translate("IP address of the local WAN interface."))
	local_ipaddr.combobox_manual = "-- Enter IP address --"

	gre_map.uci:foreach("network", "interface", function(intf)
		for _, known_interface in ipairs(interfaces) do
			if known_interface.ifname == intf.ifname  then
				local modem = gre_map.uci:get("system", "module", "name") or ""
				local ifname
				if modem:find("Quectel") then
					ifname = "ppp_4"
				else
					ifname = intf[".name"]
				end
				if (string.match(known_interface.genName, "Mobile") and not string.match(intf[".name"], "wan")) then
					local_ipaddr:value(ifname, translate(known_interface.genName.." ("..string.upper(intf[".name"])..")"))
				elseif not string.match(known_interface.genName, "Mobile") then
					local_ipaddr:value(intf[".name"], translate(known_interface.genName.." ("..string.upper(intf[".name"])..")"))
				end
			elseif string.match(intf[".name"], "lan_") then
				local_ipaddr:value(intf[".name"], translate("VLAN ("..string.upper(intf[".name"])..")"))
			elseif string.match(intf[".name"], "lan") then
				local_ipaddr:value(intf[".name"], translate("LAN"))
			end
		end
	end)

function local_ipaddr.write(self, section, value)
	-- TODO: no v6 support?
	if string.match(value, "(%d+)%.(%d+)%.(%d+)%.(%d+)") then
		gre_map.uci:set(self.config, section, "ipaddr", value)
		gre_map.uci:delete(self.config, section, "tunlink")
	else
		gre_map.uci:set(self.config, section, "tunlink", value)
		gre_map.uci:delete(self.config, section, "ipaddr")
	end
end

function local_ipaddr.cfgvalue(self, section)
	return gre_map.uci:get(self.config, section, "ipaddr") or gre_map.uci:get(self.config, section, "tunlink")
end

local tunnel_ip = gre:option(Value, "gre_ipaddr", translate("Local GRE interface IP address"), translate("IP address of the local GRE tunnel device."))
	tunnel_ip.datatype = "ip4addr"

	function tunnel_ip.write(self, section, value)
		gre_map.uci:set("network", section_name.."_static", "ipaddr", value)
	end

	function tunnel_ip.cfgvalue(self, section)
		return gre_map.uci:get("network", section_name.."_static", "ipaddr")
	end

local tunnel_mask = gre:option(Value, "netmask", translate("Local GRE interface netmask"), translate("Netmask of the local GRE tunnel device."))
	tunnel_mask:depends({["dmvpn."..section_name..".config_mode"] = "hub"})
	tunnel_mask.datatype = "ip4addr"

	function tunnel_mask.write(self, section, value)
		gre_map.uci:set("network", section_name.."_static", self.option, value)
	end

	function tunnel_mask.cfgvalue(self, section)
		return gre_map.uci:get("network", section_name.."_static", "netmask") or ""
	end

local remote_tunnel_ip = gre:option(Value, "gre_remote_ipaddr", translate("Remote GRE interface IP address"),translate("IP address of the remote GRE tunnel device."))
	remote_tunnel_ip:depends({["dmvpn."..section_name..".config_mode"] = "spoke"})
	remote_tunnel_ip.datatype ="ip4addr"

	function remote_tunnel_ip.write(self, section, value)
		gre_map.uci:set("quagga", section_name.."_dmvpn", "proto_address", value)
		gre_map.uci:save("quagga")
	end

	function remote_tunnel_ip.cfgvalue(self, section)
		return uci:get("quagga", section_name.."_dmvpn", "proto_address")
	end

local gre_mtu = gre:option(Value, "mtu", translate("GRE MTU"), translate("MTU size of GRE Tunnel device"))

local gre_secrets = gre:option(Value, "gre_secrets", translate("GRE keys"), translate("Key for incoming/outgoing packets"))
	gre_secrets.datatype = "and(range(0, 65535), lengthvalidation(0,5,'^[0-9]+$'))"

	function gre_secrets.write(self, section, value)
		gre_map.uci:set(self.config, section, "ikey", value)
		gre_map.uci:set(self.config, section, "okey", value)
	end

	function gre_secrets.cfgvalue(self, section)
		local tmp = gre_map.uci:get(self.config, section, "ikey") or ""
		if tmp and tmp ~= "" then
			return tmp
		else
			return gre_map.uci:get(self.config, section, "okey")
		end
	end

-----------------------------------------------------------------------------------------
------------------------------- IPsec transport configration ----------------------------
-----------------------------------------------------------------------------------------

local ipsec_map = Map("strongswan")
	ipsec_map.addremove = false

local ipsec_main = ipsec_map:section(NamedSection, section_name.."_dmvpn", "conn", translate("IPsec Parameters Configuration"))

local negotiation = ipsec_main:option(ListValue, "aggressive", translate("Negotiation Mode"), translate("ISAKMP (Internet Security Association and Key Management Protocol) phase 1 exchange mode"))
	negotiation.default = "no"
	negotiation:value("no",translate("Main"))
	negotiation:value("yes",translate("Aggressive"))

local iden  = ipsec_main:option( ListValue, "my_identifier_type", translate("My identifier type"), translate("Choose one accordingly to your IPSec configuration"))
	iden.nowrite = true
	iden.default = "fqdn"
	iden:value("fqdn", translate("FQDN"))
	iden:value("user_fqdn", translate("User FQDN"))
	iden:value("address", translate("Address"))

local id = ipsec_main:option( Value, "my_identifier", translate("My identifier"), translate("Set the device identifier for IPSec tunnel"))

	function id.validate(self, value, section)
		if iden:formvalue(section) == "address" then
			if not value:match("[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+") then
				ipsec_map.message = translate("IP address expected in \"My identifier\" field")
				return nil
			end
		end
		return value
	end

local ipsec = ipsec_map:section( TypedSection, "p1_proposal", translate(""))
	ipsec.anonymous = true
	ipsec.cfgsections = function (self)
		return {"p1_proposal"}
	end

	ipsec:tab("phase1",  translate("Phase 1"))
	ipsec:tab("phase2", translate("Phase 2"))

local ike_ea = ipsec:taboption("phase1", ListValue, "ike_encryption_algorithm", translate("Encryption algorithm"), translate("The encryption algorithm must match with another incoming connection to establish IPSec"))
    ike_ea.default = "3des"
    ike_ea:value("des","DES")
    ike_ea:value("3des","3DES")
    ike_ea:value("aes128","AES 128")
    ike_ea:value("aes192","AES 192")
    ike_ea:value("aes256","AES 256")

	function ike_ea.write(self, section, value)
		self.map:set(section_name.."_dmvpn", self.option, value)
	end

	function ike_ea.cfgvalue(self, section)
		return self.map:get(section_name.."_dmvpn", self.option)
	end

local ike_aa = ipsec:taboption("phase1", ListValue, "ike_authentication_algorithm", translate("Authentication"), translate("The authentication algorithm must match with another incoming connection to establish IPSec"))
	ike_aa.default = "sha1"
	ike_aa:value("md5", "MD5")
	ike_aa:value("sha1", "SHA1")
	ike_aa:value("sha256", "SHA256")
	ike_aa:value("sha384", "SHA384")
	ike_aa:value("sha512", "SHA512")

	function ike_aa.write(self, section, value)
		self.map:set(section_name.."_dmvpn", self.option, value)
	end

	function ike_aa.cfgvalue(self, section)
		return self.map:get(section_name.."_dmvpn", self.option)
	end

local ike_dh_group = ipsec:taboption("phase1", ListValue, "ike_dh_group", translate("DH group"), translate("The DH (Diffie-Hellman) group must match with another incoming connection to establish IPSec"))
	ike_dh_group.default = "modp1536"
	ike_dh_group:value("modp768", "MODP768")
	ike_dh_group:value("modp1024", "MODP1024")
	ike_dh_group:value("modp1536", "MODP1536")
	ike_dh_group:value("modp2048", "MODP2048")
	ike_dh_group:value("modp3072", "MODP3072")
	ike_dh_group:value("modp4096", "MODP4096")

	function ike_dh_group.write(self, section, value)
		self.map:set(section_name.."_dmvpn", self.option, value)
	end

	function ike_dh_group.cfgvalue(self, section)
		return self.map:get(section_name.."_dmvpn", self.option)
	end

local ike_keylife = ipsec:taboption("phase1", Value, "ikelifetime", translate("Lifetime (h)"), translate("The time duration for phase 1"))
	ike_keylife.datatype = "lengthvalidation(0,64,'^[0-9]+$')"
	ike_keylife.displayInline = true
	ike_keylife.forcewrite = true
	ike_keylife.default = "8"

	function ike_keylife.write(self, section, value)
		local letter = ike_time:formvalue(section)
		if letter then
			value = string.format("%s%s", value, letter)
		end

		ipsec_map.uci:set(self.config, section_name.."_dmvpn", self.option, value)
	end

	function ike_keylife.cfgvalue(self, section)
		local value = uci:get(self.config, section_name.."_dmvpn", self.option)

		if value then
			value = value:match("%d+")
		end

		return value
	end

ike_time = ipsec:taboption("phase1", ListValue, "ikeletter", translate(""), translate(""))
	ike_time:value("h", translate("Hours"))
	ike_time:value("m", translate("Minutes"))
	ike_time:value("s", translate("Seconds"))
	ike_time.displayInline = true
	ike_time.default = "h"

	function ike_time.write(self, section, value)
		self.map:set(section_name.."_dmvpn", self.option, value)
	end

	function ike_time.cfgvalue(self, section)
		local value = uci:get(self.config, section_name.."_dmvpn", self.option)

		if value then
			value = value:match("%a+")
		end

		return  value
	end

local esp_ea = ipsec:taboption("phase2", ListValue, "esp_encryption_algorithm", translate("Encryption algorithm"), translate("The encryption algorithm must match with another incoming connection to establish IPSec"))
	esp_ea.default = "3des"
	esp_ea:value("des","DES")
	esp_ea:value("3des","3DES")
	esp_ea:value("aes128","AES 128")
	esp_ea:value("aes192","AES 192")
	esp_ea:value("aes256","AES 256")

	function esp_ea.write(self, section, value)
		self.map:set(section_name.."_dmvpn", self.option, value)
	end

	function esp_ea.cfgvalue(self, section)
		return self.map:get(section_name.."_dmvpn", self.option)
	end

local esp_ha = ipsec:taboption("phase2", ListValue, "esp_hash_algorithm", translate("Hash algorithm"), translate("The hash algorithm must match with another incoming connection to establish IPSec"))
	esp_ha.default = "sha1"
	esp_ha:value("md5", "MD5")
	esp_ha:value("sha1", "SHA1")
	esp_ha:value("sha256", "SHA256")
	esp_ha:value("sha384", "SHA384")
	esp_ha:value("sha512", "SHA512")

	function esp_ha.write(self, section, value)
		self.map:set(section_name.."_dmvpn", self.option, value)
	end

	function esp_ha.cfgvalue(self, section)
		return self.map:get(section_name.."_dmvpn", self.option)
	end


local pfs_group = ipsec:taboption("phase2", ListValue, "esp_pfs_group", translate("PFS group"), translate("The PFS (Perfect Forward Secrecy) group must match with another incoming connection to establish IPSec"))
	pfs_group.default = "modp1536"
	pfs_group:value("modp768", "MODP768")
	pfs_group:value("modp1024", "MODP1024")
	pfs_group:value("modp1536", "MODP1536")
	pfs_group:value("modp2048", "MODP2048")
	pfs_group:value("modp3072", "MODP3072")
	pfs_group:value("modp4096", "MODP4096")
	pfs_group:value("no_pfs", "No PFS")

	function pfs_group.write(self, section, value)
		self.map:set(section_name.."_dmvpn", self.option, value)
	end

	function pfs_group.cfgvalue(self, section)
		return self.map:get(section_name.."_dmvpn", self.option)
	end

keylife = ipsec:taboption("phase2", Value, "keylife", translate("Lifetime (h)"), translate("The time duration for phase 2"))
	keylife.datatype = "lengthvalidation(0,64,'^[0-9]+$')"
	keylife.displayInline = true
	keylife.forcewrite = true
	keylife.default = "8"

	function keylife.write(self, section, value)
		local letter = time:formvalue(section)

		if letter then
			value = string.format("%s%s", value, letter)
		end

		ipsec_map.uci:set(self.config, section_name.."_dmvpn", self.option, value)
	end

	function keylife.cfgvalue(self, section)
		local value = uci:get(self.config, section_name.."_dmvpn", self.option)

		if value then
			value = value:match("%d+")
		end

		return  value
	end

time = ipsec:taboption("phase2", ListValue, "letter", translate(""), translate(""))
	time:value("h", translate("Hours"))
	time:value("m", translate("Minutes"))
	time:value("s", translate("Seconds"))
	time.displayInline = true
	time.default = "h"

	function time.write(self, section, value)
		self.map:set(section_name.."_dmvpn", self.option, value)
	end

	function time.cfgvalue(self, section)
		local value = uci:get(self.config, section_name.."_dmvpn", self.option)

		if value then
			value = value:match("%a+")
		end

		return value
	end

local keys = ipsec_map:section(TypedSection, "preshared_keys", translate("IPsec Pre-shared Keys"), translate(""))
	keys.addremove = true
	keys.anonymous = true
	keys.template  = "cbi/tblsection"
	keys.novaluetext = translate("There are no preshared keys created yet")

local psk = keys:option( Value, "psk_key", translate("Pre-shared key"), translate("A shared password to authenticate between the peers. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
	psk.password = true
	psk.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~.-]+$',5)"

local id_select = keys:option(DynamicList, "id_selector", translate("Secret's ID selector"), translate("Each secret can be preceded by a list of optional ID selectors. A selector is an IP address, a Fully Qualified Domain Name, user@FQDN or %any. When using IKEv1 use IP address"))
	id_select.placeholder = "%any, IP or FQDN"

-----------------------------------------------------------------------------------------
------------------------------- NHRP attributes configration ----------------------------
-----------------------------------------------------------------------------------------

local nhrp_map = Map("quagga", translate(""), translate(""))
	nhrp_map.addremove = false

local nhrp = nhrp_map:section(NamedSection, section_name.."_dmvpn", "nhrp_instance", translate("NHRP Parameters Configuration"))

local nhrp_net_id = nhrp:option(Value, "network_id", translate("NHRP network ID"), translate("NHRP network identifier"))
	nhrp_net_id.default = "1"

local nhrp_hldtm = nhrp:option(Value, "holdtime", translate("NHRP hold time"), translate("Specifies the holding time for NHRP Registration Requests and Resolution Replies sent from this interface or shortcut-target. The holdtime is specified in seconds and defaults to two hours."))
	nhrp_hldtm.default = "7200"

-----------------------------------------------------------------------------------------
---------------------------- Various options setup before save --------------------------
-----------------------------------------------------------------------------------------

function gre_map.on_before_save(self)
	local is_enabled = dmvpn_map:get(section_name, "enabled")
	local work_mode = luci.http.formvalue("cbid.dmvpn."..section_name..".config_mode") or ""
	if is_enabled == "1" then
		gre_map.uci:set("network", section_name, "disabled", "0")
		gre_map.uci:set("network", section_name, "auto", "1")
		if work_mode == "spoke" then
			gre_map.uci:set("network", section_name, "peeraddr",
				luci.http.formvalue("cbid.dmvpn."..section_name..".hub_address"))
		elseif work_mode == "hub" then
			gre_map.uci:set("network", section_name, "peeraddr", "0.0.0.0")
		end

		if work_mode == "spoke" then
			gre_map.uci:set("network", section_name.."_static", "netmask", "255.255.255.255")
			gre_map.uci:set("network", section_name.."_dmvpn_route", "route")
			gre_map.uci:set("network", section_name.."_dmvpn_route", "dep", section_name)
			gre_map.uci:set("network", section_name.."_dmvpn_route", "interface", section_name)
			gre_map.uci:set("network", section_name.."_dmvpn_route", "table", "main")
			gre_map.uci:set("network", section_name.."_dmvpn_route", "target",
				luci.http.formvalue("cbid.network."..section_name..".gre_remote_ipaddr"))
			gre_map.uci:set("network", section_name.."_dmvpn_route", "netmask", "255.255.255.255")
		elseif work_mode == "hub" then
			local route_exists = gre_map.uci:get("network", section_name.."_dmvpn_route")
			if route_exists ~= nil or route_exists ~= "" then
				gre_map.uci:delete("network", section_name.."_dmvpn_route")
			end
		end
	else
		gre_map.uci:set("network", section_name, "disabled", "1")
		gre_map.uci:set("network", section_name, "auto", "0")
	end
end

function ipsec_map.on_before_save(self)
	local is_enabled = dmvpn_map:get(section_name, "enabled")

	if is_enabled == "1" then
		local work_mode = luci.http.formvalue("cbid.dmvpn."..section_name..".config_mode") or ""

		ipsec_map.uci:set("strongswan", section_name.."_dmvpn", "enabled", "1")
		if work_mode == "spoke" then
			ipsec_map.uci:set("strongswan", section_name.."_dmvpn", "right",
				luci.http.formvalue("cbid.dmvpn."..section_name..".hub_address"))
		else
			ipsec_map.uci:delete("strongswan", section_name.."_dmvpn", "right")
		end
	else
		ipsec_map.uci:set("strongswan", section_name.."_dmvpn", "enabled", "0")
	end
end

function explode(delimiter, text)
	local text_arr, arr_length
	text_arr={}
	arr_length=0
	if(#text == 1) then
		return {text}
	end
	while true do
		l = string.find(text, delimiter, arr_length, true)
		if l ~= nil then
			table.insert(text_arr, string.sub(text, arr_length, l - 1))
			arr_length = l + 1
		else
			table.insert(text_arr, string.sub(text, arr_length))
			break
		end
	end
	return text_arr
end

function nhrp_map.on_before_save(self)
	local is_enabled = dmvpn_map:get(section_name, "enabled")
	if is_enabled == "1" then
		nhrp_map.uci:set("quagga", "nhrp", "enabled", "1")
		nhrp_map.uci:set("quagga", section_name.."_dmvpn", "enabled", "1")
		local local_hub_address, ifname_selected, work_mode = "", "", ""
		local tunnellink_name_selected = gre_map:get(section_name, "tunlink")
		if tunnellink_name_selected then
			local exploded = explode("_", tunnellink_name_selected)
			local newtunlink = exploded[1]

			for _, ex in ipairs(exploded) do
				if nhrp_map.uci:get("network", newtunlink, "proto") then
					tunnellink_name_selected = newtunlink
					break
				else
					newtunlink = newtunlink .. "_".. ex
				end
			end

			ifname_selected = uci:get("network", tunnellink_name_selected, "ifname")
			work_mode = luci.http.formvalue("cbid.dmvpn."..section_name..".config_mode") or ""

			local ntm = require "luci.model.network".init()
			local data = { ipaddrs = { } }

			for _, interface in ipairs(ntm.get_interfaces()) do
				for i, a in ipairs(interface:ipaddrs()) do
					data.ipaddrs[#data.ipaddrs+1] = {
						addr      = a:host():string()
					}
					if interface.ifname == ifname_selected then
						local_hub_address = data.ipaddrs[#data.ipaddrs].addr
					end
				end
			end
		end

		if work_mode == "spoke" then
			nhrp_map.uci:set("quagga", section_name.."_dmvpn", "nbma_address", luci.http.formvalue("cbid.dmvpn."..section_name..".hub_address"))
		elseif work_mode == "hub" then
			nhrp_map.uci:set("quagga", section_name.."_dmvpn", "proto_address", "dynamic")
			nhrp_map.uci:set("quagga", section_name.."_dmvpn", "nbma_address", local_hub_address)
		end
	else
		nhrp_map.uci:set("quagga", section_name.."_dmvpn", "enabled", "0")
	end
end

return dmvpn_map, gre_map, ipsec_map, nhrp_map
