local m, agent, sys,  o, port, remote, deathtrap = false, enable, com, comname
local ds = require "luci.dispatcher"
local uci  = require "luci.model.uci".cursor()
local fw = require "luci.model.firewall"
local fs = require "nixio.fs"
local x = uci.cursor()
local sys = require "luci.sys"

m = Map("snmpd", translate("Trap Configuration"))
m:chain("firewall")

agent = m:section(TypedSection, "trap", translate("Trap Service Settings"))
-- agent.addremove = true
agent.anonymous = true

o = agent:option(Flag, "trap_enabled", translate("SNMP Trap"), translate("Enable SNMP (Simple Network Management Protocol) trap functionality"))
-----
--ip
-----
hst = agent:option(Value, "trap_host", translate("Host/IP"), translate("Host to transfer SNMP (Simple Network Management Protocol) traffic to"))
hst.datatype = "ipaddr"

-- -------
-- -- port
-- -------
prt = agent:option(Value, "trap_port", translate("Port"), translate("Port for trap\\'s host"))
prt.default = "162"
prt.datatype = "port"

-- -- community
-- --
c = agent:option(ListValue, "trap_community", translate("Community"), translate("The SNMP (Simple Network Management Protocol) Community is an ID that allows access to a router\\'s SNMP data"))
c:value("public", "Public")
c:value("private", "Private")
c.default = "Public"



s = m:section(TypedSection, "rule", translate("Trap Rules"))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.extedit   = ds.build_url("admin/services/snmp/trap-settings/%s")
s.template_addremove = "tlt-snmp/add_trap_rule"
s.novaluetext = translate("There are no trap rules created yet")

function s.create(self, section)
	a = m:formvalue("_newinput.action")
	existname = false
	local actionName

	uci:foreach("snmpd", "rule", function(x)
		actionName = x["action"] or ""
		if actionName == a then
			existname = true
		end
	end)

	if existname then
		if a == "sigEnb" then
			a = translate("signal strength trap")
		elseif a == "conEnb" then
			a = translate("connection type trap")
		elseif a == "digIn" then
			a = translate("digital input trap")
		elseif a == "digOCIn" then
			a = translate("digital isolated input trap")
		elseif a == "analog" then
			a = translate("analog input trap")
		elseif a == "digOut" then
			a = translate("digital output trap")
		elseif a == "digRelayOut" then
			a = translate("digital relay output trap")
		elseif a == "dig4PinIn" then
			a = translate("digital 4PIN input trap")
		elseif a == "dig4PinOut" then
			a = translate("digital 4PIN output trap")
		else
			a = translate("N/A")
		end
		m.message = translatef("err: Action %s already exists.", a)
	else
		created = TypedSection.create(self, section)
		self.map:set(created, "action", a)
	end
end

function s.parse(self, ...)
	TypedSection.parse(self, ...)

	if created then
		m.uci:save("snmpd")
		luci.http.redirect(ds.build_url("admin/services/snmp/trap-settings", created	))
	end
end

src = s:option(DummyValue, "action", translate("Action"), translate("The action to be performed when a rule is met"))
src.rawhtml = true
src.width   = "30%"

function src.cfgvalue(self, s)
	local z = self.map:get(s, "action")
	if z == "sigEnb" then
		return translate("Signal strength trap")
	elseif z == "conEnb" then
		return translate("Connection type trap")
	elseif z == "digIn" then
		return translate("Digital input trap")
	elseif z == "digOCIn" then
		return translate("Digital isolated input trap")
	elseif z == "analog" then
		return translate("Analog input trap")
	elseif z == "digOut" then
		return translate("Digital output trap")
	elseif z == "digRelayOut" then
		return translate("Digital relay output trap")
	elseif z == "dig4PinIn" then
		return translate("Digital 4PIN input trap")
	elseif z == "dig4PinOut" then
		return translate("Digital 4PIN output trap")
	else
		return translate("N/A")
	end
end

o = s:option(Flag, "enabled", translate("Enable"), translate("Make a rule active/inactive"))

return m
