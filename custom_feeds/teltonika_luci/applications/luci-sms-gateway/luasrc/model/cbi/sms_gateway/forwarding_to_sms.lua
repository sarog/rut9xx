
local utl = require "luci.util"
local nw = require "luci.model.network"
local sys = require "luci.sys"
local ntm = require "luci.model.network".init()
local m
local savePressed = luci.http.formvalue("cbi.apply") and true or false



m2 = Map("sms_gateway", translate("SMS Forwarding To SMS Configuration"), 
	translate(""))
m2.addremove = false

sc = m2:section(NamedSection, "forwarding_to_sms","forwarding_to_sms", translate("SMS Forwarding To SMS Settings"))

enb_block = sc:option(Flag, "enabled", translate("Enable"), translate("Enable/disable sms forwarding"))
enb_block.rmempty = false

en_forward = sc:option(Flag, "every_sms", translate("Forward SMS-Utilities rules"), translate("Enable/disable sms-utilities rules forwarding"))
en_forward.rmempty = false

enb_block = sc:option(Flag, "sender_num", translate("Add sender number"), translate("Enable/disable adding original message sender phone number at the end of message text. Only added if total message length is up to 480 characters"))
enb_block.rmempty = false

o = sc:option(ListValue, "mode", translate("Mode"), translate("Choose witch messages are going to be forwarded"))
o:value("everyone", translate("All messages"))
o:value("list", translate("From listed numbers"))
o.default = "everyone"

o = sc:option(DynamicList, "senders_number", translate("Sender's phone number(s)"), translate("Number(s) from witch received messages will be forwarded"))
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

o = sc:option(DynamicList, "number", translate("recipients phone numbers"), translate("Number(s) to witch received messages will be forwarded to"))
function o:validate(Values)
	local failure
	for k,v in pairs(Values) do
		if not v:match("^[+%d]%d*$") then
			m2.message = translatef("err: SMS recipient's phone number \"%s\" is incorrect!", v)
			failure = true
		end
	end
	if not failure then
		return Values
	end
	return nil
end

return m2
 

