datatpy = require "luci.cbi.datatypes"


m = Map("wget_reboot", translate("Wget Reboot"), translate(""))
s = m:section(NamedSection, "wget_reboot", translate("Wget Reboot"), translate("Wget Reboot Setup"))
s.addremove = false

-- enable wget reboot option
e = s:option(Flag, "enable", translate("Enable"), translate("Enable wget reboot feature"))

-- enable router reboot
v = s:option(ListValue, "action", translate("Action if response is received"), translate("Action after the defined number of unsuccessfull retries packet received"))
	v.template = "auto-reboot/lvalue"
	v:value("1", "Reboot")
	v:value("2", "Modem restart")
	v:value("3", "Restart mobile connection")
	v:value("4", "(Re)register")
	v:value("6", "Send SMS")
	v:value("5", "None")

-- sms number fields
o = 	s:option(DynamicList, "number", translate("Phone Number"), translate("Phone number for the SMS to be sent to"))
o:depends("action", "6")
function o:validate(Values)
	local failure
	for kn,number in pairs(Values) do
		if number == "" then
			m.message = translate("err: Phone number field is empty!")
			failure = true
		elseif not number:match("^[+%d]%d*$") then
			m.message = translatef("err: Phone number \"%s\" is incorrect!", number)
			failure = true
		end
	end
	if not failure then
		return Values
	end 
end


-- message field
local textx = ""
o = s:option(Value, "message")
o:depends("action", "6")
o.template  = "auto-reboot/msg-field"
o.formvalue = function(self, section)
	textx = m:formvalue("cbid.sms_utils.1.message")
	if textx and #textx > 0 then		
	return textx
	end
end

-- ping inverval column and number validation
t = s:option(ListValue, "time", translate("Interval between requests"), translate("Time interval in minutes between two requests"))
	t.template = "auto-reboot/time"
	--t:depends("enable", "1")
	t:value("1", translate("1 mins"))
	t:value("2", translate("2 mins"))
	t:value("3", translate("3 mins"))
	t:value("4", translate("4 mins"))
	t:value("5", translate("5 mins"))
	t:value("15", translate("15 mins"))
	t:value("30", translate("30 mins"))
	t:value("60", translate("1 hour"))
	t:value("120", translate("2 hours"))

-- Check the message text
function t:validate(Values)
	local failure
	if textx == "" then
		m.message = translate("err: Message text field is empty!")
		failure = true
	end
	if not failure then
		return Values
	end	
	return nil
end 

l = s:option(Value, "timeout", translate("Wget timeout (sec)"), translate("Time interval (in seconds) wget wait response. Range [1 - 9999]"))
l.default = "10"
l.datatype = "and(range(1,9999), lengthvalidation(1,4,'^[0-9]+$'))"
l.rmempty = false

-- number of retries and number validation
k = s:option(Value, "retry", translate("Retry count"), translate("Number of retries after unsuccessful to receive reply packets. Range [1 - 9999]"))
k.default = "2"
k.datatype = "and(range(1,9999), lengthvalidation(1,4,'^[0-9]+$'))"
k.rmempty = false

-- host ping from wired
l = s:option(Value, "host", translate("Host to ping"), translate("IP address or domain name which will be used to send packets to. E.g. 192.168.1.1 (or www.host.com if DNS server is configured correctly)"))
l.default = "www.google.com"
l.datatype = "host"
l.rmempty = false

return m
