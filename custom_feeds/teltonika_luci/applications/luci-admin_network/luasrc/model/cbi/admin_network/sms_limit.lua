
local m, s, o

require ("luci.http")
local sys = require "luci.sys"

m = Map("sim_switch", translate("SMS Limit Configuration"))

s = m:section(NamedSection, "rules", "rules")
	s:tab("sim1", "SIM1")
	s:tab("sim2", "SIM2")
	s.addremove = false

--[[ SIM 1 ]]--

o = s:taboption("sim1", Value, "field1")
	o.template = "cbi/legend"
	o.titleText = "SMS Limit Configuration"

-- This will not enable SIM_SWITCH to occur, it will simply stop SMS messages from being
-- sent when this option is enabled. (if switchsms_sim is enabled, it will also stop SMS messages
-- from being sent. (logic for this is in libgsm - sms.c)
local enable = s:taboption("sim1", Flag, "sms_limit_enable_sim1", translate("Enable SMS limit"))
	enable.rmempty = false

local period = s:taboption("sim1", ListValue, "period_sms_sim1", translate("Period"),
		translate("Time period for when sent SMS messages should be totaled up"))
	period:value("month", translate("Month"))
	period:value("week", translate("Week"))
	period:value("day", translate("Day"))
	period:depends({sms_limit_enable_sim1 = "1"})

local day = s:taboption("sim1", ListValue, "sms_day_sim1", translate("Start day"))
	day:depends({period_sms_sim1 = "month", sms_limit_enable_sim1 = "1"})
	for i = 1, 31 do
		day:value(i, i)
	end

local hour = s:taboption("sim1", ListValue, "sms_hour_sim1", translate("Start hour"),
		translate("A starting hour in a day for sms limit based SIM card switching period"))
	hour:depends({period_sms_sim1 = "day", sms_limit_enable_sim1 = "1"})
	for i=1,23 do
		hour:value(i, i)
	end
	hour:value("0", "24")

local weekday = s:taboption("sim1", ListValue, "sms_weekday_sim1", translate("Start day"),
		translate("A starting day in a week for sms limit based SIM card switching period"))
	weekday:value("1", translate("Monday"))
	weekday:value("2", translate("Tuesday"))
	weekday:value("3", translate("Wednesday"))
	weekday:value("4", translate("Thursday"))
	weekday:value("5", translate("Friday"))
	weekday:value("6", translate("Saturday"))
	weekday:value("0", translate("Sunday"))
	weekday:depends({period_sms_sim1 = "week", sms_limit_enable_sim1 = "1"})

local sms_limit = s:taboption("sim1", Value, "sms_sim1", translate("SMS limit"),
		translate("Maxiumum amount of SMS that are allowed to be sent in a given period"))
	sms_limit:depends({sms_limit_enable_sim1 = "1"})
	sms_limit.datatype = [[and(min(0), uinteger)]]

	-- Validate does not trigger when there is no input (e.g. empty string)
	function sms_limit.parse(self, section, novld)
		local enabled = luci.http.formvalue("cbid." .. m.config
			.. ".rules.sms_limit_enable_sim1") or "0"
		if enabled == "1" then
			local value = self:formvalue(section) or nil
			if value == nil or value == "" then
				m.message = translate("err: SIM1: SMS limit cannot be left empty")
				m.uci:delete(m.config, "rules", "sms_limit_enable_sim1")
			end
		end
		return AbstractValue.parse(self, section, novld)
	end

o = s:taboption("sim1", Value, "field2")
	o.template = "cbi/legend"
	o.titleText = "Clear SMS Limit"

local sent_sms = s:taboption("sim1", DummyValue, "_dummy_sent1", translate("SMS Sent:"))
	sent_sms.rawhtml = true

	function sent_sms.cfgvalue(self, section)
		return "<span style='line-height: 2.4'>"
			.. (sys.exec("/sbin/sms_counter.lua value_sw SLOT1") or "0")
			.. "</span>"
	end

o = s:taboption("sim1", DummyValue, "_sms_sim1", translate("SMS Limit:"))
	o.rawhtml = true

	function o.cfgvalue(self, section)
		local value = m:get(section, "sms_sim1") or "Not set"
		return "<span style='line-height: 2.4'>" .. value .. "</span>"
	end

local clear = s:taboption("sim1", Button, "_sim1_sms_reset", translate("Clear SMS limit"),
			translate("Clears previously logged sent SMS"))
	clear.customTitle = ""

	function clear.write(self, section)
		sys.exec("/sbin/sms_counter.lua reset_sw SLOT1")
	end

--[[ SIM 2 ]]--

o = s:taboption("sim2", Value, "field3")
	o.template = "cbi/legend"
	o.titleText = "SMS Limit Configuration"

-- This will not enable SIM_SWITCH to occur, it will simply stop SMS messages from being
-- sent when this option is enabled. (if switchsms_sim is enabled, it will also stop SMS messages
-- from being sent. (logic for this is in libgsm - sms.c)
local enable2 = s:taboption("sim2", Flag, "sms_limit_enable_sim2", translate("Enable SMS limit"))
	enable2.rmempty = false

local period2 = s:taboption("sim2", ListValue, "period_sms_sim2", translate("Period"),
		translate("Time period for when sent SMS messages should be totaled up"))
	period2:depends({sms_limit_enable_sim2 = "1"})
	period2:value("month", translate("Month"))
	period2:value("week", translate("Week"))
	period2:value("day", translate("Day"))

local day2 = s:taboption("sim2", ListValue, "sms_day_sim2", translate("Start day"))
	day2:depends({period_sms_sim2 = "month", sms_limit_enable_sim2 = "1"})
	for i = 1, 31 do
		day2:value(i, i)
	end

local hour2 = s:taboption("sim2", ListValue, "sms_hour_sim2", translate("Start hour"),
		translate("A starting hour in a day for sms limit based SIM card switching period"))
	hour2:depends({period_sms_sim2 = "day", sms_limit_enable_sim2 = "1"})
	for i=1,23 do
		hour2:value(i, i)
	end
	hour2:value("0", "24")

local weekday2 = s:taboption("sim2", ListValue, "sms_weekday_sim2", translate("Start day"),
		translate("A starting day in a week for sms limit based SIM card switching period"))
	weekday2:value("1", translate("Monday"))
	weekday2:value("2", translate("Tuesday"))
	weekday2:value("3", translate("Wednesday"))
	weekday2:value("4", translate("Thursday"))
	weekday2:value("5", translate("Friday"))
	weekday2:value("6", translate("Saturday"))
	weekday2:value("0", translate("Sunday"))
	weekday2:depends({period_sms_sim2 = "week", sms_limit_enable_sim2 = "1"})

local sms_limit2 = s:taboption("sim2", Value, "sms_sim2", translate("SMS limit"),
		translate("Maxiumum amount of SMS that are allowed to be sent in a given period"))
	sms_limit2.datatype = [[and(min(0), uinteger)]]
	sms_limit2:depends({sms_limit_enable_sim2 = "1"})

	-- Validate does not trigger when there is no input (e.g. empty string)
	function sms_limit2.parse(self, section, novld)
		local enabled = luci.http.formvalue("cbid." .. m.config
			.. ".rules.sms_limit_enable_sim2") or "0"
		if enabled == "1" then
			local value = self:formvalue(section) or nil
			if value == nil or value == "" then
				m.message = translate("err: SIM2: SMS limit cannot be left empty")
				m.uci:delete(m.config, "rules", "sms_limit_enable_sim2")
			end
		end
		return AbstractValue.parse(self, section, novld)
	end

o = s:taboption("sim2", Value, "field4")
	o.template = "cbi/legend"
	o.titleText = "Clear SMS Limit"

local sent_sms2 = s:taboption("sim2", DummyValue, "_dummy_sent2", translate("SMS Sent:"))
	sent_sms2.rawhtml = true

	function sent_sms2.cfgvalue(self, section)
		return "<span style='line-height: 2.4'>"
			.. (sys.exec("/sbin/sms_counter.lua value_sw SLOT2") or "0")
			.. "</span>"
	end

o = s:taboption("sim2", DummyValue, "_sms_sim2", translate("SMS Limit:"))
	o.rawhtml = true

	function o.cfgvalue(self, section)
		local value = m:get(section, "sms_sim2") or "Not set"
		return "<span style='line-height: 2.4'>" .. value .. "</span>"
	end

local clear2 = s:taboption("sim2", Button, "_sim2_sms_reset", translate("Clear SMS limit"),
			translate("Clears previously logged sent SMS"))
	clear2.customTitle = ""

	function clear2.write(self, section)
		sys.exec("/sbin/sms_counter.lua reset_sw SLOT2")
	end

function m.on_commit(self)
	local enb_sim1 = m:formvalue("cbid." .. self.config.. ".rules.sms_limit_enable_sim1") or "0"
	local enb_sim2 = m:formvalue("cbid." .. self.config.. ".rules.sms_limit_enable_sim2") or "0"
	local enb_switch_sim1 = m.uci:get(self.config, "rules", "switchsms_sim1") or "0"
	local enb_switch_sim2 = m.uci:get(self.config, "rules", "switchsms_sim2") or "0"

	if enb_sim1 == "1" or enb_sim2 == "1" or enb_switch_sim1 == "1"
	or enb_switch_sim2 == "1" then
		m.uci:set("overview", "show", "sms_limit", "1")
	else
		m.uci:set("overview", "show", "sms_limit", "0")
	end
	m.uci:commit("overview")
end

return m
