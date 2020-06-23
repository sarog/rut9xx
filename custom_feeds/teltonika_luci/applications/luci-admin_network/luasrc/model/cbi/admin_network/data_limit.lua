
local utl = require "luci.util"
local nw = require "luci.model.network"
local sys = require "luci.sys"
local eventlog = require'tlt_eventslog_lua'
local moduleVidPid = utl.trim(sys.exec("uci get -q system.module.vid")) .. ":" .. utl.trim(sys.exec("uci get -q system.module.pid"))
local moduleType = luci.util.trim(luci.sys.exec("uci get -q system.module.type"))
local m

local function cecho(string)
	luci.sys.call("echo \"" .. string .. "\" >> /tmp/luci.log")
end

m = Map("data_limit", translate("Mobile Data Limit Configuration"))
m.disclaimer_msg = true

s = m:section(NamedSection, "limit", "limit");
s.addremove = false
s:tab("primarytab", translate("SIM1"))
s:tab("secondarytab", translate("SIM2"))
s.template = "cbi/sim_manage_tabs_switch"


-----------------------
--Primary taboptions---
-----------------------
e = s:taboption("primarytab", Value, "field1")
e.template = "cbi/legend"
e.titleText = "Data Connection Limit Configuration"

prim_enb_conn = s:taboption("primarytab", Flag, "prim_enb_conn", translate("Enable data connection limit"), translate("Disables mobile data when a limit for current period is reached"))
prim_enb_conn.rmempty = false

o1 = s:taboption("primarytab", Value, "prim_conn_limit", translate("Data limit* (MB)"), translate("Disable mobile data after limit value in MB is reached"))
	o1.datatype = "uinteger"

	-- Validate does not trigger when there is no input (e.g. empty string)
	function o1.parse(self, section, novld)
		local enabled = luci.http.formvalue("cbid.data_limit.limit.prim_enb_conn") or "0"
		if enabled == "1" then
			local value = self:formvalue(section) or nil
			if value == nil or value == "" or value:match("^[1-9]%d*$]") then
				m.message = translate("err: SIM1: Data limit is a required value. Only digits are allowed.")
				m.uci:set(m.config, "limit", "prim_enb_conn", "0")
				return
			end
		end
		return AbstractValue.parse(self, section, novld)
	end

o = s:taboption("primarytab", ListValue, "prim_conn_period", translate("Period"), translate("Period for which mobile data limiting should apply"))
o:value("month", translate("Month"))
o:value("week", translate("Week"))
o:value("day", translate("Day"))

o = s:taboption("primarytab", ListValue, "prim_conn_day", translate("Start day"), translate("A starting day in a month for mobile data limiting period"))
o:depends({prim_conn_period = "month"})
for i=1,31 do
	o:value(i, i)
end

o = s:taboption("primarytab", ListValue, "prim_conn_hour", translate("Start hour"), translate("A starting hour in a day for mobile data limiting period"))
o:depends({prim_conn_period = "day"})
for i=1,23 do
	o:value(i, i)
end
o:value("0", "24")

o = s:taboption("primarytab", ListValue, "prim_conn_weekday", translate("Start day"), translate("A starting day in a week for mobile data limiting period"))
o:value("1", translate("Monday"))
o:value("2", translate("Tuesday"))
o:value("3", translate("Wednesday"))
o:value("4", translate("Thursday"))
o:value("5", translate("Friday"))
o:value("6", translate("Saturday"))
o:value("0", translate("Sunday"))
o:depends({prim_conn_period = "week"})


--------------------------------------------------------------------------------
--------------------SMS warninig------------------------------------------------
--------------------------------------------------------------------------------

e = s:taboption("primarytab", Value, "field2")
	e.template = "cbi/legend"
	e.titleText = "SMS Warning Configuration"
	e:depends({prim_enb_conn = "1"})

prim_enb_wrn = s:taboption("primarytab", Flag, "prim_enb_wrn", translate("Enable SMS warning"), translate("Enables sending of warning SMS message when mobile data limit for current period is reached"))
	prim_enb_wrn.rmempty = false
	prim_enb_wrn:depends({prim_enb_conn = "1"})

o = s:taboption("primarytab", Value, "prim_wrn_limit", translate("Data limit* (MB)"), translate("Send warning SMS message after limit value in MB is reached"))
	o.datatype = "uinteger"
	o:depends({prim_enb_conn = "1"})

	function o:validate(Value)
		local failure
		if Value == nil or Value == "" then
			m.message = translate("err: mobile data limit value is empty!")
			return nil
		elseif not Value:match("^[1-9]%d*$") then
			m.message = translate("err: mobile data limit value is incorrect!")
			return nil
		end
		return Value
	end

o = s:taboption("primarytab", ListValue, "prim_wrn_period", translate("Period"), translate("Period for which SMS warning for mobile data limit should apply"))
	o:value("month", translate("Month"))
	o:value("week", translate("Week"))
	o:value("day", translate("Day"))
	o:depends({prim_enb_conn = "1"})

o = s:taboption("primarytab", ListValue, "prim_wrn_day", translate("Start day"), translate("A starting day in a month for mobile data limit SMS warning"))
	o:depends({prim_wrn_period = "month", prim_enb_conn = "1"})
	for i=1,31 do
		o:value(i, i)
	end

o = s:taboption("primarytab", ListValue, "prim_wrn_hour", translate("Start hour"), translate("A starting hour in a day for mobile data limit SMS warning"))
	o:depends({prim_wrn_period = "day", prim_enb_conn = "1"})
	for i=1,23 do
		o:value(i, i)
	end
	o:value("0", "24")

o = s:taboption("primarytab", ListValue, "prim_wrn_weekday", translate("Start day"), translate("A starting day in a week for mobile data limit SMS warning"))
	o:value("1", translate("Monday"))
	o:value("2", translate("Tuesday"))
	o:value("3", translate("Wednesday"))
	o:value("4", translate("Thursday"))
	o:value("5", translate("Friday"))
	o:value("6", translate("Saturday"))
	o:value("0", translate("Sunday"))
	o:depends({prim_wrn_period = "week", prim_enb_conn = "1"})

e = s:taboption("primarytab", Value, "prim_wrn_number", translate("Phone number"), translate("A phone number to send warning SMS message to, e.g. +37012345678"))
	e:depends({prim_enb_conn = "1"})

-------------------------
--Secondary taboptions---
-------------------------
e = s:taboption("secondarytab", Value, "field3")
e.template = "cbi/legend"
e.titleText = "Data Connection Limit Configuration"

sec_enb_conn = s:taboption("secondarytab", Flag, "sec_enb_conn", translate("Enable data connection limit"), translate("Disables mobile data when a limit for current period is reached"))
sec_enb_conn.rmempty = false

o1 = s:taboption("secondarytab", Value, "sec_conn_limit", translate("Data limit* (MB)"), translate("Disable mobile data after limit value in MB is reached"))
	o1.datatype = "uinteger"

	-- Validate does not trigger when there is no input (e.g. empty string)
	function o1.parse(self, section, novld)
		local enabled = luci.http.formvalue("cbid.data_limit.limit.sec_enb_conn") or "0"
		if enabled == "1" then
			local value = self:formvalue(section) or nil
			if value == nil or value == "" or value:match("^[1-9]%d*$]") then
				m.message = translate("err: SIM2: Data limit is a required value. Only digits are allowed.")
				m.uci:set(m.config, "limit", "sec_enb_conn", "0")
				return
			end
		end
		return AbstractValue.parse(self, section, novld)
	end


o = s:taboption("secondarytab", ListValue, "sec_conn_period", translate("Period"), translate("Period for which mobile data limiting should apply"))
o:value("month", translate("Month"))
o:value("week", translate("Week"))
o:value("day", translate("Day"))

o = s:taboption("secondarytab", ListValue, "sec_conn_day", translate("Start day"), translate("A starting time for mobile data limiting period"))
o:depends({sec_conn_period = "month"})
for i=1,31 do
	o:value(i, i)
end

o = s:taboption("secondarytab", ListValue, "sec_conn_hour", translate("Start hour"), translate("A starting time for mobile data limiting period"))
o:depends({sec_conn_period = "day"})
for i=1,23 do
	o:value(i, i)
end
o:value("0", "24")

o = s:taboption("secondarytab", ListValue, "sec_conn_weekday", translate("Start day"), translate("A starting time for mobile data limiting period"))
o:value("1", translate("Monday"))
o:value("2", translate("Tuesday"))
o:value("3", translate("Wednesday"))
o:value("4", translate("Thursday"))
o:value("5", translate("Friday"))
o:value("6", translate("Saturday"))
o:value("0", translate("Sunday"))
o:depends({sec_conn_period = "week"})


--------------------------------------------------------------------------------
--------------------SMS warninig section----------------------------------------
--------------------------------------------------------------------------------

e = s:taboption("secondarytab", Value, "field4")
	e.template = "cbi/legend"
	e.titleText = "SMS Warning Configuration"
	e:depends({sec_enb_conn = "1"})

o = s:taboption("secondarytab", Flag, "sec_enb_wrn", translate("Enable SMS warning"), translate("Enables sending of warning SMS message when mobile data limit for current period is reached"))
	o.rmempty = false
	o:depends({sec_enb_conn = "1"})


o = s:taboption("secondarytab", Value, "sec_wrn_limit", translate("Data limit* (MB)"), translate("Send warning SMS message after limit value in MB is reached"))
	o.datatype = "uinteger"
	o:depends({sec_enb_conn = "1"})

	function o:validate(Value)
		local failure
		if Value == nil or Value == "" then
			m.message = translate("err: mobile data limit value is empty!")
			return nil
		elseif not Value:match("^[1-9]%d*$") then
			m.message = translate("err: mobile data limit value is incorrect!")
			return nil
		end
		return Value
	end

o = s:taboption("secondarytab", ListValue, "sec_wrn_period", translate("Period"), translate("Period for which mobile data limiting should apply"))
	o:value("month", translate("Month"))
	o:value("week", translate("Week"))
	o:value("day", translate("Day"))
	o:depends({sec_enb_conn = "1"})


o = s:taboption("secondarytab", ListValue, "sec_wrn_day", translate("Start day"), translate("A starting time for mobile data limiting period"))
	o:depends({sec_wrn_period = "month", sec_enb_conn = "1"})
	for i=1,31 do
		o:value(i, i)
	end

o = s:taboption("secondarytab", ListValue, "sec_wrn_hour", translate("Start hour"), translate("A starting time for mobile data limiting period"))
	o:depends({sec_wrn_period = "day", sec_enb_conn = "1"})
	for i=1,23 do
		o:value(i, i)
	end
	o:value("0", "24")

o = s:taboption("secondarytab", ListValue, "sec_wrn_weekday", translate("Start day"), translate("A starting time for mobile data limiting period"))
	o:value("1", translate("Monday"))
	o:value("2", translate("Tuesday"))
	o:value("3", translate("Wednesday"))
	o:value("4", translate("Thursday"))
	o:value("5", translate("Friday"))
	o:value("6", translate("Saturday"))
	o:value("0", translate("Sunday"))
	o:depends({sec_wrn_period = "week", sec_enb_conn = "1"})

e = s:taboption("secondarytab", Value, "sec_wrn_number", translate("Phone number"), translate("A phone number to send warning SMS message to, e.g. +37012345678"))
	e:depends({sec_enb_conn = "1"})


--------------------------------------------------------------------------------
-------------------- Delete clear data ----------------------------------------
--------------------------------------------------------------------------------
e = s:taboption("primarytab", Value, "field5")
e.template = "cbi/legend"
e.titleText = "Clear Data Limit"

local sim_used = getParam("/sbin/gpio.sh get SIM")

local data_used1 = s:taboption("primarytab", DummyValue, "_dummy_used1", translate("Data used:"))
	data_used1.rawhtml = true

	function data_used1.cfgvalue(self, section)
		local data_used_file = io.open("/tmp/limit_total_data", "r")
		local value = "N/A"
		local postfix = ""

		if data_used_file ~= nil then
			value = data_used_file:read("*all")
			value = string.format("%.1f", value)

			if sim_used == "1" then
				if tonumber(value) >= 1000 then
					postfix = " GB"
					value = value / 1000 --MB to GB
					value = string.format("%.1f", value)
				else
					postfix = " MB"
				end
			else
				value = "N/A"
				postfix = ""
			end
		end

		return "<span style='line-height: 2.4'>" .. value .. postfix .. "</span>"
	end

local data_limit1 = s:taboption("primarytab", DummyValue, "_dummy_limit1", translate("Data limit:"))
	data_limit1.rawhtml = true

function data_limit1.cfgvalue(self, section)
	local value = m:get(section, "prim_conn_limit")
	local postfix = " MB"
	local enabled = m.uci:get(m.config, "limit", "prim_enb_conn") or "0"
	
	if enabled == "1" then
		if tonumber(value) >= 1000 then
			postfix = " GB"
			value = value / 1000 --MB to GB
			value = string.format("%.1f", value)
		end
	else
		value = "Not set"
		postfix = ""
	end

	return "<span style='line-height: 2.4'>" .. value .. postfix .. "</span>"
end

o = s:taboption("primarytab", Button, "_clear_prim")
o.template  = "admin_network/button"
o.title = translate("Clear data limit")
o.inputtitle = translate("Clear")
o.inputstyle = "apply"
o.onclick = true

e = s:taboption("secondarytab", Value, "field6")
e.template = "cbi/legend"
e.titleText = "Clear Data Limit"

local data_used2 = s:taboption("secondarytab", DummyValue, "_dummy_used2", translate("Data used:"))
	data_used2.rawhtml = true

	function data_used2.cfgvalue(self, section)
		local data_used_file = io.open("/tmp/limit_total_data", "r")
		local value = "N/A"
		local postfix = ""

		if data_used_file ~= nil then
			value = data_used_file:read("*all")
			value = string.format("%.1f", value)

			if sim_used ~= "1" then
				if tonumber(value) >= 1000 then
					postfix = " GB"
					value = value / 1000 --MB to GB
					value = string.format("%.1f", value)
				else
					postfix = " MB"
				end
			else
				value = "N/A"
				postfix = ""
			end
		end


		return "<span style='line-height: 2.4'>" .. value .. postfix .."</span>"
	end

local data_limit2 = s:taboption("secondarytab", DummyValue, "_dummy_limit2", translate("Data limit:"))
	data_limit2.rawhtml = true

function data_limit2.cfgvalue(self, section)
	local value = m:get(section, "sec_conn_limit")
	local postfix = " MB"
	local enabled = m.uci:get(m.config, "limit", "sec_enb_conn") or "0"

	if enabled == "1" then
		if tonumber(value) >= 1000 then
			postfix = " GB"
			value = value / 1000 --MB to GB
			value = string.format("%.1f", value)
		end
	else
		value = "Not set"
		postfix = ""
	end

	return "<span style='line-height: 2.4'>" .. value .. postfix .. "</span>"
end

o = s:taboption("secondarytab", Button, "_clear_sec")
o.template  = "admin_network/button"
o.title = translate("Clear data limit")
o.inputtitle = translate("Clear")
o.inputstyle = "apply"
o.onclick = true

function m.on_parse(self)
	if m:formvalue("cbid.data_limit.limit._clear_prim") then
		luci.sys.exec("/sbin/clear_data_limit.sh 1 >/dev/null 2>/dev/null")
		luci.sys.exec("logger -t mdcollectd 'SIM1 database cleared!'")
		t = {requests = "insert", table = "EVENTS", type="Web UI", text="SIM 1 database cleared"}
		eventlog:insert(t)
	elseif m:formvalue("cbid.data_limit.limit._clear_sec") then
		luci.sys.exec("/sbin/clear_data_limit.sh 0 >/dev/null 2>/dev/null")
		luci.sys.exec("logger -t mdcollectd 'SIM2 database cleared!'")
		t = {requests = "insert", table = "EVENTS", type="Web UI", text="SIM 2 database cleared"}
		eventlog:insert(t)
	end
end


function m.on_after_commit(map)
	local primEnbConn = m.uci:get(m.config, "limit", "prim_enb_conn") or "0"
	local secEnbConn = m.uci:get(m.config, "limit", "sec_enb_conn") or "0"

	if primEnbConn == "1" or secEnbConn == "1" then
		m.uci:set("mdcollectd", "config", "datalimit", "1")
		m.uci:set("mdcollectd", "config", "interval", "10")
		m.uci:set("overview", "show", "data_limit", "1")
		m.uci:commit("overview")
	else
		m.uci:set("mdcollectd", "config", "datalimit", "0")
		m.uci:set("overview", "show", "data_limit", "0")
		m.uci:commit("overview")
	end
	m.uci:save("mdcollectd")
	m.uci:commit("mdcollectd")
	luci.sys.exec("/etc/init.d/limit_guard restart >/dev/null 2>/dev/null")
end

return m
