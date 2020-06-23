
local utl = require "luci.util"
local nw = require "luci.model.network"
local sys = require "luci.sys"
local ntm = require "luci.model.network".init()
local m
local savePressed = luci.http.formvalue("cbi.apply") and true or false
local textx = ""


m2 = Map("sms_utils", translate("Auto Reply Configuration"), 
	translate(""))
m2.addremove = false

sc = m2:section(NamedSection, "auto_reply","status", translate("Reply Configuration"))

enb_block = sc:option(Flag, "enabled", translate("Enable"), translate(""))
enb_block.rmempty = false

o = sc:option(Flag, "every_sms", translate("Reply SMS-Utilities rules"), translate("It will reply to sms rules, from SMS-Utilities"))
o.rmempty = false

del = sc:option(Flag, "delete_msg", translate("Don't save received message"), translate(""))
del.rmempty = false

-- message field
msg = sc:option(Value, "msg")
msg.template = "sms_gateway/auto_reply_textarea"
msg.formvalue = function(self, section)
	textx = m2:formvalue("cbid.sms_utils.1.message")						  
	if textx and #textx > 0 then		
	return textx
	end
end

mode = sc:option(ListValue, "mode", translate("Mode"), translate(""))
mode:value("everyone", translate("Everyone"))
mode:value("list", translate("Listed numbers"))
mode.default = "everyone"

function mode:validate(Values)
	local failure
	if textx == "" then
		m2.message = translate("err: Message field is empty!")
		failure = true
	end
	if not failure then
		return Values
	end	
	return nil
end 

number = sc:option(DynamicList, "number", translate("Recipient's phone number"), translate(""))
number:depends("mode", "list")
function number:validate(Values)
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



function m2.handle(self, state, data)
		msg = m2:formvalue("cbid.sms_utils.auto_reply.msg")
		m2.uci:set("sms_utils", "auto_reply", "msg", msg)
		m2.uci:commit("sms_utils")
end


m2.message = translate("err: Be careful not to send sms messages to yourself!")
 
return m2--, m
 

