local utl = require "luci.util"
local sys = require "luci.sys"
local m, s, s2, o

m = Map( "tcplogger", translate( "Wireless Hotspot Logging Settings" ))

m:chain("wireless")

s = m:section( NamedSection, "general", "general", translate("Logging Settings"))

s:option( Flag, "enabled", translate("Enable"),
	translate("Enable wireless traffic logging"))

s = m:section( NamedSection, "syslog", "server", translate("Syslog Server Settings"))

s:option( Flag, "enabled", translate("Enable"),
	translate("Enable wireless traffic logging to a syslog server"))

s:option( Value, "host", translate("Server address" ),
	translate("The domain name or IP address of the syslog server"))

o = s:option( Value, "port", translate("Port" ),
	translate("The TCP/IP port of the syslog server"))
o.datatype = "port"

o = s:option( ListValue, "proto", translate("Protocol" ),
	translate("Protocol of the syslog server"))
o:value("tcp", translate("TCP"))
o:value("udp", translate("UDP"))

o = s:option( Value, "prefix", translate("Prefix text"),
	translate("Prefix custom text to streamed messages"))
o.datatype = "fieldvalidation('^[a-zA-Z0-9_+.\-]+$',0)"

o = s:option( ListValue, "fproto", translate("Protocol filter"),
	translate("Filter log messages depending on protocol"))
o:value("tcp", translate("TCP"))
o:value("udp", translate("UDP"))
o:value("", translate("Any"))
o.default=""

o = s:option( Value, "fport", translate("Port filter"),
	translate("Filter log messages depending on port of port range"))
o.datatype = "portrange"

s = m:section( NamedSection, "ftp", "server", translate("FTP Server Settings"))

s:option( Flag, "enabled", translate("Enable"),
	translate("Enable wireless traffic uploading to a ftp server"))

s:option( Value, "host", translate("Server address" ),
	translate("The domain name or IP address of the server"))

o = s:option( Value, "user", translate("User name" ),
	translate("The user name of the FTP server that will be used for logs uploading"))

o = s:option( Value, "psw", translate("Password" ),
	translate("The password of the FTP server that will be used for logs uploading. Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:;[]"))
o.password = true
o.datatype = "password"

o = s:option( Value, "port", translate("Port" ),
	translate("The TCP/IP port of the server"))
o.datatype = "port"

local extra_name_info = s:option( ListValue, "extra_name_info", translate("File name extras"),
	translate("Extra information to be added to file name"))
extra_name_info:value( "none", translate("No extra information" ))
extra_name_info:value( "mac", translate("Mac address" ))
extra_name_info:value( "serial", translate("Serial number" ))
extra_name_info:value( "custom", translate("Custom string" ))

o = s:option( Value, "custom_string", translate("Custom string" ),
	translate("Custom string to be added to ftp file name"))
o:depends({ extra_name_info = "custom" })
o.datatype = "fieldvalidation('^[a-zA-Z0-9_+.\-]+$',0)"

-----------------------------------------------------------------------

s2 = m:section( TypedSection, "interval", translate("FTP Upload Settings"),
	translate("You can configure your timing settings for the log upload via FTP feature here." ))
s2.addremove = false
s2.anonymous = true

o = s2:option( ListValue, "fixed", translate("Mode"),
	translate("The schedule mode to be used for uploading to FTP server"))
o:value( "1", translate("Fixed" ))
o:value( "0", translate("Interval" ))

o = s2:option( Value, "fixed_hour", translate("Hours"),
	translate("Uploading will be performed on this specific time of the day. Range [0 - 23]"))
o.datatype = "range(0,23)"
o:depends( "fixed", "1" )

o = s2:option( Value, "fixed_minute", translate("Minutes"),
	translate("Uploading will be performed on this specific time of the day. Range [0 - 59]"))
o.datatype = "range(0,59)"
o:depends( "fixed", "1" )

o = s2:option( ListValue, "interval_time", translate("Upload interval"),
	translate("Upload logs to server every x hours"))
o:value("1", translate("1 hour"))
o:value("2", translatef("%d hours", 2))
o:value("4", translatef("%d hours", 4))
o:value("8", translatef("%d hours", 8))
o:value("12", translatef("%d hours", 12))
o:value("24", translatef("%d hours", 24))
o:depends( "fixed", "0" )

o = s2:option(StaticList, "weekdays", translate("Days"),
	translate("Uploading will be performed on these days only"))
o:value("mon",translate("Monday"))
o:value("tue",translate("Tuesday"))
o:value("wed",translate("Wednesday"))
o:value("thu",translate("Thursday"))
o:value("fri",translate("Friday"))
o:value("sat",translate("Saturday"))
o:value("sun",translate("Sunday"))

o.write = function (self, section, value)
	local nw_value = table.concat(value, ",")
	return AbstractValue.write(self, section, nw_value)
end
o.valuelist = function(self, section)
	local val = self.map:get(section, self.option)

	return val and luci.util.split(val, ",")
end

return m
