local utl = require "luci.util"
local dual_sim = utl.trim(luci.sys.exec("uci get hwinfo.hwinfo.dual_sim"))

-- SIM IDLE
local m, s, en, o, ping, en2, ping2, period, weekd, day, hours, minutes, test, s1, c, c1, pack, pack1

m = Map("sim_idle_protection", translate("SIM Idle Protection Configuration"))
s = m:section(NamedSection, "sim1", "", translate(""))
s.addremove = false
s:tab("sim1", translate("SIM1"))
s:tab("sim2", translate("SIM2"))
s.template = "cbi/sim_manage_tabs_switch"
-- SIM IDLE Protection enable
-- SIM conf

en = s:taboption("sim1", Flag, "enable", translate("Enable"), translate("Enable SIM Idle protection feature for SIM1"))
en.rmempty = false
	
function en.write(self, section, value)
	if value then
		m.uci:set(self.config, section, self.option, value)
	end
	m.uci:save(self.config)
	m.uci:commit(self.config)
end

o = s:taboption("sim1", ListValue, "period", translate("Period"), translate(""))
o:value("month", translate("Month"))
o:value("week", translate("Week"))

o = s:taboption("sim1", ListValue, "day", translate("Day"), translate(""))
o:depends({period = "month"})
for i=1,31 do
	o:value(i,i)
end

o = s:taboption("sim1", ListValue, "weekday", translate("Day"), translate(""))
o:value("1", translate("Monday"))
o:value("2", translate("Tuesday"))
o:value("3", translate("Wednesday"))
o:value("4", translate("Thursday"))
o:value("5", translate("Friday"))
o:value("6", translate("Saturday"))
o:value("0", translate("Sunday"))
o:depends({period = "week"})

o = s:taboption("sim1", ListValue, "hour" ,translate("Hour"), translate(""))
o:depends({period = "month"})
o:depends({period = "week"})
for i=1,23 do
	o:value(i,i)
end
o:value("0","24")

o = s:taboption("sim1", ListValue, "min", translate("Minute"), translate(""))
o:depends({period = "month"})
o:depends({period = "week"})
for i=0,59 do
	o:value(i,i)
end

function o.write(self, section, value)
	if value then
		m.uci:set(self.config, section, self.option, value)
	end
	m.uci:commit(self.config)
end

ping = s:taboption("sim1", Value, "host", translate("Host to ping"), translate("IP address or domain name which will be used to send ping packets to. E.g. 192.168.1.1 (or www.host.com if DNS server is configured correctly)"))
ping.default = "127.0.0.1"
ping.datatype = "ipaddr"

function ping.write(self, section, value)
	if value then 
		m.uci:set(self.config, section, self.option, value)
	end
	m.uci:commit(self.config)
end

pack = s:taboption("sim1", Value, "packet_size", translate("Ping package size"), translate("Ping package size in bytes. Range [1 - 1000]"))
pack.default = "56"
pack.datatype = "range(1,1000)"

function pack.write(self, section, value)
	if value then
		m.uci:set(self.config, section, self.option, value)
	end
	m.uci:commit(self.config)
end

c = s:taboption("sim1", Value, "count", translate("Ping requests"), translate("How many requests will be sent. Range [1 - 30]"))
c.default = "2"
c.datatype = "range(1,30)"

function c.write(self, section, value)
	if value then
		m.uci:set(self.config, section, self.option, value)
	end
	m.uci:commit(self.config)
end

------------SIM2 CONF-----------
if dual_sim == "1" then

	en2 = s:taboption("sim2", Flag, "enable2", translate("Enable"), translate("Enable SIM Idle protection feature for SIM2"))
	en2.rmempty = false
	
	function en2.cfgvalue(self, section)
		value = m.uci:get(self.config, "sim2", "enable")
		if value then
			return value
		end
	end
	
	function en2.write(self, section, value)
		if value then
			m.uci:set(self.config, "sim2", "enable", value)
		end
		m.uci:save(self.config)
		m.uci:commit(self.config)
	end

	period = s:taboption("sim2", ListValue, "period2", translate("Period"), translate(""))
	period:value("month", translate("Month"))
	period:value("week", translate("Week"))

	day = s:taboption("sim2", ListValue, "day2", translate("Day"), translate(""))
	day:depends({period2 = "month"})
	for i=1,31 do
 	       day:value(i,i)
	end

	weekd = s:taboption("sim2", ListValue, "weekday2", translate("Day"), translate(""))
	weekd:value("1", translate("Monday"))
	weekd:value("2", translate("Tuesday"))
	weekd:value("3", translate("Wednesday"))
	weekd:value("4", translate("Thursday"))
	weekd:value("5", translate("Friday"))
	weekd:value("6", translate("Saturday"))
	weekd:value("0", translate("Sunday"))
	weekd:depends({period2 = "week"})

	hours = s:taboption("sim2", ListValue, "hour2" ,translate("Hour"), translate(""))
	hours:depends({period2 = "month"})
	hours:depends({period2 = "week"})
	for i=1,23 do
	        hours:value(i,i)
	end
	hours:value("0","24")

	minutes = s:taboption("sim2", ListValue, "min2", translate("Minute"), translate(""))
	minutes:depends({period2 = "month"})
	minutes:depends({period2 = "week"})
	for i=0,59 do
	        minutes:value(i,i)
	end
	
------------functions to get the configuration------------------------

	function period.cfgvalue(self, section)
		periodd = m.uci:get(self.config, "sim2", "period")
		if periodd then 
			return periodd
		end
	end

	function weekd.cfgvalue(self, section)
		weekdd = m.uci:get(self.config, "sim2", "weekday")
		if weekdd then
			return weekdd
		end
	end

	function day.cfgvalue(self, section)
		dayy = m.uci:get(self.config, "sim2", "day")
		if dayy then
			return dayy
		end
	end
	function hours.cfgvalue(self, section)
		hourss = m.uci:get(self.config, "sim2", "hour")
		if hourss then
			return hourss
		end
	end
	function minutes.cfgvalue(self, section)
		minutess = m.uci:get(self.config, "sim2", "min")
		if minutess then
			return minutess
		end
	end

---------------------WRITE functions-----------------------------------------
	
	function period.write(self, section, value)
		if value then
			m.uci:set(self.config, "sim2", "period", value)
		end
		if value == "month" then
			m.uci:delete(self.config, "sim2", "weekday")
			m.uci:save(self.config)
			m.uci:commit(self.config)
		elseif value == "week" then
			m.uci:delete(self.config, "sim2", "day")
			m.uci:save(self.config)
			m.uci:commit(self.config)
		end
		m.uci:save(self.config)
		m.uci:commit(self.config)
	end
	
	function weekd.write(self, section, value)
		if value then
			m.uci:set(self.config, "sim2", "weekday", value)
		end
		m.uci:save(self.config)
		m.uci:commit(self.config)
	end

	function day.write(self, section, value)
		if value then
			m.uci:set(self.config, "sim2", "day", value)
		end
		m.uci:save(self.config)
		m.uci:commit(self.config)
	end

	function hours.write(self, section, value)
		if value then
			m.uci:set(self.config, "sim2", "hour", value)
		end
		m.uci:save(self.config)
		m.uci:commit(self.config)
	end

	function minutes.write(self, section, value)
		if value then
			m.uci:set(self.config, "sim2", "min", value)
		end
		m.uci:save(self.config)
		m.uci:commit(self.config)
	end

	ping2 = s:taboption("sim2", Value, "host2", translate("Host to ping"), translate("IP address or domain name which will be used to send ping packets to. E.g. 192.168.1.1 (or www.host.com if DNS server is configured correctly)"))
	ping2.default = "127.0.0.1"
	ping2.datatype = "ipaddr"

	function ping2.cfgvalue(self, section)
		value = m.uci:get(self.config, "sim2", "host")
		if value then
			return value
		end
	end
	
	function ping2.write(self, section, value)
		if value then
			m.uci:set(self.config, "sim2", "host", value)
		end
		m.uci:commit(self.config)
	end
	
	pack1 = s:taboption("sim2", Value, "packet_size2", translate("Ping package size"), translate("Ping package size in bytes. Range [1 - 1000]"))
	pack1.default = "56"
	pack1.datatype = "range(1,1000)"

	function pack1.cfgvalue(self, section)
		value = m.uci:get(self.config, "sim2", "packet_size")
		if value then
			return value
		end
	end

	function pack1.write(self, section, value)
		if value then
			m.uci:set(self.config, "sim2", "packet_size", value)
		end
		m.uci:commit(self.config)
	end
end

	c1 = s:taboption("sim2", Value, "count2", translate("Ping requests"), translate("How many requests will be sent. Range [1 - 30]"))
	c1.default = "2"
	c1.datatype = "range(1,30)"

	function c1.cfgvalue(self, section)
		value = m.uci:get(self.config, "sim2", "count")
		if value then
			return value
		end
	end

	function c1.write(self, section, value)
		if value then
			m.uci:set(self.config, "sim2", "count", value)
		end
		m.uci:commit(self.config)
	end

return m
