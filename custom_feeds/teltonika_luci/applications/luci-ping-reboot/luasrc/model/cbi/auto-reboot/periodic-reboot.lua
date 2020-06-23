m = Map("periodic_reboot", translate("Periodic Reboot"), translate(""))
s = m:section(NamedSection, "periodic_reboot", translate("Periodic Reboot"), translate("Periodic Reboot Setup"))
s.addremove = false

-- enable periodic reboot option
e = s:option(Flag, "enable", translate("Enable"), translate("Enable periodic reboot feature"))
function e.write(self, section, value)
	local day = m:formvalue("cbid.periodic_reboot.periodic_reboot.day")
	if  value == 1 or value == "1" then		
		if day ~= nil then
			luci.sys.call("uci set periodic_reboot.periodic_reboot.enable=1; uci commit;")
		else
			m.message = translate("err: No days selected")
			luci.sys.call("uci set periodic_reboot.periodic_reboot.enable=0; uci commit;")
		end
	else
		luci.sys.call("uci set periodic_reboot.periodic_reboot.enable=0; uci commit;")
	end
end

day = s:option(StaticList, "day", translate("Days"), translate("Periodic reboot will be performed on these days only"))
	day:value("sun",translate("Sunday"))
	day:value("mon",translate("Monday"))
	day:value("tue",translate("Tuesday"))
	day:value("wed",translate("Wednesday"))
	day:value("thu",translate("Thursday"))
	day:value("fri",translate("Friday"))
	day:value("sat",translate("Saturday"))

t = s:option(Value, "hours", translate("Hours"), translate("Periodic reboot will be performed on this specific time of the day. Range [0 - 23]"))
t.default = "23"
t.datatype = "and(range(0, 23), lengthvalidation(1,2,'^[0-9]+$'))"
t.rmempty = false

-- period interval min
t = s:option(Value, "minutes", translate("Minutes"), translate("Periodic reboot will be performed on this specific time of the day. Range [0 - 59]"))
t.default = "0"
t.datatype = "and(range(0, 59), lengthvalidation(1,2,'^[0-9]+$'))"
t.rmempty = false

return m
