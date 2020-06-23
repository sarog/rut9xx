local sys = require "luci.sys"
local dsp = require "luci.dispatcher"
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()

local b='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
local function b64enc(data)
    return ((data:gsub('.', function(x)
        local r,b='',x:byte()
        for i=8,1,-1 do r=r..(b%2^i-b%2^(i-1)>0 and '1' or '0') end
        return r
    end)..'0000'):gsub('%d%d%d?%d?%d?%d?', function(x)
        if (#x < 6) then return '' end
        local c=0
        for i=1,6 do c=c+(x:sub(i,i)=='1' and 2^(6-i) or 0) end
        return b:sub(c+1,c+1)
    end)..({ '', '==', '=' })[#data%3+1])
end

function b64dec(data)
    data = string.gsub(data, '[^'..b..'=]', '')
    return (data:gsub('.', function(x)
        if (x == '=') then return '' end
        local r,f='',(b:find(x)-1)
        for i=6,1,-1 do r=r..(f%2^i-f%2^(i-1)>0 and '1' or '0') end
        return r;
    end):gsub('%d%d%d?%d?%d?%d?%d?%d?', function(x)
        if (#x ~= 8) then return '' end
        local c=0
        for i=1,8 do c=c+(x:sub(i,i)=='1' and 2^(8-i) or 0) end
        return string.char(c)
    end))
end

local m, s, o

arg[1] = arg[1] or ""

m = Map("sms_gateway",
	translate("Scheduled Messages Configuration"))
	
m.redirect = dsp.build_url("admin/services/sms_gateway/scheduled_messages")

--[[
m.redirect = dsp.build_url("admin/status/event/log_report")
if m.uci:get("eventslog_report", arg[1]) ~= "rule" then
	luci.http.redirect(dsp.build_url("admin/status/event/log_report"))
	return
else
	--local name = m:get(arg[1], "name") or m:get(arg[1], "_name")
	--if not name or #name == 0 then
	--	name = translate("(Unnamed Entry)")
	--end
	--m.title = "%s - %s" %{ translate("Firewall - Port Forwards"), name }
end
]]--
s = m:section(NamedSection, arg[1], "msg", translate("Modify scheduled message"))
s.anonymous = true
s.addremove = false

o = s:option(Flag, "enable", translate("Enable"), translate("Enable/disable message sending"))

o = s:option(Value, "phonenumber", translate("Recipient's phone number"), translate("Phone number to whitch your messages are going to be sent. Allowed characters: (0-9#*+)"))
o.datatype = "fieldvalidation('^[0-9#*+]+$',0)"
o.rmempty = false

local textx = ""
o = s:option(Value, "message")
o.template  = "sms_gateway/sms_field"
o.formvalue = function(self, section)
	textx = m:formvalue("cbid.sms_utils.1.message")
	if textx and #textx > 0 then
		uci:set(self.config, section, "is_b64", "1")
		uci:commit(self.config)
		return b64enc(textx)
	end
end

function o.cfgvalue(self, section)
	local msg = self.map:get(section, "message") or ""
	local is_base64 = uci:get(self.config, section, "is_b64")
	if msg and is_base64 and is_base64 == "1" then
		return b64dec(msg)
	elseif msg then
		return msg
	else
		return ""
	end
end


--o = s:option(Value, "message", translate("Message text"), translate("Message text. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
--o.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',0)"

o = s:option(ListValue, "repeats", translate("Message sending Interval"), translate("Send messages every selected time interval"))
o:value("day", translate("Day"))
o:value("week", translate("Week"))
o:value("month", translate("Month"))
o:value("year", translate("Year"))
o.default = m.uci:get("sms_gateway", arg[1], "repeats")
function o.cfgvalue(...)
        local v = Value.cfgvalue(...)
                return v
end

function o:validate(Values)
	local phonenumber = m:formvalue("cbid.eventslog_report."..arg[1]..".phonenumber")
	--local message = m:formvalue("cbid.eventslog_report."..arg[1]..".message") 
	local failure
	
	if phonenumber == "" then
		m.message = translate("err: Recipient's phone number field is empty!")
		failure = true
	else
		if textx == "" then
			m.message = translate("err: Message text field is empty")
			failure = true
		end
	end
	
	if not failure then
		return Values
	end
	return nil
end

o = s:option(ListValue, "weekday", translate("Weekday"), translate("Day of the week to send messages"))                        
o:value("sun", translate("Sunday"))
o:value("mon", translate("Monday"))
o:value("tue", translate("Tuesday"))
o:value("wed", translate("Wednesday"))
o:value("thu", translate("Thursday"))
o:value("fri", translate("Friday"))
o:value("sat", translate("Saturday"))
o:depends("repeats", "week")                                                                                                             
function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
		return v
end		

o = s:option(ListValue, "month", translate("Month"), translate("Month of the year to send messages"))                              
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
o:depends("repeats", "year") 
function o.cfgvalue(...) 
	local v = Value.cfgvalue(...)
		return v
end

o = s:option(ListValue, "monthday", translate("Month day"), translate("Day of the month to send messages")) 
o:value("1", translate("1"))
o:value("2", translate("2"))
o:value("3", translate("3"))
o:value("4", translate("4"))
o:value("5", translate("5"))
o:value("6", translate("6"))                                                                                                             
o:value("7", translate("7"))
o:value("8", translate("8"))                                                                                                             
o:value("9", translate("9"))
o:value("10", translate("10"))                                                                                                             
o:value("11", translate("11"))
o:value("12", translate("12"))                                                                                                             
o:value("13", translate("13"))
o:value("14", translate("14"))                                                                                                             
o:value("15", translate("15"))
o:value("16", translate("16"))                                                                                                             
o:value("17", translate("17"))
o:value("18", translate("18"))                                                                                                             
o:value("19", translate("19"))
o:value("20", translate("20"))                                                                                                           
o:value("21", translate("21"))                                                                                                           
o:value("22", translate("22"))                                                                                                      
o:value("23", translate("23"))                                                                                                           
o:value("24", translate("24"))                                                                                                           
o:value("25", translate("25"))                                                                                                      
o:value("26", translate("26"))                                                                                                           
o:value("27", translate("27"))                                                                                                           
o:value("28", translate("28"))                                                                                              
o:value("29", translate("29")) 
o:value("30", translate("30"))
o:value("31", translate("31"))
o:value("28-31", translate("Last day of the Month"))
o:depends("repeats", "month") 
o:depends("repeats", "year")
function o.cfgvalue(...)
	local v = Value.cfgvalue(...)
		return v
end

o = s:option(ListValue, "hour", translate("Hour"), translate("Hour of the day to send messages"))                          
o:value("1", translate("1"))                                                                                                          
o:value("2", translate("2"))                                                                                                          
o:value("3", translate("3"))                                                                                                          
o:value("4", translate("4"))                                                                                                          
o:value("5", translate("5"))                                                                                                          
o:value("6", translate("6"))                                                                                                          
o:value("7", translate("7"))                                                                                                          
o:value("8", translate("8"))                                                                                                          
o:value("9", translate("9"))                                                                                                          
o:value("10", translate("10"))                                                                                                        
o:value("11", translate("11"))                                                                                                        
o:value("12", translate("12"))                                                                                                        
o:value("13", translate("13"))                                                                                                        
o:value("14", translate("14"))                                                                                                        
o:value("15", translate("15"))                                                                                                        
o:value("16", translate("16"))                                                                                                        
o:value("17", translate("17"))                                                                                                        
o:value("18", translate("18"))                                                                                                        
o:value("19", translate("19"))                                                                                                        
o:value("20", translate("20"))                                                                                                        
o:value("21", translate("21"))                                                                                                        
o:value("22", translate("22"))                                                                                                        
o:value("23", translate("23"))                                                                                                        
o:value("0", translate("00"))                                                                                                        
function o.cfgvalue(...)                                                                                                              
        local v = Value.cfgvalue(...)                                                                                                 
		return v                                                                                                              
end

o = s:option(ListValue, "minute", translate("Minute"), translate("Minute of the hour to send messages"))                          
o:value("1", translate("1"))                                                                                                          
o:value("2", translate("2"))                                                                                                          
o:value("3", translate("3"))                                                                                                          
o:value("4", translate("4"))                                                                                                          
o:value("5", translate("5"))                                                                                                          
o:value("6", translate("6"))                                                                                                          
o:value("7", translate("7"))                                                                                                          
o:value("8", translate("8"))                                                                                                          
o:value("9", translate("9"))                                                                                                          
o:value("10", translate("10"))                                                                                                        
o:value("11", translate("11"))                                                                                                        
o:value("12", translate("12"))                                                                                                        
o:value("13", translate("13"))                                                                                                        
o:value("14", translate("14"))                                                                                                        
o:value("15", translate("15"))                                                                                                        
o:value("16", translate("16"))                                                                                                        
o:value("17", translate("17"))                                                                                                        
o:value("18", translate("18"))                                                                                                        
o:value("19", translate("19"))                                                                                                        
o:value("20", translate("20"))                                                                                                        
o:value("21", translate("21"))                                                                                                        
o:value("22", translate("22"))                                                                                                        
o:value("23", translate("23"))                                                                                                        
o:value("24", translate("24"))
o:value("25", translate("25"))                                                                                                        
o:value("26", translate("26"))                                                                                                        
o:value("27", translate("27"))                                                                                                        
o:value("28", translate("28"))                                                                                                        
o:value("29", translate("29"))                                                                                                        
o:value("30", translate("30"))                                                                                                        
o:value("31", translate("31"))                                                                                                        
o:value("32", translate("32"))                                                                                                        
o:value("33", translate("33"))                                                                                                        
o:value("34", translate("34"))                                                                                                        
o:value("35", translate("35"))                                                                                                        
o:value("36", translate("36"))                                                                                                        
o:value("37", translate("37"))                                                                                                        
o:value("38", translate("38"))                                                                                                        
o:value("39", translate("39"))
o:value("40", translate("40"))                                                                                                        
o:value("41", translate("41"))                                                                                                        
o:value("42", translate("42"))                                                                                                        
o:value("43", translate("43"))                                                                                                        
o:value("44", translate("44"))                                                                                                        
o:value("45", translate("45"))                                                                                                        
o:value("46", translate("46"))                                                                                                        
o:value("47", translate("47"))                                                                                                        
o:value("48", translate("48"))                                                                                                        
o:value("49", translate("49"))                                                                                                        
o:value("50", translate("50"))                                                                                                        
o:value("51", translate("51"))                                                                                                        
o:value("52", translate("52"))                                                                                                        
o:value("53", translate("53"))                                                                                                        
o:value("54", translate("54"))
o:value("55", translate("55"))                                                                                                        
o:value("56", translate("56"))                                                                                                        
o:value("57", translate("57"))                                                                                                        
o:value("58", translate("58"))                                                                                                        
o:value("59", translate("59"))                                                                                                        
o:value("0", translate("00"))
function o.cfgvalue(...)                                                                                                              
        local v = Value.cfgvalue(...)                                                                                                 
		return v
end

function m.on_after_save()
	
	require("uci")
	local x = uci.cursor()
	local scriptsh = "scheduled_sms_sender.sh"
	luci.sys.call('sed -i /' .. scriptsh .. '/d /etc/crontabs/root')
    luci.sys.call('sed -i /last_day_of_month.s/d /etc/crontabs/root')
	local crontab = io.open("/etc/crontabs/root", "a")
	x:foreach("sms_gateway", "msg", function(s)

			if s.enable == "1" then
                if s.monthday == "28-31" then
                    local datstring = string.format("%s %s %s * * %s %s %s", s.minute, s.hour, s.monthday, "/sbin/last_day_of_month.sh", s.phonenumber, "\n")
					crontab:write(datstring)
				elseif s.repeats == "day" then
					local datstring = string.format("%s %s * * * %s %s %s", s.minute, s.hour, "/sbin/scheduled_sms_sender.sh", s.phonenumber, "\n")
					crontab:write(datstring)
				elseif s.repeats == "week" then
					local datstring = string.format("%s %s * * %s %s %s %s", s.minute, s.hour, s.weekday, "/sbin/scheduled_sms_sender.sh", s.phonenumber, "\n")
					crontab:write(datstring)
				elseif s.repeats == "month" then
					local datstring = string.format("%s %s %s * * %s %s %s", s.minute, s.hour, s.monthday, "/sbin/scheduled_sms_sender.sh", s.phonenumber, "\n")
					crontab:write(datstring)
				elseif s.repeats == "year" then
					local datstring = string.format("%s %s %s %s * %s %s %s", s.minute, s.hour, s.monthday, s.month, "/sbin/scheduled_sms_sender.sh", s.phonenumber, "\n")
					crontab:write(datstring)
					
				end
			end
			
	end)
	crontab:close()
end



return m
