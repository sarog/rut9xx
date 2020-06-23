local sys = require "luci.sys"
local dsp = require "luci.dispatcher"
local ft = require "luci.tools.input-output"
local utl = require "luci.util"

local m, s, o

arg[1] = arg[1] or ""

m = Map("ioman", translate("Custom I/O Status Labels"))
m.redirect = dsp.build_url("admin/services/input-output/")

if arg[1] == "1" then	
	s = m:section(NamedSection, "iolabels", "iostatus", translate("Customize digital input and state fields"))
	o = s:option(Value, "digitalinput", translate("Digital input name"), translate("Digital input name label"))
	o = s:option(Value, "diopen", translate("Input open state name"), translate("Digital input open label"))
	o = s:option(Value, "dishorted", translate("Input shorted state name"), translate("Digital input shorted label"))

	s = m:section(TypedSection, "ioman", translate("Digital input configuration"))
	o = s:option(ListValue, "active_DIN1_status", translate("Active state"), translate("Default digital input active state"))
	o:value("0", translate("Input open"))
	o:value("1", translate("Input shorted"))
end

if arg[1] == "2" then
	s = m:section(NamedSection, "iolabels", "iostatus", translate("Customize digital galvanicaly isolated input and state fields"))
	o = s:option(Value, "digitalisolated", translate("Digital isolated input name"), translate("Digital isolated input name label"))
	o = s:option(Value, "disolhigh", translate("High logic level state"), translate("Digital isolated input high level label"))
	o = s:option(Value, "disollow", translate("Low logic level state"), translate("Digital isolated input low level label"))

	s = m:section(TypedSection, "ioman", translate("Digital isolated input configuration"))
	o = s:option(ListValue, "active_DIN2_status", translate("Active state"), translate("Default digital isolated input active state"))
	o:value("0", translate("Low level"))
	o:value("1", translate("High level"))
end

if arg[1] == "3" then
	s = m:section(NamedSection, "iolabels", "iostatus", translate("Customize analog input and value fields"))
	s.template_addremove = "input-output/analogfield"

	o = s:option(Value, "anformultiply")
	o.datatype = "range(-999999,999999)"
        o.placeholder = "Value"
	o = s:option(Value, "anforoffset")
	o.datatype = "range(-999999,999999)"
        o.placeholder = "Value"
	o = s:option(Value, "anforadd")
	o.datatype = "range(-999999,999999)"
        o.placeholder = "Value"
	o = s:option(Value, "anfordivide")
	o.datatype = "range(0.000001,999999)"
        o.placeholder = "Value"
	o = s:option(Value, "analoginput", translate("Analog input name"))
	o = s:option(Value, "anformeasunit", translate("User defined unit of measurement"))
end

if arg[1] == "4" then
	s = m:section(NamedSection, "iolabels", "iostatus", translate("Customize open collector output and state fields"))
	o = s:option(Value, "opencollector", translate("Open collector output name"), translate("Open collector output name label"))
	o = s:option(Value, "ocouton", translate("Open collector active state"), translate("Open collector output active state label"))
	o = s:option(Value, "ocoutoff", translate("Open collector inactive state"), translate("Open collector output inactive state label"))

	s = m:section(TypedSection, "ioman", translate("Open collector output configuration"))
	o = s:option(ListValue, "active_DOUT1_status", translate("Active state"), translate("Default open collector output active state"))
	o:value("0", translate("Low level"))
	o:value("1", translate("High level"))
end

if arg[1] == "5" then
	s = m:section(NamedSection, "iolabels", "iostatus", translate("Customize relay output and state fields"))
	o = s:option(Value, "relayoutput", translate("Relay output name"), translate("Relay output name label"))
	o = s:option(Value, "relon", translate("Relay out active state"), translate("Relay output active state label"))
	o = s:option(Value, "reloff", translate("Relay out inactive state"), translate("Relay output inactive state label"))

	s = m:section(TypedSection, "ioman", translate("Relay output configuration"))
	o = s:option(ListValue, "active_DOUT2_status", translate("Active state"), translate("Default relay output active state"))
	o:value("0", translate("Contacts open"))
	o:value("1", translate("Contacts closed"))
end

if arg[1] == "6" then
	s = m:section(NamedSection, "iolabels", "iostatus", translate("Customize digital 4PIN input and state fields"))
	o = s:option(Value, "4p_digitalinput", translate("Digital input name"), translate("Digital 4PIN input name label"))
	o = s:option(Value, "4p_diopen", translate("High level state name"), translate("Digital 4PIN input high level label"))
	o = s:option(Value, "4p_dishorted", translate("Low level state name"), translate("Digital 4PIN input low level label"))

	s = m:section(TypedSection, "ioman", translate("Digital 4PIN isolated input configuration"))
	o = s:option(ListValue, "active_DIN3_status", translate("Active state"), translate("Default 4PIN isolated input active state"))
	o:value("0", translate("Low level"))
	o:value("1", translate("High level"))
end

if arg[1] == "7" then
	s = m:section(NamedSection, "iolabels", "iostatus", translate("Customize digital 4PIN output and state fields"))
	o = s:option(Value, "4p_opencollector", translate("Open collector output name"), translate("Open collector output name label"))
	o = s:option(Value, "4p_ocouton", translate("Open collector active state"), translate("Open collector output active state label"))
	o = s:option(Value, "4p_ocoutoff", translate("Open collector inactive state"), translate("Open collector output inactive state label"))

	s = m:section(TypedSection, "ioman", translate("Digital 4PIN output configuration"))
	state = s:option(ListValue, "active_DOUT3_status", translate("Digital 4PIN output"), translate("Default digital 4PIN output active state"))
	state:value("0", translate("Low level"))
	state:value("1", translate("High level"))
end

if arg[1] == "del" then
	m = Map("ioman", translate("Press \"Save\" button to restore default I/O status labels"))
	m.redirect = dsp.build_url("admin/services/input-output/")	
	--	luci.sys.exec("uci commit ioman")
	function m.on_commit()
		luci.sys.exec("uci delete ioman.iolabels")
		luci.sys.exec("uci set ioman.iolabels=iostatus")
		m.uci.commit("ioman")
	end
else
	s.addremove = false
	s.anonymous = true
	s.rmempty = false
end

return m
