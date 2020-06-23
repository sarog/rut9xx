-- PING Reboot
local m, s, e, v, t, k, l, o, sim2, numb
local show = require("luci.tools.status").show_mobile()
dsp = require "luci.dispatcher"

m = Map("ping_reboot", translate("Ping Reboot"))
	m.redirect = dsp.build_url("admin", "services", "auto-reboot", "ping-reboot")

s = m:section(NamedSection, arg[1], "ping_reboot", translate("Ping Reboot Settings"))
	s.addremove = false

-- enable ping reboot option
e = s:option(Flag, "enable", translate("Enable"), translate("Enable ping reboot feature"))
e.rmempty = false

e = s:option(Flag, "stop_action", translate("No action on data limit"), translate("Stop actions when mobile data limit is reached"))
	e.rmempty = false

-- enable router reboot
v = s:option(ListValue, "action", translate("Action if no echo is received"), translate("Action after the defined number of unsuccessfull retries (no echo reply for sent ICMP (Internet Control Message Protocol) packet received)"))
	v.template = "auto-reboot/lvalue"
	v:value("1", "Reboot")
	v:value("2", "Modem restart")
	v:value("3", "Restart mobile connection")
	v:value("4", "(Re)register")
	v:value("6", "Send SMS")
	v:value("5", "None")


-- sms number fields
numb = 	s:option(DynamicList, "number", translate("Phone Number"), translate("Phone number for the SMS to be sent to"))
numb:depends("action", "6")
function numb:validate(Values)
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
t = s:option(ListValue, "time", translate("Interval between pings"), translate("Time interval in minutes between two ping packets"))
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

-- Laikas iki rebooto po nesekmingo pingo
l = s:option(Value, "time_out", translate("Ping timeout (sec)"), translate("Time interval (in seconds) to wait for ICMP (Internet Control Message Protocol) echo reply packet. Range [1 - 9999]"))
l.default = "10"
--l:depends("enable", "1")
l.datatype = "and(range(1,9999), lengthvalidation(1,4,'^[0-9]+$'))"
l.rmempty = false

-- Ping packet size
z = s:option(Value, "packet_size", translate("Packet size"), translate("Ping packet size in bytes. Range [0 - 1000]"))
z.default = "56"
--z:depends("enable", "1")
z.datatype = "and(range(0,1000), lengthvalidation(1,4,'^[0-9]+$'))"
z.rmempty = false

-- number of retries and number validation
k = s:option(Value, "retry", translate("Retry count"), translate("Number of failed to receive ICMP (Internet Control Message Protocol) echo reply packets. Range [1 - 9999]"))
k.default = "2"
--k:depends("reboot", "1")
k.datatype = "and(range(1,9999), lengthvalidation(1,4,'^[0-9]+$'))"
k.rmempty = false

t = s:option(ListValue, "interface", translate("Interface"), translate(""))
t:value("1", translate("Automatically selected"))
if show then
	t:value("2", translate("Ping from mobile"))
end

local interface = luci.http.formvalue("cbid.ping_reboot."..arg[1]..".interface")

-- host ping from wired
l = s:option(Value, "host", translate("Host to ping"), translate("IP address or domain name which will be used to send ping packets to. E.g. 192.168.1.1 (or www.host.com if DNS server is configured correctly)"))
l.default = "127.0.0.1"
l.datatype = "host"
l:depends("interface", "1")
if interface and interface == "1" then
	l.rmempty = false
end

-- host ping from sim1
l = s:option(Value, "host1", translate("Host to ping from SIM 1"), translate("IP address or domain name which will be used to send ping packets to. E.g. 192.168.1.1 (or www.host.com if DNS server is configured correctly)"))
l.default = "127.0.0.1"
l.datatype = "host"
l:depends("interface", "2")
if interface and interface == "2" then
	l.rmempty = false
end

-- host ping from sim2
sim2 = s:option(Value, "host2", translate("Host to ping from SIM 2"), translate("IP address or domain name which will be used to send ping packets to. E.g. 192.168.1.1 (or www.host.com if DNS server is configured correctly)"))
sim2.default = "127.0.0.1"
sim2.datatype = "host"
sim2:depends("interface", "2")
if interface and interface == "2" then
	sim2.rmempty = false
end

msg = s:option(DummyValue, "message", "")
	msg.rawhtml = true
	--msg.template = "cbi/message"

function msg.cfgvalue(self, section)
	time_value = m:formvalue(string.format("cbid.%s.%s.time", self.config, "time")) or m.uci:get(self.config, section, "time")
	retry_value = m:formvalue(string.format("cbid.%s.%s.time", self.config, "retry")) or m.uci:get(self.config, section, "retry")

	if time_value and retry_value then
		local time = time_value * retry_value
		return string.format('<span style="color: #808080; font-style: italic; font-size: 10px;">Reboot will be performed after %d min.</span>', time)
	end
end

return m
