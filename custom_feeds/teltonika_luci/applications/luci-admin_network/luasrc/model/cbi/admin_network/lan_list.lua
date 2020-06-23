
local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()
local util = require ("luci.util")

local CFG_MAP = "network"
local CFG_SEC = "interface"

local m ,s, o

m = Map(CFG_MAP, translate("LAN"))
m.spec_dir = nil
m.pageaction = false


s = m:section( TypedSection, CFG_SEC, translate("LAN Networks List"), translate("") )
s.addremove = true
s.addremove_except = 1
s.anonymous = true
s.template = "cbi/tblsection"
s.template_addremove = "admin_network/add_lan"
s.addremoveAdd = true
s.novaluetext = translate("There are no LAN network yet")

uci:foreach(CFG_MAP, CFG_SEC, function(sec)
	-- Entry signifies that there already is a section, therefore we will disable the ability to add or remove another section
	s.addremoveAdd = false
end)

function s.validate(self, value)
	s = string.sub(value, 1, 3)
	if s ~= "lan" then
		return nil
	end
	return value
end



s.extedit = luci.dispatcher.build_url("admin", "network", "vlan", "lan", "%s", "vlan")

local name = s:option( DummyValue, "_name", translate("LAN name"), translate(""))
function name.cfgvalue(self, section)
	return section:gsub("^%l", string.upper) or "Unknown"
end

local ifname = s:option( DummyValue, "ifname", translate("Interface name"), translate(""))


-------------
function s.parse(self, section)
	local cfgname = luci.http.formvalue("cbid." .. self.config .. "." .. self.sectiontype .. ".name") or ""
	local webrole = "lan_"
	local delButtonFormString = "cbi.rts." .. self.config .. "."
	local delButtonPress = false
	local configName
-- 	local uFound
	local existname = false
	uci:foreach("network", "interface", function(x)
		if not delButtonPress then
			configName = x[".name"] or ""
			if luci.http.formvalue(delButtonFormString .. configName) then
				delButtonPress = true
			end
		end

		newname= webrole..cfgname
		if configName == newname then
			existname = true
		end
	end)

	if delButtonPress then
		uci:foreach("dhcp", "dhcp", function(sec)
			if sec.interface == configName then
				uci:delete("dhcp", sec[".name"])
			end
		end	)

		uci:foreach("firewall", "zone", function(sec)
			if sec.name == configName then
				uci:delete("firewall", sec[".name"])
			end
		end	)

		uci:save("network")
		uci:commit("network")
		uci:save("firewall")
		uci:commit("firewall")
		uci:save("dhcp")
		uci:commit("dhcp")

		-- delete buttons is pressed, don't execute function 'openvpn_new'
		cfgname = false
 		luci.sys.call("/sbin/luci-reload >/dev/null 2>&1 &")
	end

	if cfgname and cfgname ~= '' then
		lan_new(self, cfgname, existname)
	end
	TypedSection.parse( self, section )
	uci:commit("network")
end

function lan_new(self,name, exist)
	local t = {}
	sum=0
	uci:foreach("network", "interface", function(l)
		type = string.sub(l[".name"], 1, 3)
		if type == "lan" then
			sum=sum+1
		end
	end	)
	if tonumber(sum) >= 16 then
		m.message = translatef("err: Maximum LAN instance count has been reached")
	elseif exist then
		name = ("lan_"..name)
		m.message = translatef("err: Name %s already exists.", name)

	elseif name and #name > 0 then

		if not (string.find(name, "[%c?%p?%s?]+") == nil) then
			m.message = translate("err: Only alphanumeric characters are allowed.")
		else
			namew = name
			name = ("lan_"..name)
			t["ifname"] = "brl-lan"
			t["proto"] = "static"
			t["ipaddr"] = "192.168.1.1"
			t["netmask"] = "255.255.255.0"
			t["disabled"] = "1"

			uci:section("network", "interface", name,t)

			zone_exists = 0
			uci:foreach("firewall", "zone", function(sec)
				if sec.name == name then
					zone_exists = 1
				end
			end	)

			if zone_exists == 0 then
				local z = {}
				z["name"] = name
				z["network"] = name
				z["input"] = "ACCEPT"
				z["output"] = "ACCEPT"
				z["forward"] = "REJECT"
				uci:section("firewall", "zone", nil,z)
				local f = {}
				f["src"] = name
				f["dest"] = "wan"
				uci:section("firewall", "forwarding", nil,f)
				uci:commit("firewall")
			end

			dhcp_exists = 0
			uci:foreach("dhcp", "dhcp", function(sec)
				if sec.interface then
					if sec.interface == name then
						dhcp_exists = 1
					end
				end
			end	)
			if tonumber(dhcp_exists) == 0 then
				local s = {}
				s["interface"] = name
				s["start"] = "100"
				s["limit"] = "150"
				s["leasetime"] = "12h"
				s["time"] = "12"
				s["letter"] = "h"
				uci:section("dhcp", "dhcp", nil,s)
				uci:commit("dhcp")
			end
			uci:save("network")
			uci:commit("network")
			uci:save("firewall")
			uci:commit("firewall")

			m.message = translate("scs:New LAN instance was created successfully. Configure it now")
			luci.http.redirect(luci.dispatcher.build_url("admin", "network", "vlan", "lan", name, "vlan"))
		end
	else
		m.message = translate("err: To create a new LAN instance it's name has to be entered!")
	end
end

-- For some reason, uci still thinks that the changes have not been commited (uci changes still has it as 'not commited')
-- because of this, we need to commit changes after save. 
m.on_after_save = function()
	uci:commit("network")
end

return m
