
local utl = require "luci.util"
local nw = require "luci.model.network"
local sys = require "luci.sys"
local ntm = require "luci.model.network".init()
local m
local savePressed = luci.http.formvalue("cbi.apply") and true or false



m2 = Map("sms_gateway", translate("SMS Forwarding To Email Configuration"), 
	translate(""))
m2.addremove = false

sc = m2:section(NamedSection, "forwarding_to_smtp","forwarding_to_smtp", translate("SMS Forwarding To Email Settings"))

o = sc:option(Flag, "enabled", translate("Enable"), translate("Enable/disable sms forwarding to SMTP"))
o.rmempty = false

en_forward = sc:option(Flag, "every_sms", translate("Forward SMS-Utilities rules"), translate("Enable/disable sms-utilities rules forwarding"))
en_forward.rmempty = false

o = sc:option(Flag, "sender_num", translate("Add sender's number"), translate("Enable/disable adding sender phone number at the end of email text body"))
o.rmempty = false

o = sc:option(Value, "subject", translate("Subject"), translate("Subject of an email. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
o.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',0)"

o = sc:option(Value, "smtpip", translate("SMTP server"), translate("SMTP (Simple Mail Transfer Protocol) server. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
o.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',0)"

o = sc:option(Value, "smtpport", translate("SMTP server port"), translate("SMTP (Simple Mail Transfer Protocol) server port"))
o.datatype = "port"

o = sc:option(Flag, "secureconnection", translate("Secure connection"), translate("Use only if server supports SSL or TLS"))
o.rmempty= false
o.default="0"

o = sc:option(Value, "username", translate("User name"), translate("User name for authentication on SMTP (Simple Mail Transfer Protocol) server. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
o.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',0)"

o = sc:option(Value, "password", translate("Password"), translate("Password for authentication on SMTP (Simple Mail Transfer Protocol) server. Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:; []"))
o.password = true
o.datatype = "password(1)"

o = sc:option(Value, "senderemail", translate("Sender's email address"), translate("An address that will be used to send your email from. Allowed characters (a-zA-Z0-9._%+-)"))
o.datatype = "fieldvalidation('^[a-zA-Z0-9._%%+-]+@[a-zA-Z0-9.-]+[.][a-zA-Z]+$',0)"

o = sc:option(DynamicList, "recipemail", translate("Recipient's email address"), translate("For whom you want to send an email to. Allowed characters (a-zA-Z0-9._%+-)"))


o = sc:option(ListValue, "mode", translate("Mode"), translate("Choose witch messages are going to be forwarded"))
o:value("everyone", translate("All messages"))
o:value("list", translate("From listed numbers"))
o.default = "everyone"

o = sc:option(DynamicList, "number", translate("Sender's phone number(s)"), translate("Number(s) from witch received messages will be forwarded"))
o:depends("mode", "list")
function o:validate(Values)
	local failure
	for k,v in pairs(Values) do
		if not v:match("^[+%d]%d*$") then
			m2.message = translatef("err: SMS sender's phone number \"%s\" is incorrect!", v)
			failure = true
		end
	end
	if not failure then
		return Values
	end
	return nil
end

return m2
 

