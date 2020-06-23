--[[
Teltonika R&D. ver 0.1
]]--


local fs = require "nixio.fs"
local fw = require "luci.model.firewall"
require("luci.fs")
require("luci.config")

local logDir, o, needReboot = false
local deathTrap = { }
m = Map("system", translate("Troubleshoot Settings"), translate(""))

s2 = m:section(TypedSection, "system", translate("Troubleshoot"))
s2.addremove = false

con = s2:option(ListValue, "conloglevel", translate("System log level"), translate("You can watch logs by choosing the group from dropdown list and clicking button show"))
con:value(8, translate("Debug"))
con:value(7, translate("Info"))
con:value(6, translate("Notice"))
con:value(5, translate("Warning"))
con:value(4, translate("Error"))
con:value(3, translate("Critical"))
con:value(2, translate("Alert"))
con:value(1, translate("Emergency"))

--
-- log to flash or ram
--
logDir = s2:option(ListValue, "log_type", translate("Save log in"), translate("Specifies where logs will be saved. To apply setting router must be rebooted"))
logDir:value("circular", translate("RAM memory"))
logDir:value("file", translate("Flash memory"))

function logDir.write(self, section, value)
	Value.write(self, section, value)
	if value == "file" then
		m.uci:set("system", section, "log_file", "/usr/var/log/messages")
		m.uci:set("system", section, "log_size", "1000")
	else
		m.uci:delete("system", section, "log_file")
		m.uci:delete("system", section, "log_size")
	end
	m.uci:commit("system")
	needReboot = true
end

if luci.tools.status.show_mobile() then
--
-- enable gsmd log
--
o = s2:option(Flag, "enable_gsmd_log", translate("Include GSMD information"), translate("Check to include GSMD information to logs"))
o.rmempty = false

--
-- enable pppd log
--
	o = s2:option(Flag, "enable_pppd_debug", translate("Include PPPD information"), translate("Check to include PPPD information to logs"))
	o.rmempty = false

--
-- enable chat log
--
o = s2:option(Flag, "enable_chat_log", translate("Include chat script information"), translate("Check to include chat script information to logs"))
o.rmempty = false

end

o2 = s2:option(Flag, "enable_topology", translate("Include network topology information"), translate("Check to include network topology information"))
o2.rmempty = false
o2.default = "0"

function o2.write(self, section, value)

end

conLog = s2:option(Button, "_log")
conLog.title      = translate("System log")
conLog.inputtitle = translate("Show")
conLog.inputstyle = "apply"

conLog = s2:option(Button, "_kerlog")
conLog.title      = translate("Kernel log")
conLog.inputtitle = translate("Show")
conLog.inputstyle = "apply"

conLog = s2:option(Button, "_download")
conLog.title      = translate("Troubleshoot file")
conLog.inputtitle = translate("Download")
conLog.inputstyle = "apply"
conLog.timeload = true
conLog.onclick = true
conLog.template = "admin_system/dwbutton"


conLog = s2:option(Button, "_downloadpcap")
conLog.title      = translate("TCP dump file")
conLog.inputtitle = translate("Download")
conLog.inputstyle = "apply"
conLog.timeload = true
conLog.onclick = true
conLog.template = "admin_system/dwbutton"



tcp = s2:option(Flag, "tcp_dump", translate("Enable TCP dump"), translate("Used for packet analysis."))
tcp.default = "0"

local tcp_ifaces = luci.sys.exec("tcpdump -D | awk -F'.' '{print $2}' | awk '{print $1}'")

function string:split( inSplitPattern, outResults )
   if not outResults then
	  outResults = {}
   end

   local theStart = 1
   local theSplitStart, theSplitEnd = string.find( self, inSplitPattern, theStart )
   while theSplitStart do
	  table.insert( outResults, string.sub( self, theStart, theSplitStart-1 ) )
	  theStart = theSplitEnd + 1
	  theSplitStart, theSplitEnd = string.find( self, inSplitPattern, theStart )
   end
   return outResults
end

local ifaces_table = string.split(tcp_ifaces, "\n" )
tcp1 = s2:option(ListValue, "tcp_dump_interface", translate("Select interface"), translate("Interface to collect packets data."))
for i,v in ipairs(ifaces_table) do tcp1:value(v) end
tcp1.default = "any"
tcp1:depends("tcp_dump", "1")

tcp1 = s2:option(ListValue, "tcp_dump_filter", translate("Select protocol filter"), translate("Tracked packets type."))
ifaces_table = {"icmp", "tcp", "udp", "arp"}
tcp1:value("", translate("All"))
for i,v in ipairs(ifaces_table) do tcp1:value(v) end
tcp1:depends("tcp_dump", "1")

tcp1 = s2:option(ListValue, "tcp_inout", translate("Select packets direction"), translate("Packets direction in router"))
tcp1:value("inout", translate("IN/OUT"))
tcp1:value("in", translate("Incoming"))
tcp1:value("out", translate("Outgoing"))
tcp1:depends("tcp_dump", "1")

tcp1 = s2:option(Value, "tcp_host", translate("Host"), translate("Enter ip or hostname to track"))
tcp1:depends("tcp_dump", "1")


tcp1 = s2:option(Value, "tcp_port", translate("Port"), translate("Enter port to track"))
tcp1:depends("tcp_dump", "1")

local mounts = luci.sys.mounts()

tcp1 = s2:option(ListValue, "tcp_mount", translate("Select storage"), translate("Storage to save a dump file"))
tcp1:value("/tmp", translate("Internal storage"))
for i,v in ipairs(mounts) do tcp1:value(v.mountpoint) end
tcp1:depends("tcp_dump", "1")

local last_save = m:formvalue("cbid.system.system.tcp_mount")
if last_save ~= nil then
	luci.sys.call("uci -q set system.system.tcpdump_last_save=\""..last_save.."\"")
end
--~ ----------------------Pcap download--------------------------
if m:formvalue("cbid.system.system._downloadpcap") then
	local location = luci.util.trim(luci.sys.exec("uci -q get system.system.tcpdump_last_save"))
	if location ~= nil then
		local file_found=io.open(location.."/tcpdebug.pcap", "r")
		if file_found==nil then
			m.message = translate("err: No captured file found.")
		else
			luci.http.redirect(luci.dispatcher.build_url("admin/system/tcpdumppcap"))
		end
	else
		m.message = translate("err: No TCP dump file recorded.")
	end
end
-----------------------Troubleshoot download-----------------
if m:formvalue("cbid.system.system._download") then
	local p1 = luci.http.formvalue("cbid.system.system.enable_topology")
	if p1 == "1" then
		luci.http.redirect(luci.dispatcher.build_url("admin/system/trdownload1"))
	else
		luci.http.redirect(luci.dispatcher.build_url("admin/system/trdownload"))
	end
end

if m:formvalue("cbid.system.system._log") then
	luci.http.redirect(luci.dispatcher.build_url("admin/status/syslog"))
end

if m:formvalue("cbid.system.system._kerlog") then
	luci.http.redirect(luci.dispatcher.build_url("admin/status/dmesg"))
end


function m.on_after_commit(self)
    -- do something if the UCI configuration got committed
	luci.sys.call("/etc/init.d/log restart")
	luci.http.redirect(luci.dispatcher.build_url("admin","system","admin","troubleshoot"))
end

luci.sys.call("/sbin/luci-reload >/dev/null 2>&1 &")

return m, m2, m3
