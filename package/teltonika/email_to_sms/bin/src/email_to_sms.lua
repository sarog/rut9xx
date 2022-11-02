#!/usr/bin/lua

local pop3 = require "pop3"

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

local function trim(s)
  return (s:gsub("^%s*(.-)%s*$", "%1"))
end

local function read_output(command)
	local f = io.popen(command)
	local l = f:read("*a")
	f:close()
	return trim(l)
end

local function get_modem()
	local util = require("vuci.util")
	local gsmd = util.ubus("gsmd", "get_modems") or {}
	local modem_list = gsmd.modems or {}
	local modem = modem_list[1].id
	return modem
end

local function send_big_sms(number, text, limit, modem)
	-- local C1 = "@$\r\n_ !\"#%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
	-- local filtertable = {}
	-- for i = 1, #text do
	-- 	local s = string.char(text:byte(i))
	-- 	if C1:find(s, 1, true) then
	-- 		filtertable[#filtertable + 1] = s
	-- 	end
	-- end
	-- text = table.concat(filtertable)

	if #text == 0 then
		print("message is empty")
		return false
	end

	local MAX_SMS_SIZE = 1200
	local is_absolute_unit = (#text > MAX_SMS_SIZE)
	local no = 1
	while #text > MAX_SMS_SIZE do
		local txt = "SMS-CHAIN#" .. tostring(no) .. "\n" .. text:sub(1, MAX_SMS_SIZE)
		local data = b64enc(txt)
		local file = io.open("/tmp/.smstext", "w")
		text = text:sub(MAX_SMS_SIZE + 1)
		no = no + 1

		file:write(data)
		file:close()
		response=os.execute("gsmctl -O "..modem.." -S --send-b64 "..number)
	end

	if is_absolute_unit then
		print("is_absolute_unit")
		text = "SMS-CHAIN#" .. tonumber(no) .. "\n" .. text
	end
	local data = b64enc(text)
	local file = io.open("/tmp/.smstext", "w")

	file:write(data)
	file:close()
	response=os.execute("gsmctl -O "..modem.." -S --send-b64 "..number.." -L "..limit)
	-- print("====\n"..response.."\n=======")
	print("SMS: ["..#text.."]================\n".. text.."===================")

	return true
end

local enabled = read_output("uci -q get email_to_sms.pop3.enabled")
local function main()
	if tonumber(enabled) == 1 then
		ssl = tonumber(read_output("uci -q get email_to_sms.pop3.ssl")) == 1
		ssl_verify = tonumber(read_output("uci -q get email_to_sms.pop3.ssl_verify")) == 1
		local some_mail = {
			host	 = read_output("uci -q get email_to_sms.pop3.host");
			username = read_output("uci -q get email_to_sms.pop3.username");
			password = read_output("uci -q get email_to_sms.pop3.password");
			port = read_output("uci -q get email_to_sms.pop3.port");
			limit = read_output("uci -q get email_to_sms.pop3.limit");
			modem = read_output("uci -q get email_to_sms.pop3.modem_id");
			--host     = os.getenv("LUA_MAIL_HOST") or 'pop.gmail.com';
			--port = '995';
		}
		if not some_mail.modem or #some_mail.modem == 0 then
			some_mail.modem = get_modem()
		end
		some_mail.limit = tonumber(some_mail.limit) or 0
		local mbox = pop3.new()
		if ssl then
			mbox:open_tls(some_mail.host, some_mail.port, nil, ssl_verify)
		else
			mbox:open(some_mail.host, some_mail.port)
		end
		print('open   :', mbox:is_open())
		if mbox:is_open() then
			mbox:auth(some_mail.username, some_mail.password)
			if mbox:is_auth() then
				print('auth   :', mbox:is_auth())
				for k, msg in mbox:messages() do
					local anytext = nil
					local plaintext = nil
					number=msg:subject()
					for i,v in ipairs(msg:full_content()) do
						if v.text then
							if number:match('^[0-9+]+$') ~= nil then
								if v.type == "text/plain" then
									plaintext = v.text
								else
									anytext = v.text
								end
							end
						end
					end
					local mytext = (plaintext ~= nil) and plaintext or anytext
					if mytext ~= nil and send_big_sms(number, mytext, some_mail.limit, some_mail.modem) then
						mbox:dele(k)
					end
				end
			end
		end
		--print('test   :', mbox:close())
		mbox:close()
	end
end

local function start()
	local reboot=0
	local find = read_output("grep -q /usr/bin/email_to_sms /etc/crontabs/root; echo $?")
	if tonumber(find) == 0 then
		os.execute("sed -i '\\/usr\\/bin\\/email_to_sms/d' /etc/crontabs/root")
		reboot=1
	end
	if tonumber(enabled) == 1 then
		local command=""
		local time_format = read_output("uci -q get email_to_sms.pop3.time")
		if time_format == "min" then
			local min_number = read_output("uci -q get email_to_sms.pop3.min")
			command = 'echo "*/'..tonumber(min_number)..' * * * * lua /usr/bin/email_to_sms read" >>/etc/crontabs/root'
		elseif time_format == "hour" then
			local hour_number = read_output("uci -q get email_to_sms.pop3.hour")
			command = 'echo "0 */'..tonumber(hour_number)..' * * * lua /usr/bin/email_to_sms read" >>/etc/crontabs/root'
		elseif time_format == "day" then
			local day_number = read_output("uci -q get email_to_sms.pop3.day")
			command = 'echo "0 0 */'..tonumber(day_number)..' * * lua /usr/bin/email_to_sms read" >>/etc/crontabs/root'
		end
		reboot=1
		print(command)
		os.execute(command)
	end
	if tonumber(reboot) == 1 then
		os.execute("/etc/init.d/cron restart")
	end
end

local function stop()
	local find = read_output("grep -q /usr/bin/email_to_sms /etc/crontabs/root; echo $?")
	if tonumber(find) == 0 then
		os.execute("sed -i '\\/usr\\/bin\\/email_to_sms/d' /etc/crontabs/root")
		os.execute("/etc/init.d/cron restart")
	end
end


local out =
[[unknown command line argument.

usage:
  email_to_sms read
  email_to_sms start
]]
--
-- Program execution
--
if #arg > 0 and #arg < 2 then
	if arg[1] == "read" then main()
	elseif arg[1] == "start" then start()
	elseif arg[1] == "stop" then stop()
	else
		print(out)
	end
else
	print(out)
end
