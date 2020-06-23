-- local dsp = require "luci.dispatcher"

local m = Map("quagga", translatef("OSPF Protocol Configuration"), "")

FileUpload.size = "1000000"
FileUpload.sizetext = translate("Selected file is too large, max 1 MB")
FileUpload.sizetextempty = translate("Selected file is empty")

local s = m:section( NamedSection, "ospf", "ospf", translate("General Settings"), "")

s:option(Flag, "enabled", "Enable", "Enable/Disable OSPF protocol")
s:option(Flag, "enabled_vty", "Enable vty", "Enable/Disable vty access from LAN")
s:option(FileUpload, "custom_conf", "Import config", "Use imported OSPF configuration")
s:option(Value, "id", translate("Router ID"), translate("IP address or any arbitrary 32bit number"))

s = m:section(TypedSection, "ospf_interface", translate("OSPF interface"))
	s.template  = "cbi/tblsection"
	s.addremove = true
	s.anonymous = true
	s.template_addremove = "quagga/add_rem_interface"
	s.novaluetext = translate("There are no OSPF interfaces created yet")
	s.extedit = luci.dispatcher.build_url("admin", "network", "routes", "dynamic_routes", "basic_interface", "%s")

s:option(Flag, "enabled", "Enable", "Enable OSPF interface")
s:option( DummyValue, "ifname", translate("Interface"), translate("Indicates whether a configuration is active or not"))

function s.parse(self, ...)
	local cfgname = luci.http.formvalue("cbid." .. self.config .. "." .. self.sectiontype .. ".interface_name")
	local addButton= luci.http.formvalue("cbid." .. self.config .. "." .. self.sectiontype .. ".add")
	local existname = false
	local delButtonFormString = "cbi.rts." .. self.config .. "."
	local delButtonPress
	local configName

	m.uci:foreach("quagga", "ospf_interface", function(x)
		configName = x[".area_name"] or ""

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
end

function interface_new(self,name, exist)
	local t = {}

	if exist then
		m.message = translatef("err: interface %s already exists.", name)
	elseif name and #name > 0 then
		t["ifname"] = name
		t["enabled"] = "1"
		name = name:gsub("%-", "")

		m.uci:section("quagga", "ospf_interface", name, t)

		m.message = translate("scs:New interface instance was created successfully. Configure it now")
	else
		m.message = translate("err: To create a new interface instance it's name has to be entered!")
	end
end


--========================= networks pridejimas =========================--

s = m:section(TypedSection, "ospf_network", translate("OSPF networks"))
	s.template  = "cbi/tblsection"
	s.addremove = true
	s.anonymous = true
	s.extedit   = false
	s.novaluetext = translate("There are no networks created yet")

s:option(Flag, "enabled", "Enable", "Enable OSPF network")

o = s:option(Value, "net", translate("Network"), translate(""))
	o.datatype = "ip4addr"

s:option(Value, "area", translate("Area"), translate(""))

function m.on_commit(self)
	local enabled = m:formvalue("cbid." .. m.config .. ".ospf.enabled" ) or "0"
	local firewall_rule = m.uci:get("firewall", "A_OSPFIGP")

	if firewall_rule then
		local firewall_ed = m.uci:set("firewall", "A_OSPFIGP", "enabled", enabled)
	else
		local options = {
			enabled=enabled,
			target='ACCEPT',
			src='wan',
			proto='89',
			name='Allow-OSPFIGP-WAN-traffic'
		}

		m.uci:set("firewall", "A_OSPFIGP", "rule")
		m.uci:tset("firewall", "A_OSPFIGP", options)
	end

	m.uci:commit("firewall")
end

return m
