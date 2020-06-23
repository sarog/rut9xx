local dsp = require "luci.dispatcher"
local m, s, o
local sid = arg[1] or ""

m = Map("eventslog_report",
	translate("Events Log Report Configuration"))

m.redirect = dsp.build_url("admin/status/event/log_report")
if m.uci:get("eventslog_report", sid) ~= "rule" then
	luci.http.redirect(dsp.build_url("admin/status/event/log_report"))
	return
end

s = m:section(NamedSection, sid, "rule", translate("Modify events log file report rule"))
s.anonymous = true
s.addremove = false

o = s:option(Flag, "enable", translate("Enable"), translate("Make a rule active/inactive"))

o = s:option(ListValue, "event", translate("Events log"),
	translate("Events log for which the rule is applied"))
o:value("system", translate("System"))
o:value("network", translate("Network"))
o:value("all", translate("All"))

o = s:option(ListValue, "type", translate("Transfer type"),
	translate("Events log file transfer type"))
o:value("Email", translate("Email"))
o:value("FTP", translate("FTP"))
o:value("tcp", translate("Syslog server"))

o.validate = function (self, value, section)
	if value == "tcp" then
		local exists = false

		self.map.uci:foreach(self.config, self.sectiontype, function(s)
			if s[".name"] ~= section and s.type and s.type == "tcp" then
				exists = true
				return false
			end
		end)

		if exists then
			self.map.message = translate("err: Only one 'TCP server' rule available, please select different transfer method")
			return nil, translate("Please select different transfer method")
		end
	end

	return value
end

o = s:option(Flag, "compress", translate("Compress file"),
	translate("Compress events log file using gzip"))
o:depends("type", "Email")
o:depends("type", "FTP")

o = s:option(Value, "subject", translate("Subject"),
	translate("Subject of an email. Allowed characters (a-zA-Z0-9!@#$%&*+-/ =?^_`{|}~.:;\\)"))
o:depends("type", "Email")
o.datatype = "lengthvalidation(1, 128, '^[a-zA-Z0-9!@#$%%&*+/ =?^_`{|}~.:;-]+$')"

o = s:option(Value, "message", translate("Message"), translate("Message to send in email. Allowed characters (a-zA-Z0-9!@#$%&*+-/ =?^_`{|}~.:;\\)"))
o:depends("type", "Email")
o.datatype = "lengthvalidation(1, 128, '^[a-zA-Z0-9!@#$%%&*+/ =?^_`{|}~.:;-]+$')"


o = s:option(Value, "smtpIP", translate("SMTP server"),
	translate("SMTP (Simple Mail Transfer Protocol) server. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
o:depends("type", "Email")
o.datatype = "host"

o = s:option(Value, "smtpPort", translate("SMTP server port"),
	translate("SMTP (Simple Mail Transfer Protocol) server port"))
o:depends("type", "Email")
o.datatype = "port"

o = s:option(Flag, "secureConnection", translate("Secure connection"),
	translate("Use only if server supports SSL or TLS"))
o:depends("type", "Email")

o = s:option(Value, "host", translate("Host"),
	translate("Server host name, e.g. ftp.example.com, 192.168.123.123"))
o:depends("type", "FTP")
o:depends("type", "tcp")
o.datatype = "host"

o = s:option( Value, "port", translate("Port" ),
	translate("The port of the server"))
o.datatype = "port"
o:depends("type", "tcp")

o = s:option( ListValue, "proto", translate("Protocol" ))
o:value("tcp", "TCP")
o:value("udp", "UDP")
o:depends("type", "tcp")

o = s:option(Value, "user", translate("User name"),
	translate("User name for authentication on SMTP (Simple Mail Transfer Protocol) or FTP (File Transfer Protocol) server. Allowed characters (a-zA-Z0-9!@#$%&*+-/\\ =?^_`[({|})]~\",.;<>)"))
o.datatype = "secure_username_input"
o:depends("type", "Email")
o:depends("type", "FTP")

o = s:option(Value, "password", translate("Password"),
	translate("Password for authentication on SMTP (Simple Mail Transfer Protocol) or FTP (File Transfer Protocol) server (minimum length - 1, maximum length - 64). Allowed characters (a-zA-Z0-9!@#$%&*+/=?^_`{|}~.\-<>:;[\] )"))
o.datatype = "and(password(1), string_length(1, 64))"
o.password = true
o:depends("type", "Email")
o:depends("type", "FTP")

o = s:option(Value, "senderEmail", translate("Sender's email address"),
	translate("An address that will be used to send your email from. Allowed characters (a-zA-Z0-9._%+-)"))
o:depends("type", "Email")
o.datatype = "fieldvalidation('^[a-zA-Z0-9._%%+-]+@[a-zA-Z0-9.-]+[.][a-zA-Z]+$',0)"

o = s:option(DynamicList, "recipEmail", translate("Recipient's email address"),
	translate("For whom you want to send an email to. Allowed characters (a-zA-Z0-9._%+-)"))
o:depends("type", "Email")
function o:validate(Values)
	local smtpIP = m:formvalue("cbid.eventslog_report."..sid..".smtpIP")
	local smtpPort = m:formvalue("cbid.eventslog_report."..sid..".smtpPort")
	local senderEmail = m:formvalue("cbid.eventslog_report."..sid..".senderEmail")
	local failure

	if smtpIP == "" then
		m.message = translate("err: SMTP server field is empty!")
		failure = true
	else
		if smtpPort == "" then
			m.message = translate("err: SMTP server port field is empty")
			failure = true
		else
			if senderEmail == "" then
				m.message = translate("err: Sender's email address field is empty!")
				failure = true
			else
				for k,v in pairs(Values) do
					if not v:match("^[a-zA-Z0-9._%%+-]+@[a-zA-Z0-9.-]+[.][a-zA-Z]+$") then
						m.message = translatef("err: Recipient's email address is incorrect!")
						failure = true
					end
				end
			end
		end
	end

	if not failure then
		return Values
	end
	return nil
end

o = s:option(ListValue, "repeat", translate("Interval between reports"), translate("Send reports every select time interval"))
o:value("week", translate("Week"))
o:value("month", translate("Month"))
o:value("year", translate("Year"))
o:depends("type", "Email")
o:depends("type", "FTP")

function o:validate(Values)
	local host = m:formvalue("cbid.eventslog_report."..sid..".host")
	local user = m:formvalue("cbid.eventslog_report."..sid..".user")
	local failure

	if host == "" then
		m.message = translate("err: Host name field is empty!")
		failure = true
	else
		if user == "" then
			m.message = translate("err: Username field is empty!")
			failure = true
		end
	end
	if not failure then
		return Values
	end
	return nil
end

o = s:option(ListValue, "wday", translate("Weekday"),
	translate("Day of the week to get events log report"))
o:value("0", translate("Sunday"))
o:value("1", translate("Monday"))
o:value("2", translate("Tuesday"))
o:value("3", translate("Wednesday"))
o:value("4", translate("Thursday"))
o:value("5", translate("Friday"))
o:value("6", translate("Saturday"))
o:depends("repeat", "week")
function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
		return v
end

o = s:option(ListValue, "month", translate("Month"),
	translate("Month of the year to get events log report"))
o:value("1", translate("January"))
o:value("2", translate("February"))
o:value("3", translate("March"))
o:value("4", translate("April"))
o:value("5", translate("May"))
o:value("6", translate("June"))
o:value("7", translate("July"))
o:value("8", translate("August"))
o:value("9", translate("September"))
o:value("10", translate("October"))
o:value("11", translate("November"))
o:value("12", translate("December"))
o:depends("repeat", "year")

o = s:option(ListValue, "day", translate("Month day"),
	translate("Day of the month to get events log report"))
for i=1, 31 do
	o:value(i)
end
o:depends("repeat", "month")
o:depends("repeat", "year")

o = s:option(ListValue, "hour", translate("Hour"), translate("Hour of the day to get events log report"))
for i=1, 23 do
	o:value(i)
end
o:value("00", "24")
o:depends("type", "Email")
o:depends("type", "FTP")

return m
