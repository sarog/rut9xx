local sys = require "luci.sys"
local dsp = require "luci.dispatcher"
local utl = require "luci.util"
local uci = require "uci".cursor()
local show =  luci.tools.status.show_mobile()

-- local socket = require 'socket'
-- local ssl = require "ssl"
-- local https = require "ssl.https"
-- local smtp = require "socket.smtp"
-- local mime = require "mime"
-- local ltn12 = require "ltn12"

local gps = uci:get("hwinfo", "hwinfo", "gps") or "0"
local in_out = uci:get("hwinfo", "hwinfo", "in_out") or "0"
local is_io = uci:get("hwinfo", "hwinfo", "4pin_io") or "0"
local rs485 = uci:get("hwinfo", "hwinfo", "rs485") or "0"
local rs232 = uci:get("hwinfo", "hwinfo", "rs232") or "0"
local snmp = utl.trim(sys.exec("opkg list-installed | grep tlt_custom_pkg_snmpd | wc -l"))

local m, s, o

arg[1] = arg[1] or ""

m = Map("events_reporting", translate("Event Reporting Configuration"))

m.redirect = dsp.build_url("admin/status/event/report")
if m.uci:get("events_reporting", arg[1]) ~= "rule" then
	luci.http.redirect(dsp.build_url("admin/status/event/report"))
	return
else
	--local name = m:get(arg[1], "name") or m:get(arg[1], "_name")
	--if not name or #name == 0 then
	--	name = translate("(Unnamed Entry)")
	--end
	--m.title = "%s - %s" %{ translate("Firewall - Port Forwards"), name }
end

s = m:section(NamedSection, arg[1], "rule", translate("Modify Event Reporting Rule"))
s.anonymous = true
s.addremove = false

o = s:option(Flag, "enable", translate("Enable"), translate("Make a rule active/inactive"))
o.rmempty = false

o = s:option(ListValue, "event", translate("Event type"), translate("Event type for which the rule is applied"))
o:value("Config", translate("Config change"))
o:value("DHCP", translate("New DHCP client"))
--o:value("FW", translate("FW upgrade"))
if show then
	o:value("Mobile Data", translate("Mobile data"))
	o:value("SMS", translate("SMS"))
	o:value("SIM switch", translate("SIM switch"))
	o:value("Signal strength", translate("Signal strength"))
end
o:value("Reboot", translate("Reboot"))
o:value("SSH", translate("SSH"))
o:value("Web UI", translate("Web UI"))
o:value("WiFi", translate("New WiFi client"))
o:value("Port", translate("LAN port state"))
o:value("Backup", translate("WAN Failover"))
o:value("Restore Point", translate("Restore Point"))
if tonumber(gps)== 1 then
	o:value("GPS", translate("GPS"))
end

function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	return v
end

o = s:option(ListValue, "eventMarkC", translate("Event subtype"), translate("Event subtype for which the rule is applied"))
o:value("all", translate("All"))
--o:value("restore point", translate("Restore point"))
o:value("open vpn", translate("OpenVPN"))
if show then
	o:value("sms", translate("SMS"))
	o:value("mobile traffic", translate("Mobile traffic"))
	o:value("multiwan", translate("Multiwan"))
	o:value("sim switch", translate("SIM switch"))
	o:value("mobile", translate("Mobile"))
	o:value("data limit", translate("Data limit"))
	if tonumber(gps)== 1 then
		o:value("gps", translate("GPS"))
	end
end
o:value("events reporting", translate("Events reporting"))
o:value("periodic reboot", translate("Periodic reboot"))
if tonumber(snmp) > 0 then
	o:value("snmp", translate("SNMP"))
end
o:value("ping reboot", translate("Ping reboot"))
o:value("auto update", translate("Auto update"))
o:value("site blocking", translate("Site blocking"))
o:value("pptp", translate("PPTP"))
--o:value("administration", translate("Administration"))
o:value("hotspot", translate("Hotspot"))
if tonumber(in_out)== 1 or tonumber(is_io) == 1 then
	o:value("input/output", translate("Input/output"))
end

o:value("content blocker", translate("Content blocker"))
o:value("login page", translate("Login page"))
o:value("language", translate("Language"))
o:value("profile", translate("Profile"))
o:value("ddns", translate("DDNS"))
o:value("ipsec", translate("IPsec"))
o:value("access control", translate("Access control"))
o:value("dhcp", translate("DHCP"))
if tonumber(rs232)== 1 or tonumber(rs485)== 1 then
	o:value("rs232/rs485", translate("RS232/RS485"))
end

o:value("vrrp", translate("VRRP"))
o:value("ssh", translate("SSH"))
o:value("network", translate("Network"))
o:value("wireless", translate("Wireless"))
o:value("firewall", translate("Firewall"))
o:value("ntp", translate("NTP"))
o:value("l2tp", translate("L2TP"))
o:value("other", translate("Other"))
o:depends("event", "Config")
o.default = m.uci:get("events_reporting", arg[1], "eventMark")

function o.write(self, section, value)
	local ev = luci.http.formvalue("cbid.events_reporting."..arg[1]..".event")
	if ev == "Config" then
		m.uci:set("events_reporting", section, "eventMark", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end

o = s:option(ListValue, "eventMarkD", translate("Event subtype"), translate("Event subtype for which the rule is applied"))
o:value("all", translate("All"))
o:value("wifi", translate("Connected from WiFi"))
o:value("lan", translate("Connected from LAN"))
o:depends("event", "DHCP")
o.default = m.uci:get("events_reporting", arg[1], "eventMark")
function o.write(self, section, value)
	local ev = luci.http.formvalue("cbid.events_reporting."..arg[1]..".event")
	if ev == "DHCP" then
		m.uci:set("events_reporting", section, "eventMark", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end

--o = s:option(ListValue, "eventMarkFw", translate("Event subtype"), translate("Event subtype for which the rule is applied"))
--o:value("all", translate("All"))
--o:value("file", translate("From file"))
--o:value("server", translate("From server"))
--o:depends("event", "FW")
--o.default = m.uci:get("events_reporting", arg[1], "eventMark")
--function o.write(self, section, value)
--	local ev = luci.http.formvalue("cbid.events_reporting."..arg[1]..".event")
--	if ev == "FW" then
--		m.uci:set("events_reporting", section, "eventMark", value)
--		m.uci:save("events_reporting")
--		m.uci:commit("events_reporting")
--	end
--end

o = s:option(ListValue, "eventMarkM", translate("Event subtype"), translate("Event subtype for which the rule is applied"))
o:value("all", translate("All"))
o:value(" connected", translate("Connected"))
o:value("disconnected", translate("Disconnected"))
o:depends("event", "Mobile Data")
o.default = m.uci:get("events_reporting", arg[1], "eventMark")
function o.write(self, section, value)
	local ev = luci.http.formvalue("cbid.events_reporting."..arg[1]..".event")
	if ev == "Mobile Data" then
		m.uci:set("events_reporting", section, "eventMark", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end

o = s:option(ListValue, "eventMarkR", translate("Event subtype"), translate("Event subtype for which the rule is applied"))
o:value("all", translate("All"))
o:value("Boot start up, reason unknown", translate("After unexpected shut down"))
o:value("fw upgrade", translate("After FW upgrade"))
o:value("web ui", translate("From Web UI"))
if show then
	o:value("sms", translate("From SMS"))
	o:value("call", translate("From Call"))
end
o:value("input/output", translate("From Input/output"))
o:value("ping reboot", translate("From ping reboot"))
o:value("periodic reboot", translate("From periodic reboot"))
o:value("from button", translate("From button"))
-- o:value("factory reset button", translate("After factory reset button"))
-- o:value("restore", translate("After restore"))
o:depends("event", "Reboot")
o.default = m.uci:get("events_reporting", arg[1], "eventMark")
function o.write(self, section, value)
	local ev = luci.http.formvalue("cbid.events_reporting."..arg[1]..".event")
	if ev == "Reboot" then
		m.uci:set("events_reporting", section, "eventMark", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end

o = s:option(ListValue, "eventMarkSMS", translate("Event subtype"), translate("Event subtype for which the rule is applied"))
-- o:value("all", translate("All"))
-- o:value("sent", translate("SMS sent"))
o:value("received from", translate("SMS received"))
o:depends("event", "SMS")
o.default = m.uci:get("events_reporting", arg[1], "eventMark")
function o.write(self, section, value)
	local ev = luci.http.formvalue("cbid.events_reporting."..arg[1]..".event")
	if ev == "SMS" then
		m.uci:set("events_reporting", section, "eventMark", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end

o = s:option(ListValue, "eventMarkSSH", translate("Event subtype"), translate("Event subtype for which the rule is applied"))
o:value("all", translate("All"))
o:value("succeeded", translate("Successful authentication"))
o:value("bad", translate("Unsuccessful authentication"))
o:depends("event", "SSH")
o.default = m.uci:get("events_reporting", arg[1], "eventMark")
function o.write(self, section, value)
	local ev = luci.http.formvalue("cbid.events_reporting."..arg[1]..".event")
	if ev == "SSH" then
		m.uci:set("events_reporting", section, "eventMark", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end

o = s:option(ListValue, "eventMarkWUI", translate("Event subtype"), translate("Event subtype for which the rule is applied"))
o:value("all", translate("All"))
o:value("was successful", translate("Successful authentication"))
o:value("not successful", translate("Unsuccessful authentication"))
o:depends("event", "Web UI")
o.default = m.uci:get("events_reporting", arg[1], "eventMark")
function o.write(self, section, value)
	local ev = luci.http.formvalue("cbid.events_reporting."..arg[1]..".event")
	if ev == "Web UI" then
		m.uci:set("events_reporting", section, "eventMark", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end

o = s:option(ListValue, "eventMarkWF", translate("Event subtype"), translate("Event subtype for which the rule is applied"))
o:value("all", translate("All"))
o:value("connected", translate("Connected"))
o:value("disconnected", translate("Disconnected"))
o:depends("event", "WiFi")
o.default = m.uci:get("events_reporting", arg[1], "eventMark")
function o.write(self, section, value)
	local ev = luci.http.formvalue("cbid.events_reporting."..arg[1]..".event")
	if ev == "WiFi" then
		m.uci:set("events_reporting", section, "eventMark", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end

o = s:option(ListValue, "eventMarkGPS", translate("Event subtype"), translate("Event subtype for which the rule is applied"))
o:value("all", translate("All"))
o:value("left geofence", translate("Left geofence"))
o:value("entered geofence", translate("Entered geofence"))
o:depends("event", "GPS")
o.default = m.uci:get("events_reporting", arg[1], "eventMark")

function o.write(self, section, value)
	local ev = luci.http.formvalue("cbid.events_reporting."..arg[1]..".event")
	if ev == "GPS" then
		m.uci:set("events_reporting", section, "eventMark", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end

o = s:option(ListValue, "eventMarkSIMSW", translate("Event subtype"), translate("Event subtype for which the rule is applied"))
o:value("all", translate("All"))
o:value("SIM 1 to SIM 2", translate("From SIM 1 to SIM 2"))
o:value("SIM 2 to SIM 1", translate("From SIM 2 to SIM 1"))
o:depends("event", "SIM switch")
o.default = m.uci:get("events_reporting", arg[1], "eventMark")
function o.write(self, section, value)
	local ev = luci.http.formvalue("cbid.events_reporting."..arg[1]..".event")
	if ev == "SIM switch" then
		m.uci:set("events_reporting", section, "eventMark", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end

o = s:option(ListValue, "eventMarkPort", translate("Event subtype"), translate("Event subtype for which the rule is applied"))
o:value("all", translate("All"))
o:value("unplugged", translate("Unplugged"))
o:value("plugged in", translate("Plugged in"))
o:depends("event", "Port")
o.default = m.uci:get("events_reporting", arg[1], "eventMark")
function o.write(self, section, value)
	local ev = luci.http.formvalue("cbid.events_reporting."..arg[1]..".event")
	if ev == "Port" then
		m.uci:set("events_reporting", section, "eventMark", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end

o = s:option(ListValue, "eventMarkBackup", translate("Event subtype"), translate("Event subtype for which the rule is applied"))
o:value("all", translate("All"))
o:value("main", translate("Switched to main"))
o:value("backup", translate("Switched to failover"))
o:depends("event", "Backup")
o.default = m.uci:get("events_reporting", arg[1], "eventMark")
function o.write(self, section, value)
	local ev = luci.http.formvalue("cbid.events_reporting."..arg[1]..".event")
	if ev == "Backup" then
		m.uci:set("events_reporting", section, "eventMark", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end

o = s:option(ListValue, "eventMarkRestorePoint", translate("Event subtype"), translate("Event subtype for which the rule is applied"))
o:value("all", translate("All"))
o:value("download", translate("Save"))
o:value("restore", translate("Load"))
o:depends("event", "Restore Point")
o.default = m.uci:get("events_reporting", arg[1], "eventMark")
function o.write(self, section, value)
	local ev = luci.http.formvalue("cbid.events_reporting."..arg[1]..".event")
	if ev == "Restore Point" then
		m.uci:set("events_reporting", section, "eventMark", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end

o = s:option(ListValue, "eventMarksignal", translate("Event subtype"), translate("Event subtype for which the rule is applied"))
o:value("all", translate("All"))
o:value("Signal strength dropped below -113 dBm", translate("-121dBm -113dBm"))
o:value("Signal strength dropped below -98 dBm", translate("-113dBm -98dBm"))
o:value("Signal strength dropped below -93 dBm", translate("-98dBm -93dBm"))
o:value("Signal strength dropped below -75 dBm", translate("-93dBm -75dBm"))
o:value("Signal strength dropped below -60 dBm", translate("-75dBm -60dBm"))
o:value("Signal strength dropped below -50 dBm", translate("-60dBm -50dBm"))
o:depends("event", "Signal strength")
o.default = m.uci:get("events_reporting", arg[1], "eventMark")

function o.write(self, section, value)
	local ev = luci.http.formvalue("cbid.events_reporting."..arg[1]..".event")
	if ev == "Signal strength" then
		m.uci:set("events_reporting", section, "eventMark", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end

o = s:option(ListValue, "action", translate("Action"), translate("Action to perform when an event occurs"))
if show then
	o:value("sendSMS", translate("Send SMS"))
end
o:value("sendEmail", translate("Send email"))
function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	return v
end

emailSend = s:option(Flag, "emailSend", translate("Enable delivery retry"), translate("Send email again if first try send message was unsuccessful."))
emailSend:depends("action", "sendEmail")
emailSend.default = "1"
emailSend.rmempty = false

checkint = s:option(ListValue, "checkint", translate("Retry interval"), translate("How much time between interval min."))
checkint:depends("emailSend", "1")
checkint:value("5", translate("5 min."))
checkint:value("10", translate("10 min."))
checkint:value("15", translate("15 min."))
checkint:value("30", translate("30 min."))
checkint:value("60", translate("60 min."))
checkint.default = "5"

email_count = s:option(ListValue, "email_count", translate("Retry count"), translate("How much time to check email server."))
email_count:depends("emailSend", "1")
email_count:value("2", translate("2"))
email_count:value("3", translate("3"))
email_count:value("4", translate("4"))
email_count:value("5", translate("5"))
email_count:value("6", translate("6"))
email_count:value("7", translate("7"))
email_count:value("8", translate("8"))
email_count:value("9", translate("9"))
email_count:value("10", translate("10"))
email_count.default = "2"

smsSend = s:option(Flag, "smsSend", translate("Enable delivery retry"), translate("Send sms again if first try send sms was unsuccessful."))
smsSend:depends("action", "sendSMS")
smsSend.default = "1"
smsSend.rmempty = false

checksmsint = s:option(ListValue, "checksmsint", translate("Retry interval"), translate("How much time between interval min."))
checksmsint:depends("smsSend", "1")
checksmsint:value("1", translate("1 min."))
checksmsint:value("5", translate("5 min."))
checksmsint:value("10", translate("10 min."))
checksmsint:value("15", translate("15 min."))
checksmsint:value("30", translate("30 min."))
checksmsint:value("60", translate("60 min."))
checksmsint.default = "5"

sms_count = s:option(ListValue, "sms_count", translate("Retry count"), translate("How much time to check connection."))
sms_count:depends("smsSend", "1")
sms_count:value("2", translate("2"))
sms_count:value("3", translate("3"))
sms_count:value("4", translate("4"))
sms_count:value("5", translate("5"))
sms_count:value("6", translate("6"))
sms_count:value("7", translate("7"))
sms_count:value("8", translate("8"))
sms_count:value("9", translate("9"))
sms_count:value("10", translate("10"))
sms_count.default = "2"

emailsub = s:option(Value, "subject", translate("Subject"), translate("Subject of an email. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
emailsub:depends("action", "sendEmail")
emailsub.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',0)"

message = s:option(Value, "message", translate("Message text on Event"), translate("Message to send. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
message.template = "events_reporting/events_textbox"
message.default = "Router name - %rn; Event type - %et; Event text - %ex; Time stamp - %ts;"
message.rows = "8"
message.indicator = arg[1]

msbf = s:option(Flag, "enable_block", translate("Enable redundancy protection"), translate("Enable timer protection from overflow of messages, due to frequent signal strength variations"))
msbf:depends("event", "Signal strength")
msbf.default = "1"
msbf.rmempty = false

rstr_timer = s:option(ListValue, "restrict_time", translate("Minimal reporting interval"), translate("Minimal time interval for sending event messages"))
rstr_timer:depends("enable_block", "1")
rstr_timer:value("1", translate("1 min"))
rstr_timer:value("2", translate("2 min"))
rstr_timer:value("5", translate("5 min"))
rstr_timer:value("10", translate("10 min"))
rstr_timer:value("15", translate("15 min"))
rstr_timer:value("30", translate("30 min"))
rstr_timer:value("60", translate("60 min"))
rstr_timer:value("120", translate("2 hours"))
rstr_timer:value("240", translate("4 hours"))
rstr_timer:value("720", translate("12 hours"))
rstr_timer.default = "5"

function message.write(self, section, value)
	local value = luci.http.formvalue("cbid.events_reporting."..arg[1]..".message")
	value = string.gsub(value, "%s", " ")
	if value then
		m.uci:set("events_reporting", section, "message", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end

stat_flag = s:option(Flag, "send_status", translate("Get status after reboot"), translate("Receive router status information"))
stat_flag:depends("event", "Reboot")

stat_e = s:option(Value, "status_msg", translate("Status message after reboot"), translate("Receive router status information. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
stat_e:depends("send_status", "1")
stat_e.template = "events_reporting/events_textbox"
stat_e.rows = "8"
stat_e.indicator = arg[1]
stat_e.default = "Router name - %rn; WAN IP - %wi; Data Connection state - %cs; Connection type - %ct; Signal strength - %ss; New FW available - %fs;"

function stat_e.write(self, section, value)
	local value = luci.http.formvalue("cbid.events_reporting."..arg[1]..".status_msg")
	value = string.gsub(value, "%s", " ")
	if value then
		m.uci:set("events_reporting", section, "status_msg", value)
		m.uci:save("events_reporting")
		m.uci:commit("events_reporting")
	end
end
--------------------------------------------------
smtpIP = s:option(Value, "smtpIP", translate("SMTP server"), translate("SMTP (Simple Mail Transfer Protocol) server. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
smtpIP:depends("action", "sendEmail")
smtpIP.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',0)"

smtpPort = s:option(Value, "smtpPort", translate("SMTP server port"), translate("SMTP (Simple Mail Transfer Protocol) server port"))
smtpPort:depends("action", "sendEmail")
smtpPort.datatype = "port"

secCon = s:option(Flag, "secureConnection", translate("Secure connection"), translate("Use only if server supports SSL or TLS"))
secCon:depends("action", "sendEmail")
secCon.rmempty= false
secCon.default="0"

username = s:option(Value, "userName", translate("User name"), translate("User name for authentication on SMTP (Simple Mail Transfer Protocol) server. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
username.noautocomplete = true
username:depends("action", "sendEmail")
username.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',0)"

passwd = s:option(Value, "password", translate("Password"), translate("Password for authentication on SMTP (Simple Mail Transfer Protocol) server. Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:; []"))
passwd.password = true
passwd.noautocomplete = true
passwd:depends("action", "sendEmail")
passwd.datatype = "password(1)"

senderEmail = s:option(Value, "senderEmail", translate("Sender's email address"), translate("An address that will be used to send your email from. Allowed characters (a-zA-Z0-9._%+-)"))
senderEmail:depends("action", "sendEmail")
senderEmail.datatype = "fieldvalidation('^[a-zA-Z0-9._%%+-]+@[a-zA-Z0-9.-]+[.][a-zA-Z]+$',0)"


o = s:option(ListValue, "recipient_format", translate("Recipients"), translate("You can choose add single numbers in a list or use User group list"))
o:depends("action", "sendSMS")
o:value("single", translate("Single number"))
o:value("group", translate("User group"))

telnum = s:option(DynamicList, "telnum", translate("Recipient's phone number"), translate("For whom you want to send a SMS to, e.g. +37012345678"))
telnum:depends("recipient_format", "single")
function telnum:validate(Values)
	local message = m:formvalue("cbid.events_reporting."..arg[1]..".message")
	local failure
	if message == "" then
		m.message = translate("err: Message field is empty!")
		failure = true
	else
		for k,v in pairs(Values) do
			if not v:match("^[+%d]%d*$") then
				m.message = translatef("err: SMS recipient's phone number \"%s\" is incorrect!", v)
				failure = true
			end
		end
	end
	if not failure then
		return Values
	end
	return nil
end

o = s:option(ListValue, "group", translate("Group"), translate("A recipient's phone number users group"))
o:depends("recipient_format", "group")
m.uci:foreach("sms_utils", "group", function(s)
	o:value(s.name, s.name)
end)

recEmail = s:option(DynamicList, "recipEmail", translate("Recipient's email address"), translate("For whom you want to send an email to. Allowed characters (a-zA-Z0-9._%+-)"))
recEmail:depends("action", "sendEmail")

testMail = s:option(Button, "_testMail")
testMail.template = "events_reporting/events_button"
testMail.title = translate("Send test email")
testMail.inputtitle = translate("Send")
testMail.inputstyle = "apply"
testMail.onclick = true
testMail:depends("action", "sendEmail")

function sslCreate()
    local sock = socket.tcp()
    return setmetatable({
        connect = function(_, host, port)
            local r, e = sock:connect(host, port)
            if not r then return r, e end
            sock = ssl.wrap(sock, {mode='client', protocol='tlsv1_2'})
            return sock:dohandshake()
        end
    }, {
        __index = function(t,n)
            return function(_, ...)
                return sock[n](sock, ...)
            end
        end
    })
end

function m.on_parse(self)
	if m:formvalue("cbid.events_reporting."..arg[1].."._testMail") then
        local t = {}
		local ev_type = m:formvalue("cbid.events_reporting."..arg[1]..".event")
		local recMail = {}
		local status = ""
		local message = m:formvalue("cbid.events_reporting."..arg[1]..".message")
		local secure = m:formvalue("cbid.events_reporting."..arg[1]..".secureConnection") or "0"
		local smtpIP = m:formvalue("cbid.events_reporting."..arg[1]..".smtpIP")
		local smtpPort = m:formvalue("cbid.events_reporting."..arg[1]..".smtpPort")
		local username = m:formvalue("cbid.events_reporting."..arg[1]..".userName")
		local passwd = m:formvalue("cbid.events_reporting."..arg[1]..".password")
		local senderEmail = m:formvalue("cbid.events_reporting."..arg[1]..".senderEmail")
		local subject = m:formvalue("cbid.events_reporting."..arg[1]..".subject")
		recMail = m:formvalue("cbid.events_reporting."..arg[1]..".recipEmail")

        if type(recMail) == "table" then
             for i = 1, #recMail, 1 do

                table.insert(t, tostring(" "..recMail[i].." "))
                luci.sys.exec("echo \"|".. recMail[i] .. "|\" > /tmp/ff")
            end
            allRecMail = table.concat(t)
        else
            luci.sys.exec("echo \"|".. recMail .. "|\" > /tmp/ff")
            allRecMail = m:formvalue("cbid.events_reporting."..arg[1]..".recipEmail")
        end

		if string.find(message, "\%et") then
			message = string.gsub(message, "%%et", ev_type)
		end
		if string.find(message, "\%ex") then
			message = string.gsub(message, "%%ex", "Test email")
		end
		message = luci.sys.exec("/usr/sbin/parse_msg \"".. message .. "\"")

		--isvalomas tempinis failiukas
		luci.sys.call("echo -n >/tmp/sendmail")

		if secure == "1" then
			if username and username ~= "" then
				username = " -au".. username
			end

			if passwd and passwd ~= "" then
				passwd = " -ap".. passwd
			end
			luci.sys.call("(echo -e \"subject:".. subject .."\nfrom:".. senderEmail .."\n\n".. message .."\" | sendmail -v -H \"exec openssl s_client -quiet -connect ".. smtpIP ..":".. smtpPort .." -tls1 -starttls smtp\" -f ".. senderEmail .." ".. username .." ".. passwd .." ".. allRecMail .."; echo $? >/tmp/sendmail ) &")
			for time = 0, 30, 1 do
				status = luci.util.trim(luci.sys.exec("cat /tmp/sendmail"))
				if status ~= "" then
					break
				end
				luci.sys.exec("sleep 1")
				if time == 30 then
					luci.sys.exec("killall sendmail")
				end
			end
		elseif secure == "0" then
			if username and username ~= "" then
				username = " -au".. username
			end

			if passwd and passwd ~= "" then
				passwd = " -ap".. passwd
			end

			luci.sys.exec("(echo -e \"subject:".. subject .."\nfrom:".. senderEmail .."\n\n".. message .."\" | sendmail -S \"".. smtpIP ..":".. smtpPort .."\" -f ".. senderEmail .." ".. username .." ".. passwd .." ".. allRecMail .."; echo $? >/tmp/sendmail )&")
			for time = 0, 30, 1 do
				status = luci.util.trim(luci.sys.exec("cat /tmp/sendmail"))
				if status ~= "" then
					break
				end
				luci.sys.exec("sleep 1")
				if time == 30 then
					luci.sys.exec("killall sendmail")
				end
			end
		end
		--trinamas tempinisfailiukas
		luci.sys.call("rm /tmp/sendmail")

		if status == "0" then
			m.message = translate("scs: Mail sent successful")
		else
			m.message = translate("err: Mail sent failed")
		end

	end
	return 0
end

function recEmail:validate(Values)
	local message = m:formvalue("cbid.events_reporting."..arg[1]..".message")
	local smtpIP = m:formvalue("cbid.events_reporting."..arg[1]..".smtpIP")
	local smtpPort = m:formvalue("cbid.events_reporting."..arg[1]..".smtpPort")
	local username = m:formvalue("cbid.events_reporting."..arg[1]..".userName")
	local passwd = m:formvalue("cbid.events_reporting."..arg[1]..".password")
	local senderEmail = m:formvalue("cbid.events_reporting."..arg[1]..".senderEmail")
	local sendStatus = m:formvalue("cbid.events_reporting."..arg[1]..".send_status") or "0"
	local statmsg = m:formvalue("cbid.events_reporting."..arg[1]..".status_msg")
	local failure

	if message == "" then
		m.message = translate("err: Message field is empty!")
		luci.sys.call("uci set events_reporting."..arg[1]..".message=' '")
		luci.sys.call("uci commit "..self.config)
		failure = true
	else
		if sendStatus == "1" then
			if statmsg == "" or statmsg == " \n" then
				m.message = translate("err: Status message field is empty!")
				luci.sys.call("uci set events_reporting."..arg[1]..".status_msg=' '")
				luci.sys.call("uci commit "..self.config)
				failure = true
			else
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
			end
		else
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
		end
	end
	if not failure then
		return Values
	end
	return nil
end

return m
