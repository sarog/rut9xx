require("luci.sys")
require("luci.sys.zoneinfo")
require("luci.tools.webadmin")
require("luci.fs")
require("luci.config")

local m, s, o
local has_ntpd = luci.fs.access("/usr/sbin/ntpd")

m = Map("system", translate("NTP"), translate("Hostname, NTP and timezone configuration."))
m:chain("luci")

s = m:section(TypedSection, "system", translate("System Properties"))
s.anonymous = true
s.addremove = false

o = s:option(DummyValue, "_systime", translate("Local Time"))
o.template = "admin_system/clock_status"


o = s:option(Value, "hostname", translate("Hostname"))
o.datatype = "hostname"

function o.write(self, section, value)
	Value.write(self, section, value)
	luci.sys.hostname(value)
end


o = s:option(ListValue, "zonename", translate("Timezone"))
o:value("UTC")

for i, zone in ipairs(luci.sys.zoneinfo.TZ) do
	o:value(zone[1])
end

function o.write(self, section, value)
	local function lookup_zone(title)
		for _, zone in ipairs(luci.sys.zoneinfo.TZ) do
			if zone[1] == title then return zone[2] end
		end
	end

	AbstractValue.write(self, section, value)
	local timezone = lookup_zone(value) or "GMT0"
	self.map.uci:set("system", section, "timezone", timezone)
	luci.fs.writefile("/etc/TZ", timezone .. "\n")
end

--
-- NTP
--

if has_ntpd then

	-- timeserver setup was requested, create section and reload page
	if m:formvalue("cbid.system._timeserver._enable") then
		m.uci:section("system", "timeserver", "ntp",
			{
                	server = { "0.europe.pool.ntp.org", "3.europe.pool.ntp.org" }
			}
		)

		m.uci:save("system")
		luci.http.redirect(luci.dispatcher.build_url("admin/services/ntpc-service", arg[1]))
		return
	end

	local has_section = false
	m.uci:foreach("system", "timeserver", 
		function(s) 
			has_section = true 
			return false
	end)

	if not has_section then

		s = m:section(TypedSection, "timeserver", translate("Time Synchronization"))
		s.anonymous   = true
		s.cfgsections = function() return { "_timeserver" } end

		x = s:option(Button, "_enable")
		x.title      = translate("Time Synchronization is not configured yet.")
		x.inputtitle = translate("Setup Time Synchronization")
		x.inputstyle = "apply"

	else
		
		s = m:section(TypedSection, "timeserver", translate("Time Synchronization"))
		s.anonymous = true
		s.addremove = false

		o = s:option(Flag, "enable", translate("Enable builtin NTP"))
		o.rmempty = false

		function o.cfgvalue(self)
			return luci.sys.init.enabled("sysntpd")
				and self.enabled or self.disabled
		end

		function o.write(self, section, value)
			if value == self.enabled then
				luci.sys.init.enable("sysntpd")
				luci.sys.call("env -i /etc/init.d/sysntpd start >/dev/null")
			else
				luci.sys.call("env -i /etc/init.d/sysntpd stop >/dev/null")
				luci.sys.init.disable("sysntpd")
			end
		end


		o = s:option(DynamicList, "server", translate("NTP server candidates"))
		o.datatype = "host"
		o:depends("enable", "1")

		-- retain server list even if disabled
		function o.remove() end

	end
end


return m
