--[[
Copyright 2015 Teltonika
]]--

local ds = require "luci.dispatcher"
local fw = require "luci.model.firewall"

local m, s
local fw_section

function logger(string)
	os.execute("logger \"" ..string .. "\"")
end
m = Map("portscan", translate("Port Scan Prevention"), translate(""))

s = m:section(TypedSection, "port_scan", translate("Port Scan"))

o = s:option(Flag, "enable", translate("Enable"), translate("Enable port scan prevention."))

o = s:option(Value, "seconds", translate("Interval"), translate("Time interval in seconds, counting how much port scan."))
o.datatype = "range(10,60)"
o.rmempty = false

o = s:option(Value, "hitcount", translate("Scan count"), translate("How much port can scan before bloked."))
o.datatype = "range(5,65534)"
o.rmempty = false


s = m:section(TypedSection, "defending", translate("Defending type"))

o = s:option(Flag, "syn_fin", translate("SYN-FIN attack"), translate("Protect from SYN-FIN attack."))
o = s:option(Flag, "syn_rst", translate("SYN-RST attack"), translate("Protect from SYN-RST attack."))
o = s:option(Flag, "x_max", translate("X-Mas attack"), translate("Protect from X-Mas attack."))
o = s:option(Flag, "nmap_fin", translate("FIN scan"), translate("Protect from nmap FIN scan."))
o = s:option(Flag, "null_flags", translate("NULLflags attack"), translate("Protect from NULLflags attack."))

return m