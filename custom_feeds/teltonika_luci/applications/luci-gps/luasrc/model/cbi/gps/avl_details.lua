local sys = require "luci.sys"
local dsp = require "luci.dispatcher"
local utl = require "luci.util"

local m, s, o
local section_type = "avl_rule"

arg[1] = arg[1] or ""

m = Map("gps", translate("AVL Rule Data Configuration"))

m.redirect = dsp.build_url("admin/services/gps/avl")
if m.uci:get("gps", arg[1]) == "section" then
	section_type = "avl_rule_main"
elseif m.uci:get("gps", arg[1]) ~= "avl_rule" then
	luci.http.redirect(dsp.build_url("admin/services/gps/avl"))
	return
end

s = m:section(NamedSection, arg[1], section_type, "")
s.anonymous = true
s.addremove = false

--ft.opt_enabled(s, Button)
o = s:option(Flag, "enabled", translate("Enable"), translate("To enable input configuration"))
o.rmempty = false

if section_type == "avl_rule" then
	o = s:option(ListValue, "wan_status", translate("WAN"), translate("Select type on your own intended configuration"))
	o:value("mobile_both", translate("Mobile Both"))
	o:value("mobile_home", translate("Mobile Home"))
	o:value("mobile_roaming", translate("Mobile Roaming"))
	o:value("wifi", translate("WiFi"))
	o:value("wired", translate("Wired"))

	o = s:option(ListValue, "din_status", translate("Digital Isolated Input"), translate("Select type on your own intended configuration"))
	o:value("low", translate("Low logic level"))
	o:value("high", translate("High logic level"))
	o:value("both", translate("Both"))
end

o = s:option(ListValue, "priority", translate("Rule priority", translate("Select type on your own intended configuration")))
o:value("low", translate("Low priority level"))
o:value("high", translate("High priority level"))
o:value("panic", translate("Panic priority level"))
o:value("security", translate("Security priority level"))

o = s:option(Value, "collect_period", translate("Collect period"), translate("Period (in seconds) for data collection"))
o.default = "5"
o.datatype = "range(0,999999)"

o = s:option(Value, "distance", translate("Min distance"), translate("Minimal distance (in meters) change in saved records to send"))
o.default = "200"
o.datatype = "range(0,999999)"

o = s:option(Value, "angle", translate("Min angle"), translate("Minimal angle change in Degrees"))
o.default = "30"
o.datatype = "range(0,360)"

o = s:option(Value, "saved_records", translate("Min saved records"), translate("Minimal saved records count to send"))
o.default = "20"
o.datatype = "range(1,32)"

o = s:option(Value, "send_period", translate("Send period"), translate("Send period (in seconds) for data sending"))
o.default = "50"
o.datatype = "range(0,999999)"

local gps_enable = m.uci:get("gps",  arg[1], ".enabled") or "0"
function m.on_commit()
	--Delete all usr_enable from ioman config
	local gpsEnable = m:formvalue("cbid.gps." .. arg[1] .. ".enabled") or "0"
	if gpsEnable ~= gps_enable then
		m.uci:foreach("gps", "rule", function(s)
			local usr_enable = s.usr_enable or ""
			gps_inst2 = s[".name"] or ""
			if usr_enable == "1" then
				m.uci:delete("gps", gps_inst2 , "usr_enable")
			end
		end)
	end
	m.uci:save("gps")
	m.uci.commit("gps")
end

return m
