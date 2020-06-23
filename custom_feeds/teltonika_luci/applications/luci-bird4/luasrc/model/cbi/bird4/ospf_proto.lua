
local dsp = require "luci.dispatcher"
local uci = require "luci.model.uci".cursor()
local area_count = 0

m = Map("bird4", translate("OSPF Protocol Configuration"))

osp = m:section(TypedSection, "ospf", translate("OSPF General Instance"))
osp.anonymous = false

hid = osp:option(Flag, "enabled", translate("Enable"), translate("Enable OSPF General Instance"))
hid.rmempty = false
hid.default = "0"

stub = osp:option(Flag, "stub", translate("Stub"), translate("Checke to change this area type to stub"))
stub.rmempty = false
stub.default = "0"

rfc = osp:option(Flag, "rfc1583compat", translate("RFC1583 compatibility"), translate("Check to make the instance compatible with RFC1583 standart"))
rfc.rmempty = false
rfc.default = "0"

import = osp:option(Value, "import", translate("Import"), translate("Select wich routes can be imported"))
import.optional = true
import:value("all", translate("All"))
import:value("none", translate("None"))
import.default= "all"

export = osp:option(Value, "export", translate("Export"), translate("Select wich routes can be exported"))
export.optional = true
export:value("all", translate("All"))
export:value("none", translate("None"))
export.default= "all"

s = m:section(TypedSection, "ospf_area", translate("OSPF Area"))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.extedit   = dsp.build_url("admin/network/routes/dynamic_routes/ospf_proto/%s")
s.template_addremove = "bird4/add_rem"
s.novaluetext = translate("There are no areas created yet")

-- uci:foreach("bird4", "ospf_area", function(sec)
-- 	--cecho("section called!")
-- 	-- Entry signifies that there already is a section, therefore we will disable the ability to add or remove another section
-- 	s.addremoveAdd = false
-- end)

s.extedit = luci.dispatcher.build_url("admin", "network", "routes", "dynamic_routes", "basic", "%s")


src = s:option(DummyValue, "name", translate("Area name"), translate("Area identification number"))

-- function src.cfgvalue(self, section)
-- -- Padaro pirma didziaja raide
-- -- 	return section:gsub("^%l", string.upper) or "Unknown"
-- 	return section
-- end

local status = s:option( DummyValue, "enabled", translate("Enable"), translate("Indicates whether a configuration is active or not"))

function status.cfgvalue(self, section)
	local val = AbstractValue.cfgvalue(self, section)
	if val == "1" then
		return translate("Yes")
	else
		return translate("No")
	end
end

function s.parse(self, ...)
	local cfgname = luci.http.formvalue("cbid." .. self.config .. "." .. self.sectiontype .. ".name")
	local addButton= luci.http.formvalue("cbid." .. self.config .. "." .. self.sectiontype .. ".add")
	local existname = false
	local delButtonFormString = "cbi.rts." .. self.config .. "."
	local delButtonPress = false
	local configName, ifconfigName, netconfigName, deleteConfig
	
	
	uci:foreach("bird4", "ospf_area", function(x)
		configName = x[".name"] or ""
		area_count = area_count + 1
		if luci.http.formvalue(delButtonFormString .. configName) then
			delButtonPress = true
			deleteConfig = uci:get("bird4",configName,"name")
		end

		if configName == cfgname then
			existname = true
		end
	end)
	
	if addButton then
		if cfgname and cfgname ~= '' then
			area_new(self, cfgname, existname)
		end
-- 		luci.sys.call("/etc/init.d/bird4 restart >/dev/null")
	end
	if delButtonPress then
		uci:foreach("bird4", "ospf_interface", function(x)
			ifconfigName = x[".name"] or ""
			configarea = x["area_name"] or ""
			if deleteConfig == configarea then
				uci.delete("bird4", ifconfigName)
				uci.save("bird4")
			end
		end)
		uci:foreach("bird4", "ospf_network", function(x)
			netconfigName = x[".name"] or ""
			configarea = x["area_name"] or ""
			if deleteConfig == configarea then

				uci.delete("bird4", netconfigName)
				uci.save("bird4")
			end
		end)

	luci.sys.call("uci commit bird4")
-- 	luci.sys.call("/etc/init.d/bird4 restart >/dev/null")
	end

	TypedSection.parse( self, section )
	uci.commit("bird4")
end

function area_new(self,name, exist)

	local t = {}

	if exist then
		m.message = translatef("err: Name %s already exists.", name)
	else
		if name and #name > 0 then
			if tonumber(name) then
				t["enabled"] = "0"
				t["name"] = name
				t["instance"] = "ospf1"
				t["stub"] = "0"
		--
				name = string.gsub(name, "%.", "_", 3)
				uci:section("bird4", "ospf_area", name,t)
				uci:save("bird4")
				uci.commit("bird4")
				m.message = translate("scs: New area instance was created successfully. Configure it now")
			else
				m.message = translate("err: New area instance name must be a number!")
			end
		else
			m.message = translate("err: To create a new area instance it's name has to be entered!")
		end
	end
end

function m.on_commit(map)
	check = luci.http.formvalue("cbid.bird4.ospf1.enabled") or "0"

	if check then
		if check == "1" then
			if area_count == 0 then
				m.message = translatef("err: Atleast one OSPF area must be configured")
			end
			m.uci:set("bird4", "ospf1", "disabled", "0")
			m.uci:set("bird4", "bird", "enabled", "1")
		else
			m.uci:set("bird4", "ospf1", "disabled", "1")
			m.uci:set("bird4", "bird", "enabled", "0")
		end
			m.uci:save("bird4")
			m.uci:commit("bird4")
			luci.sys.call("/etc/init.d/bird4 restart >/dev/null")
	end
end

return m
