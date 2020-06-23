local sys = require "luci.sys"
local util = require "luci.util"
--local used = util.trim(sys.exec("gsmctl -S -t | grep Used: | awk -F ' ' '{print $2}'")) or ""
--local memory = util.trim(sys.exec("gsmctl -S -t | grep Total: | awk -F ' ' '{print $2}'")) or ""
local output=sys.exec("gsmctl -S -t 2>/dev/null")
local array=output:split("\n")
if array[1] and array[2] then
	tmp=array[1]:split(" ")
	if tmp[2] then
		used=tmp[2]
	end
	if tmp[2] then
		tmp=array[2]:split(" ")
		memory=tmp[2]
	end
end

m = Map("sms_utils", translate("SMS Storing"))

s = m:section(NamedSection, "sim", "sms_utils", translate("Configuration"))

e = s:option(Flag, "enabled", translate("Save messages on SIM"), translate("Do not remove messages from SIM card after they have been read"))
e.rmempty = false

e = s:option(Label, "_memory", translate("SIM card memory"), translate("Information about used/available SIM card memory"))

function e.cfgvalue()
	local value = "N/A"
	if used and memory then
		if tonumber(used)>=0 and tonumber(memory)>=0 then
			value = "Used:" .. used .. " Available: " .. memory
		end
	end
	return value
end

e = s:option(Value, "free", translate("Leave free space"), translate("How much memory (number of messages) should be left free"))
if used and memory then
	e.datatype = "range(1, " .. memory ..")"
end
e.default = "1"
e.rmempty = false

return m
