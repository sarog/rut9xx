require("luci.sys")
require("luci.sys.zoneinfo")
require("luci.tools.webadmin")
require("luci.fs")
require("luci.config")
local m, s, o

function debug(string)
	os.execute("logger " .. string)
end
m = Map("system", translate("Step - General"),
	translate("Please select your timezone."))
m:chain("network")

m.wizStep = 1
m.pass_set = "0"

s1 = m:section(TypedSection, "ntpclient", translate("Time Zone Settings"))
s1.anonymous = true
s1.addremove = false

function s1.cfgsections()
	return { "ntpclient" }
end

o = s1:option(DummyValue, "_time", translate("Current system time"), translate("Device\\'s current system time. Format [year-month-day, hours:minutes:seconds]"))
o.template = "admin_system/clock_status"

local tzone = s1:option(ListValue, "zoneName", translate("Time zone"), translate("Time zone of your country"))
tzone:value(translate("UTC"))
for i, zone in ipairs(luci.sys.zoneinfo.TZ) do
	tzone:value(zone[1])
end

function tzone.cfgvalue(self, section)
	local cfgName
	local value
	m.uci:foreach("ntpclient", "ntpclient", function(s)
			cfgName = s[".name"]
			value = s.zoneName
		end)
	return value
end

function nextStep()
	local x, a
	x = uci.cursor()
	a = x:get("system", "module", "type")
	if a == "3g" or a == "3g_ppp"  then
		luci.http.redirect(luci.dispatcher.build_url("admin/system/wizard/step-mobile"))
	else
		luci.http.redirect(luci.dispatcher.build_url("admin/system/wizard/step-lan"))
	end
	return
end

if m:formvalue("cbi.wizard.next") then
	local zone = tzone:formvalue("ntpclient")

	if zone then
		local cfgName
		local cfgTimezone

		m.uci:foreach("ntpclient", "ntpclient", function(s)
				m.uci:set("ntpclient", s[".name"], "zoneName", zone)
				m.uci:save("ntpclient")
			end)

		local function lookup_zone(title)
			for _, zone in ipairs(luci.sys.zoneinfo.TZ) do
				if zone[1] == title then return zone[2] end
			end
		end

		m.uci:foreach("system", "system", function(s)
			cfgName = s[".name"]
			cfgTimezone = s.timezone
		end)

		local timezone = lookup_zone(zone) or "GMT0"
		m.uci:set("system", cfgName, "timezone", timezone)
		m.uci:save("system")
		m.uci:commit("system")
		luci.fs.writefile("/etc/TZ", timezone .. "\n")
		luci.sys.exec("export ACTION=ifdown; sh /etc/hotplug.d/iface/20-ntpclient")
		luci.sys.exec("export ACTION=; sh /etc/hotplug.d/iface/20-ntpclient")
	end
		nextStep()
end

if m:formvalue("cbi.wizard.skip") then
	luci.http.redirect(luci.dispatcher.build_url("/admin/status/overview"))
end

return m, m2
