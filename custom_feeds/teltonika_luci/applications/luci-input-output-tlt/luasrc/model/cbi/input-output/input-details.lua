local sys = require "luci.sys"
local dsp = require "luci.dispatcher"
local ft = require "luci.tools.input-output"
local utl = require "luci.util"

local m, s, o

arg[1] = arg[1] or ""

m = Map("ioman",
	translate("Input Configuration"))

is_4pin = m.uci:get("hwinfo", "hwinfo", "4pin_io") or "0"
is_io = m.uci:get("hwinfo", "hwinfo", "in_out") or "0"

m.redirect = dsp.build_url("admin/services/input-output/inputs")
if m.uci:get("ioman", arg[1]) ~= "rule" then
	luci.http.redirect(dsp.build_url("admin/services/input-output/inputs"))
	return
else
	--local name = m:get(arg[1], "name") or m:get(arg[1], "_name")
	--if not name or #name == 0 then
	--	name = translate("(Unnamed Entry)")
	--end
	--m.title = "%s - %s" %{ translate("Firewall - Port Forwards"), name }
end

s = m:section(NamedSection, arg[1], "rule", "")
s.anonymous = true
s.addremove = false

--ft.opt_enabled(s, Button)
o = s:option(Flag, "enabled", translate("Enable"), translate("To enable input configuration"))
o.rmempty = false

o = s:option(ListValue, "type", translate("Input type"), translate("Select type on your own intended configuration"))
if is_io == "1" then
	o:value("digital1", translate("Digital"))
	o:value("digital2", translate("Digital isolated"))
end
if is_4pin == "1" then
	o:value("digital3", translate("Digital 4PIN"))
end
if is_io == "1" then
	o:value("analog", translate("Analog"))
end

function o.write(self, section, value)
	if value == "analog" then
		luci.sys.call('uci set ioman.'..section..'.rule="false"')
	end
		m.uci:set("ioman", section, "type", value)
end

o = s:option(ListValue, "analogtype", translate("Analog type"), translate("Select type on your own intended configuration"))
o:value("voltagetype", translate("Voltage (up to 24V)"))
o:value("currenttype", translate("Current (up to 20mA)"))
o:depends("type", "analog")

--local txtM2 = luci.http.formvalue("cbid.ioman.cfg0492bd.type") or "notget2"
--local txtM = luci.http.formvalue("cbid.ioman."..arg[1]..".type") or "notget"
--os.execute("echo \"l"..txtM.."l\" >>/tmp/log.log")
--os.execute("echo \"l"..txtM2.."l\" >>/tmp/log.log")
minval = s:option(Value, "min", translate("Min [V]"), translate("Specify minimum voltage range"))
minval:depends("analogtype", "voltagetype")
function minval:validate(Values)
	Values = string.gsub(Values,",",".")
	if tonumber(Values) and tonumber(Values)>= 0 and tonumber(Values)<= 24 then
		return Values
	else
		return nil
	end
end
maxval = s:option(Value, "max", translate("Max [V]"), translate("Specify maximum voltage range"))
maxval:depends("analogtype", "voltagetype")
function maxval:validate(Values)
	Values = string.gsub(Values,",",".")
	if tonumber(Values) and tonumber(Values)>= 0 and tonumber(Values)<= 24 then
		return Values
	else
		return nil
	end
end

minvalc = s:option(Value, "minc", translate("Min [mA]"), translate("Specify minimum current range"))
minvalc:depends("analogtype", "currenttype")
function minvalc:validate(Values)
	Values = string.gsub(Values,",",".")
	if tonumber(Values) and tonumber(Values)>= 0 and tonumber(Values)<= 20 then
		return Values
	else
		return nil
	end
end
maxvalc = s:option(Value, "maxc", translate("Max [mA]"), translate("Specify maximum current range"))
maxvalc:depends("analogtype", "currenttype")
function maxvalc:validate(Values)
	Values = string.gsub(Values,",",".")
	if tonumber(Values) and tonumber(Values)>= 1 and tonumber(Values)<= 20 then
		return Values
	else
		return nil
	end
end

o = s:option(ListValue, "triger", translate("Trigger"), translate("Select Trigger on your own intended configuration"))
o:value("no", translate("Low level"))
o:value("nc", translate("High level"))
o:value("both", translate("Both"))
o:depends("type", "digital3")
o:depends("type", "digital2")

o = s:option(ListValue, "triger2", translate("Trigger"), translate("Inside range - Input voltage falls in the specified region, Outside range - Input voltage drops out of the specified region"))
o:value("no", translate("Input open"))
o:value("nc", translate("Input shorted"))
o:value("both", translate("Both"))
o:depends("type", "digital1")

function o.cfgvalue(self, section)
	local v = m.uci:get("ioman", section, "triger")
	return v
end

function o.write(self, section, value)
	local typ = luci.http.formvalue("cbid.ioman."..arg[1]..".type")
	if typ == "digital1" then
		m.uci:set("ioman", section, "triger", value)
	end
end

o = s:option(ListValue, "triger3", translate("Trigger"), translate("Inside range - Input voltage falls in the specified region, Outside range - Input voltage drops out of the specified region"))
o:value("in", translate("Inside range"))
o:value("out", translate("Outside range"))
o:depends("type", "analog")

function o.cfgvalue(self, section)
	local v = m.uci:get("ioman", section, "triger")
	return v
end

function o.write(self, section, value)
	local typ = luci.http.formvalue("cbid.ioman."..arg[1]..".type")
	if typ == "analog" then
		m.uci:set("ioman", section, "triger", value)
	end
end

o = s:option(ListValue, "action", translate("Action"), translate("Select action on your own intended configuration"))
if luci.tools.status.show_mobile() then
	o:value("sendSMS", translate("Send SMS"))
	o:value("changeSimCard", translate("Change SIM Card"))
end
o:value("sendEmail", translate("Send email"))
o:value("changeProfile", translate("Change profile"))
o:value("wifion", translate("Turn on WiFi"))
o:value("wifioff", translate("Turn off WiFi"))
o:value("reboot", translate("Reboot"))
o:value("output", translate("Activate output"))
o:value("postGet", translate("HTTP POST/GET"))
function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
	return v
end

https = s:option(Flag, "use_https", translate("Use HTTPS"), translate("Enable SSL data encryption"))
https:depends("action", "postGet")

verify = s:option(ListValue, "verify_cert", translate("Certificate verification"), translate("Select whether to ignore or verify server certificate"))
verify:value("ignore", translate("Ignore"))
verify:value("verify", translate("Verify"))
verify:depends("use_https", "1")
verify.default = "ignore"

h_dummy = s:option(DummyValue, "getinfo_use_https_status", translate(""))
h_dummy.default = translate("Configure Root CA certificates in System->Administration->Root CA")
h_dummy:depends("verify_cert", "verify")

httptype = s:option(ListValue, "httptype", translate("Type"), translate("Specify the HTTP request type"))
httptype:depends("action", "postGet")
httptype:value("1", translate("POST"))
httptype:value("2", translate("GET"))

header = s:option(DynamicList, "header", translate("Header"), translate("Add headers to request. Example: Content-Type: application/x-www-form-urlencoded."))
header:depends("httptype", "1")

httplink = s:option(Value, "httplink", translate("Site link"), translate("Specify link of the site."))
httplink:depends("action", "postGet")

httpdata = s:option(DynamicList, "httpdata", translate("POST data"), translate("Specify data query for a request. Example: key1=value1"))
httpdata:depends("httptype", "1")
httpdata.template = "input-output/ioman_dynamiclist"

httpdata1 = s:option(DynamicList, "httpdata1", translate("HTML query"), translate("Extra html query name/value pair. Example: key1=value1"))
httpdata1:depends("httptype", "2")
httpdata1.template = "input-output/ioman_dynamiclist"

-- smstxt = s:option(Value, "smstxt", translate("SMS text"), translate("Specify message to send in SMS, field validation (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
-- smstxt:depends("action", "sendSMS")
-- smstxt.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',0)"

smstxt = s:option(Value, "smstxt", translate("SMS text"), translate("Specify message to send in SMS, field validation (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
smstxt:depends("action", "sendSMS")
smstxt.template = "input-output/ioman_textbox"
smstxt.rows = "4"
smstxt.default = ""
smstxt.indicator = arg[1]

o = s:option(ListValue, "recipient_format", translate("Recipients"), translate("You can choose add single numbers in a list or use User group list"))
o:depends("action", "sendSMS")
o:value("single", translate("Single number"))
o:value("group", translate("User group"))

telnum = s:option(DynamicList, "telnum", translate("Recipient's phone number"), translate("Specify Recipient's phone number, e.g. +37012345678"))
telnum:depends("recipient_format", "single")

function telnum:validate(Values)
	local smstxt = m:formvalue("cbid.ioman."..arg[1]..".smstxt")
	local failure
	if smstxt == "" then
		m.message = translate("err: SMS text is incorrect!")
		failure = true
	else
		for k,v in pairs(Values) do
			if not v:match("^[+%d]%d*$") then
				m.message = translatef("err: SMS sender's phone number \"%s\" is incorrect!", v)
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

function smstxt.write(self, section, value)
	local value = luci.http.formvalue("cbid.ioman."..arg[1]..".smstxt")
	value = string.gsub(value, "%s", " ")
	if value then
		m.uci:set("ioman", section, "smstxt", value)
	end
end
emailsub = s:option(Value, "subject", translate("Subject"), translate("Specify subject of email, field validation (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
emailsub:depends("action", "sendEmail")
emailsub.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',0)"

emailtxt = s:option(Value, "message", translate("Message"), translate("Specify message to send in email, field validation (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
emailtxt:depends("action", "sendEmail")
emailtxt.template = "input-output/ioman_textbox"
emailtxt.rows = "4"
emailtxt.default = ""
emailtxt.indicator = arg[1]

function emailtxt.write(self, section, value)
	local value = luci.http.formvalue("cbid.ioman."..arg[1]..".message")
	value = string.gsub(value, "%s", " ")
	if value then
		m.uci:set("ioman", section, "message", value)
	end
end

smtpip = s:option(Value, "smtpIP", translate("SMTP server"), translate("Specify SMTP (Simple Mail Trasfer Protocol) server, field validation (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
smtpip:depends("action", "sendEmail")
smtpip.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',0)"

smtpPort = s:option(Value, "smtpPort", translate("SMTP server port"), translate("Specify SNMP server port"))
smtpPort:depends("action", "sendEmail")
smtpPort.datatype = "port"

secCon = s:option(Flag, "secureConnection", translate("Secure connection"), translate("Specify if server support SSL or TLS"))
secCon:depends("action", "sendEmail")

username = s:option(Value, "userName", translate("User name"), translate("Specify user name to connect SNMP server"))
username:depends("action", "sendEmail")
username.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',0)"

passwd = s:option(Value, "password", translate("Password"), translate("Specify the password of the user"))
passwd.password = true
passwd:depends("action", "sendEmail")
passwd.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',0)"

senderEmail = s:option(Value, "senderEmail", translate("Sender’s email address"), translate("Specify your email address"))
senderEmail:depends("action", "sendEmail")
senderEmail.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',0)"

recEmail = s:option(DynamicList, "recipEmail", translate("Recipient's email address"), translate("Specify for whom you want to send email"))
recEmail:depends("action", "sendEmail")
function recEmail:validate(Values)
	local failure
	for k,v in pairs(Values) do
		if not v:match("^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$") then
			m.message = translatef("err: Recipient's email address is incorrect!")
			failure = true
		end
	end
	if not failure then
		return Values
	end
	return nil
end

reboott = s:option(Value, "reboottime", translate("Reboot after (s)"), translate("Device will reload after a specified time, format seconds"))
reboott:depends("action", "reboot")
reboott.datatype = "uinteger"
reboott.default = "0"
reboott.rmempty = "true"

o = s:option(ListValue, "continuous", translate("Output activated"), translate("Output activated for specified time, or while condition exist"))
o:depends({action = "output",triger = "no"})
o:depends({action = "output",triger = "nc"})
o:depends({action = "output",triger2 = "no"})
o:depends({action = "output",triger2 = "nc"})
o:depends({action = "output",triger3 = "in"})
o:depends({action = "output",triger3 = "out"})
o:value("0", translate("Seconds"))
o:value("1", translate("While exist"))
o:value("2", translate("Delayed action"))

o = s:option(ListValue, "continuous2", translate("Output activated"), translate("Output activated for specified time, or while condition exist"))
o:depends({action = "output",triger = "both"})
o:depends({action = "output",triger2 = "both"})
o:value("0", translate("Seconds"))
o:value("2", translate("Delayed action"))

function o.cfgvalue(self, section)
	return m.uci:get("ioman", section, "continuous")
end

function o.write(self, section, value)
	m.uci:set("ioman", section, "continuous", value)
end

outputt = s:option(Value, "outputtime", translate("Seconds"), translate("Device will be activated for specified time, format seconds"))
outputt:depends("continuous", "0")
outputt:depends("continuous2", "0")
outputt.datatype = "uinteger"

outputt = s:option(Value, "startdelay", translate("Trigger delay, s"), translate("Input must stay activated for this many seconds to activate the output"))
outputt:depends("continuous", "2")
outputt:depends("continuous2", "2")
outputt.datatype = "uinteger"

outputt = s:option(Value, "stopdelay", translate("Action stop delay, s"), translate("Output will stay activated this many seconds after the trigger is cleared"))
outputt:depends("continuous", "2")
outputt:depends("continuous2", "2")
outputt.datatype = "uinteger"

outputtype = s:option(ListValue, "outputnb", translate("Output type"), translate("Select output type, which will be activated, depending on output time"))
if is_io == "1" then
	outputtype:value("1", translate("Open collector"))
	outputtype:value("2", translate("Relay output"))
end
if is_4pin == "1" then
	outputtype:value("3", translate("Digital 4PIN"))
end
outputtype:depends("action", "output")

-- changeSim = s:option(ListValue, "simcard", translate("Sim"), translate("Select which sim card will be changed"))
-- changeSim:value("primary", translate("Primary"))
-- changeSim:value("secondary", translate("Secondary"))
-- changeSim:depends({action = "changeSimCard",triger = "Input Configuration"})
-- changeSim:depends({action = "changeSimCard",triger = "nc"})

local uci = require "luci.model.uci".cursor()
local path = uci:get("profiles", "general", "path")

function getProfiles()
	local profiles = {}

	x = uci.cursor()
	x:foreach("profiles", "profile", function(s)
		if s['.name'] then
			if not s['updated'] then
				profiles[#profiles+1] = {s['.name'], ""}
			else
				profiles[#profiles+1] = {s['.name'], s['updated']} 
			end
		end
	end)
	
	return profiles
end

o = s:option(ListValue, "profiles", translate("Profile"), translate("Select which one profile will be set and used"))
o:depends("action", "changeProfile")
for index, profile in ipairs(getProfiles()) do
	--o:value(profile[1], profile[1].." "..profile[2])
	
	if profile[1] == "default" then
		o:value("default", "default")
	elseif profile[1] and profile[2] then
		o:value(profile[1] .. "_" .. profile[2], profile[1])
	end
end

local ioman_enable = utl.trim(sys.exec("uci -q get ioman. " .. arg[1] .. ".enabled")) or "0"
function m.on_commit()
	--Delete all usr_enable from ioman config
	local iomanEnable = m:formvalue("cbid.ioman." .. arg[1] .. ".enabled") or "0"
	if iomanEnable ~= ioman_enable then
		m.uci:foreach("ioman", "rule", function(s)
			local usr_enable = s.usr_enable or ""
			ioman_inst2 = s[".name"] or ""
			if usr_enable == "1" then
				m.uci:delete("ioman", ioman_inst2, "usr_enable")
			end
		end)
	end
	m.uci.commit("ioman")
end




return m
