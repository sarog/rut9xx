local dsp = require "luci.dispatcher"

arg[1] = arg[1] or ""

m = Map( "output_control", translate( "Periodic Output Control" ), translate( "" ) )
m.redirect = dsp.build_url("admin/services/input-output/output/periodic/")

is_4pin = m.uci:get("hwinfo", "hwinfo", "4pin_io") or "0"
is_io = m.uci:get("hwinfo", "hwinfo", "in_out") or "0"

snt = m:section( NamedSection, arg[1], "rule", translate("Edit Output Control Rule"), translate("" ))
snt.addremove = false
snt.anonymous = true

enb = snt:option(Flag, "enabled", translate("Enable"), translate("Enable output control configuration"))

pin = snt:option(ListValue, "gpio", translate("Output"), translate("Specifies for which output type rule will be applied"))
	if is_io == "1" then
		pin:value("DOUT1", "Digital OC output")
		pin:value("DOUT2", "Digital relay output")
	end
	if is_4pin == "1" then
		pin:value("DOUT3", "Digital 4PIN output")
	end

act = snt:option(ListValue, "action", translate("Action"), translate("Specifies what action will happen"))
	act:value("on", translate("On"))
	act:value("off", translate("Off"))

del = snt:option(Flag, "timeout", translate("Action timeout"), translate("Specifies if action should end after some time"))
	del.datatype = "integer"

local timeout_enabled = m:formvalue("cbid.output_control." .. arg[1] .. ".timeout")
tim = snt:option(Value, "timeout_time", translate("Timeout (sec)"), translate("Specifies after how much time this action should end"))
	tim:depends("timeout", "1")
	tim.datatype = "integer"
	if timeout_enabled and timeout_enabled == "1" then
		tim.rmempty = false
	end

tns = snt:option( ListValue, "mode", translate("Mode"), translate("Repetition mode. It can be fixed (happens at specified time) or interval (happens constantly after specified time from each other)"))
	tns:value( "fixed", translate("Fixed" ))
	tns:value( "interval", translate("Interval" ))

thr = snt:option( Value, "fixed_hour", translate("Hours"), translate("Specifies exact hour"))
	thr.datatype = "string_not_empty"
	thr:depends( "mode", "fixed" )

function thr.validate(self, value)
	if not tonumber(value) then
		return nil, "Bad hour format"
	end
	if tns.value == "fixed" and tonumber(value) < 0 or tonumber(value) > 23 then
		return nil, "Hours out of range"
	end
	return value
end

tmn = snt:option( Value, "fixed_minute", translate("Minutes"), translate("Specifies exact minutes"))
	tmn:depends( "mode", "fixed" )
	tmn.datatype = "string_not_empty"

function tmn.validate(self, value)
	if not tonumber(value) then
		return nil, "Bad minute format"
	end
	if tns.value == "fixed" and tonumber(value) < 0 or tonumber(value) > 59 then
		return nil, "Minutes out of range"
	end
	return value
end

time = snt:option(ListValue, "interval_time", translate("Interval"), translate("Specifies the interval of the selected action"))
	time:depends("mode", "interval")
	time:value("1", translate("1 min"))
	time:value("2", translate("2 mins"))
	time:value("3", translate("3 mins"))
	time:value("4", translate("4 mins"))
	time:value("5", translate("5 mins"))
	time:value("10", translate("10 mins"))
	time:value("15", translate("15 mins"))
	time:value("30", translate("30 mins"))
	time:value("60", translate("1 hour"))
	time:value("120", translate("2 hours"))
	time:value("180", translate("3 hours"))
	time:value("240", translate("4 hours"))
	time:value("360", translate("6 hours"))
	time:value("480", translate("8 hours"))
	time:value("720", translate("12 hours"))

twd = snt:option(StaticList, "day", translate("Days"), translate("Specifies in which weekdays action should happen"))
	twd:value("mon",translate("Monday"))
	twd:value("tue",translate("Tuesday"))
	twd:value("wed",translate("Wednesday"))
	twd:value("thu",translate("Thursday"))
	twd:value("fri",translate("Friday"))
	twd:value("sat",translate("Saturday"))
	twd:value("sun",translate("Sunday"))

--function m.on_save()
--	local aa = twd.value
--	luci.sys.exec("echo "..aa.." >> /rer.txt")
--end

--Uzkomentavau nes kitu atveju saugant isjungiamas ijungtas output
-- function m.on_commit(map)
-- 	local relay_outpt = m:formvalue("cbid.output_control."..arg[1]..".gpio")
-- 	luci.sys.call("/sbin/gpio.sh clear "..relay_outpt)
-- end

return m
