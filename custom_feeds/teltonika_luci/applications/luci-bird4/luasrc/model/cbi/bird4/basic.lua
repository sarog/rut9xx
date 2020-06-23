local sys = require "luci.sys"
local dsp = require "luci.dispatcher"
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()
-- local mix = require("luci.mtask")

local VPN_INST, TMODE, AREANAME

if arg[1] then
	VPN_INST = arg[1]
else
	return nil
end

local areaname = uci:get("bird4",VPN_INST,"name")
AREA_NAME = areaname
local mode, o

local m = Map("bird4", translatef("Area Instance: %s", AREA_NAME:gsub("^%l", string.upper)), "")

m.redirect = dsp.build_url("admin/network/routes/dynamic_routes/ospf_proto/")
if m.uci:get("bird4", arg[1]) ~= "ospf_area" then
	luci.http.redirect(dsp.build_url("admin/network/routes/dynamic_routes/ospf_proto/"))
  	return
end

local ss = m:section( NamedSection, VPN_INST, "ospf_area", translate("Main Settings"), "")
ss.anonymous = true
ss.addremove = false

o = ss:option( Flag, "enabled", translate("Enabled"), translate("Enable current configuration"))
o.forcewrite = true
o.rmempty = false

local stub = ss:option(Flag, "stub", translate("Stub"), translate(""))
stub.rmempty = false
stub.default = "0"
--========================= interface pridejimas =========================--

s = m:section(TypedSection, "ospf_interface", translate("OSPF interface"))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.extedit   = dsp.build_url("admin/network/routes/dynamic_routes/ospf_proto/%s")
s.template_addremove = "bird4/add_rem_interface"
s.novaluetext = translate("There are no interfaces created yet")
s.area = AREA_NAME

s.extedit = luci.dispatcher.build_url("admin", "network", "routes", "dynamic_routes", "basic_interface", "%s")


function s.cfgvalue(self, section)
-- Padaro pirma didziaja raide
-- 	return section:gsub("^%l", string.upper) or "Unknown"
	return section
end

local status = s:option( DummyValue, "ifname", translate("Interface"), translate("Indicates whether a configuration is active or not"))

function s.parse(self, ...)
	local cfgname = luci.http.formvalue("cbid." .. self.config .. "." .. self.sectiontype .. ".interface_name")
	local addButton= luci.http.formvalue("cbid." .. self.config .. "." .. self.sectiontype .. ".add")
	local existname = false
	local delButtonFormString = "cbi.rts." .. self.config .. "."
	local delButtonPress
	local configName
	uci:foreach("bird4", "ospf_interface", function(x)
		configName = x[".area_name"] or ""
--  		luci.sys.call("echo \"".. configName .."\" >> /tmp/aaaa")
		if luci.http.formvalue(delButtonFormString .. configName) then
			delButtonPress = true
		end

		if configName == cfgname then
			existname = true
		end
	end)
	if addButton then
		if cfgname and cfgname ~= '' then
			interface_new(self, cfgname, existname)
		end
	end

	TypedSection.parse( self, section )
	uci.commit("bird4")
end

function interface_new(self,name, exist)

	local t = {}

	if exist then
		m.message = translatef("err: interface %s already exists.", name)
	elseif name and #name > 0 then
		
-- 		luci.sys.call("echo \"|".. name .."|\" >> /tmp/aaaa")
		t["ifname"] = name
		t["area_name"] = AREA_NAME

		--if name == "br-lan" then
		--	name = "brlan"
		--end
		
		name = name:gsub("%-", "")
		
		long_name = name .."_".. VPN_INST
		uci:section("bird4", "ospf_interface", long_name, t)
		uci:save("bird4")
		uci.commit("bird4")
		m.message = translate("scs:New interface instance was created successfully. Configure it now")
		luci.sys.call("/etc/init.d/bird4 restart >/dev/null")
	else
		m.message = translate("err: To create a new interface instance it's name has to be entered!")
	end
end

--========================= networks pridejimas =========================--

s = m:section(TypedSection, "ospf_network", translate("OSPF networks"))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
-- s.extedit   = dsp.build_url("admin/network/bird4/ospf_proto/%s")
s.extedit   = false
s.template_addremove = "bird4/add_rem_network"
s.novaluetext = translate("There are no networks created yet")
s.area = AREA_NAME

-- s.extedit = luci.dispatcher.build_url("admin", "network", "bird4", "basic_network", "%s")


local ip = s:option( DummyValue, "ip", translate("IP"), translate("Indicates whether a configuration is active or not"))


local hidden = s:option(Flag, "hidden", translate("Hidden"), translate("Hide network"))
hidden.rmempty = false
hidden.default = "0"

function s.parse(self, ...)
	local cfgname1 = luci.http.formvalue("cbid." .. self.config .. "." .. self.sectiontype .. ".network_name")
	local addButton= luci.http.formvalue("cbid." .. self.config .. "." .. self.sectiontype .. ".add_network")
	local existname1= false
	local delButtonFormString1 = "cbi.rts." .. self.config .. "."
	local delButtonFormString2 = "cbi.rts." .. self.config .. "."
	local delButtonPress1
	local delButtonPress2
	local configName1

	uci:foreach("bird4", "ospf_network", function(x)
		configName1 = x[".name"] or ""
--  		luci.sys.call("echo \"".. configName .."\" >> /tmp/aaaa")
		if luci.http.formvalue(delButtonFormString1 .. configName1) then
			delButtonPress1 = true
		end

		if configName1 == cfgname1 then
			existname1= true
		end
	end)
	
	uci:foreach("bird4", "ospf_interface", function(x)
		configName2 = x[".name"] or ""
--  		luci.sys.call("echo \"".. configName .."\" >> /tmp/aaaa")
		if luci.http.formvalue(delButtonFormString2 .. configName2) then
			delButtonPress2 = true
		end
	end)
	
	if addButton then
		if cfgname1 and cfgname1 ~= '' then
			network_new(self, cfgname1, existname1)
		end
	end
	
	if delButtonPress1 or delButtonPress2 then
		luci.sys.call("/etc/init.d/bird4 restart >/dev/null")
	end
	
	TypedSection.parse( self, section )
	uci.commit("bird4")
end

function network_new(self,name, exist)

	local t = {}
	if exist then
		m.message = translatef("err: network %s already exists.", name)
	elseif name and #name > 0 then
		t["ip"] = name
		t["hidden"] = "0"
		t["area_name"] = AREA_NAME

		--all_trim(name)
		name = string.gsub(name, "%.", "_", 3)
		name = string.gsub(name, "/", "l", 1)
		long_name = name .."_".. VPN_INST
		uci:section("bird4", "ospf_network", long_name, t)
		uci:save("bird4")
		uci.commit("bird4")
		m.message = translate("scs:New network instance was created successfully. Configure it now")
		luci.sys.call("/etc/init.d/bird4 restart >/dev/null")
	else
		m.message = translate("err: To create a new network instance it's name has to be entered!")
	end
end

function m.on_commit(map)
	luci.sys.call("/etc/init.d/bird4 restart >/dev/null")
end

return m
