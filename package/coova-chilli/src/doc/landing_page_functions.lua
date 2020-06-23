require("uci")

uci = uci.cursor()
fs = require "luci.fs"
ip = require "luci.ip"
string = require('string')
find = string.find
gsub = string.gsub
char = string.char
byte = string.byte
format = string.format
match = string.match
gmatch = string.gmatch
md5 = require"md5"
bit = require"bit"

function escape_inner(str)
	return (string.gsub(str, "'", "'\\''"))
end

function escape(str)
	return "'" .. escape_inner(str) .. "'"
end

function escapeHTML(str)
	local escaped = str
	escaped = (string.gsub(escaped, "&", "&amp;"))
	escaped = (string.gsub(escaped, "<", "&lt;"))
	escaped = (string.gsub(escaped, ">", "&gt;"))
	escaped = (string.gsub(escaped, "\"", "&quot;"))
	escaped = (string.gsub(escaped, "'", "&#039;"))
	return escaped
end

function alphanumeric_validation(line)
	if line then
		line = string.gsub (line, "%W", "")
	end
	return line
end

function validate_ip(val)
	if val then
		return ip.IPv4(val) and true or false
	end

	return false
end

function validate_port(val)
	local val2 = string.match(val, "%d+")
	_port = tonumber(val2)
	return (val and val2 and val == val2 and _port > 0 and _port <= 65535)
end

function lengthvalidation(val, min, max, regex)
	if #val >= min and #val <= max then
		if regex then
			return (val:match(regex) ~= nil)
		else
			return true
		end
	end
	return false
end

function get_names()
	--Default names
	local names = {
		button = "button", send = "send", TelNum = "TelNum", res = "res", reason = "reason",
		sms = "sms", UserName = "UserName", reply = "reply", Password = "Password",
		redirurl = "redirurl", agree_tos = "agree_tos", expiration = "expiration"
	}

	names["uamip"] = uci:get("coovachilli", "url", "uamip") or "uamip"
	names["uamport"] = uci:get("coovachilli", "url", "uamport") or "uamport"
	names["userurl"] = uci:get("coovachilli", "url", "userurl") or "userurl"
	names["mac"] = uci:get("coovachilli", "url", "mac") or "mac"
	names["challenge"] = uci:get("coovachilli", "url", "challenge") or "challenge"

	return names
end

function get_values(config, section, option)
	local theme = uci:get(config, "general", "theme") or "custom"
	local value
	if theme ~= "custom" then
		local command = format("uci -c /etc/chilli/www/themes/ get %s.%s.%s", theme, section, option)
		value = getParam(command)
	else
		value = uci:get(config, section, option) or ""
	end
	return value
end

function debug(string)
	if debug_enable == 1 then
		os.execute("/usr/bin/logger -t hotspotlogin.cgi " .. escape(string))
	end
end

function replace(page, repl)
	if page and repl then
		local tag_name
		local repl_string
		for word in gmatch(page, "%$%a+%$") do
			tag_name = word:gsub("%$", "")

			if repl[tag_name] then
				repl_string = repl[tag_name]:gsub("%%", "%%%%")
			else
				repl_string = ""
			end
			page = page:gsub("%$" .. tag_name .. "%$", repl_string)
		end
	end
	return page
end

--To use MSCHAPv2 Authentication with,
--then uncomment the next two lines.
--local ntresponse = 1
--local chilli_response = '/usr/local/sbin/chilli_response'

--Uncomment the following line if you want to use ordinary user-password (PAP)
--for radius authentication.

function fromhex(str)
    return (gsub(str, '..', function (cc)
        return char(tonumber(cc, 16))
    end))
end

function tohex(str)
    return (gsub(str, '.', function (c)
        return format('%02x', byte(c))
    end))
end


function url_decode(str)
	if not str then return nil end
	str = string.gsub (str, "+", " ")
	str = string.gsub (str, "%%(%x%x)", function(h) return
		string.char(tonumber(h,16)) end)
	str = string.gsub (str, "\r\n", "\n")
	return str
end

function urlencode(str)
  if str then
    str = gsub(str, '\n', '\r\n')
    str = gsub(str, '([^%w ])', function(c)
      return format('%%%02X', byte(c))
    end)
    str = gsub(str, ' ', '+')
  end
  return str
end

function url_quote_remove(str)
	if str then
		str = gsub(str, "\'", "")
	end
	return str
end

function parse(str, sep, eq)
  if not sep then sep = '&' end
  if not eq then eq = '=' end
  local vars = {}
  for pair in gmatch(tostring(str), '[^' .. sep .. ']+') do
    if not find(pair, eq) then
      vars[urldecode(pair)] = ''
    else
      local key, value = match(pair, '([^' .. eq .. ']*)' .. eq .. '(.*)')
      if key then
        vars[url_decode(key)] = value
      end
    end
  end
  return vars
end

function getParam(string)
	local h = io.popen(string)
	local t = h:read()
	h:close()
	return t
end

function get_ifname(ip_addr)
	local result = getParam(format("ip addr | grep %s", escape(ip_addr)))
	local tun = string.match(result, "(tun%d+)")
	local ifname = "wlan0"

	if tun then
		local num = string.match(tun, "%d+")
		if num and tonumber(num) > 0 then
			ifname = format("wlan0-%s", num)
		end
	end

	debug("IFNAME: ".. ifname)
	return ifname
end

function check_limit(uamip, mac, section, config)
	if uamip then
		local option, start_time
		local ifname = get_ifname(uamip)
		local period = uci:get(config, section, "period")

		if period then
			if period == "3"  then
				option = "day"
			elseif period == "1" then
				option = "hour"
			elseif period == "2" then
				option = "weekday"
			end
		end

		if option then
			start_time = uci:get(config, section, option)
		end

		if mac and ifname and period and start_time then
			local command = format("/usr/sbin/statistics check_mac_all %s %s %s %s", escape(mac), escape(ifname), escape(period), escape(start_time))
			local used_data = getParam(command)

			if used_data then
				downloaded, uploaded, time = used_data:match("(%d+):(%d+):(%d+):")

				if downloaded then
					local download_limit = uci:get(config, section, "downloadlimit")
					if download_limit and tonumber(download_limit) <= tonumber(downloaded) then
						debug("Reached download limit " .. download_limit .. "<=" .. downloaded)
						return 1
					end
				end

				if uploaded then
					local upload_limit = uci:get(config, section, "uploadlimit")

					if upload_limit and tonumber(upload_limit) <= tonumber(uploaded) then
						debug("Reached upload limit")
						return 2
					end
				end

				if time then
					local time_limit = uci:get(config, section, "defsessiontimeout") or 0

					if tonumber(time_limit) > 0 and tonumber(time_limit) <= tonumber(time) then
						debug("Reached time limit")
						return 3
					end
				end
			end
		end
	end

	return false
end

function make_link(page_config, section, link_tag)
	local custom_link = get_values(page_config, section, "text")

	debug("custom_link " .. custom_link)

	if custom_link then
		local word = string.match(custom_link, '{(.*)}')

		if word then
			debug(word)
			local brackets_word = string.format("{%s}", word)
			debug(link_tag)
			local link = string.format("%s%s</a>", link_tag, word)
			custom_link = custom_link:gsub(brackets_word, link)
			debug(custom_link)
		else
			custom_link = string.format("%s%s</a>", link_tag, custom_link)
		end

		debug(custom_link)
	end

	return custom_link or ""
end

function check_result(res)
	local result = 0

	if res == "success" then -- If login successful
		debug("Result: success")
		result = 1
	elseif res == "failed" then --If login failed
		debug("Result: failed")
		result = 2
	elseif res == "logoff" then -- If logout successful
		debug("Result: logoff")
		result = 3
	elseif res == "already" then -- If tried to login while already logged in
		debug("Result: already")
		result = 4
	elseif res == "notyet" then -- If not logged in yet
		debug("Result: notyet")
		result = 5
	elseif res == "wispr" then -- If login from smart client
		debug("Result: wispr")
		result = 6
	elseif res == "popup1" then -- If requested a logging in pop up window
		debug("Result: popup1")
		result = 11
	elseif res == "popup2" then -- If requested a success pop up window
		debug("Result: popup2")
		result = 12
	elseif res == "popup3" then -- If requested a logout pop up window
		debug("Result: popup3")
		result = 13
	end

	return result
end

function get_subnet (ip_addr)
	local ip1, ip2, ip3, ip4= string.match(ip_addr, "(%d+).(%d+).(%d+).(%d+)")

	return ip3
end

function get_section(config, net)
	local section
	local remote_subnet = get_subnet(net)

	uci:foreach(config, "general", function(s)
		local subnet = get_subnet(s.net)

		if remote_subnet == subnet then
			section = s[".name"]
		end
	end)

	return section
end

function get_wifi_ssid(hotspot_section)
	local ssid

	uci:foreach("wireless", "wifi-iface",
		function(s)
			if s.hotspotid == hotspot_section then
				ssid = s.ssid												-- hotspot ssid now we can check if restricted
			end
	end)

	return ssid
end

function format_date(timestamp)
	return timestamp and os.date("%F %T", timestamp) or ""
end

function count_date(hotspot_section, config)
	local days_left, hours_left
	local time_left, end_time
	local period = uci:get(config, hotspot_section, "period") or "3"
	local timestamp = os.time()
	local year, month, weekday, day, hour = tonumber(os.date("%Y", timestamp)), tonumber(os.date("%m", timestamp)), tonumber(os.date("%w", timestamp)), tonumber(os.date("%d", timestamp)), tonumber(os.date("%H", timestamp))

	if period == "3" then --Period month
		local start_day = tonumber(uci:get(config, hotspot_section, "day") or "1")

		if start_day < day then
			month = month + 1
		end

		day = start_day
		hour = 0
	elseif period == "1" then --Period day
		local start_hour = tonumber(uci:get(config, hotspot_section, "hour") or "1")

		if start_hour < hour then
			day = day + 1
		end

		hour = start_hour
	elseif period == "2" then --period week
		local start_weekday = tonumber(uci:get(config, hotspot_section, "weekday") or "1")
		weekday = weekday == 0 and 7 or weekday

		if weekday ~= start_weekday then
			if start_weekday < weekday then
				day = day + (7 - (weekday - start_weekday))
			else
				day = day + (start_weekday - weekday)
			end
		end
		hour = 0
	end

	end_time = tonumber(os.time{year=year, month=month, day=day, hour=hour})
	time_left = end_time - timestamp

	if time_left > 86400 then
		days_left = math.floor(time_left / 86400)
		time_left = time_left % 86400
	end

	if time_left > 3600 then
		hours_left = math.floor(time_left / 3600)
		time_left = time_left % 3600
	end

	if time_left > 60 then
		minutes_left = math.floor(time_left / 60)
		time_left = time_left / 60
	end

	debug("today: " .. os.date("%x %X", timestamp))
	debug("End date: " .. os.date("%x %X", end_time))



	return {time = format_time(days_left, hours_left, minutes_left), date = os.date("%F %T", end_time)}
end

function format_time(days, hours, minutes)
	local string = ""

	if days then
		string = format("%s days ", days)
	end

	if hours then
		string = format("%s%s hours ", string, hours)
	end

	if minutes then
		string = format("%s%s minutes ", string, minutes)
	end

	return string
end

function make_date(page_config, section, exp_date)
	local text = get_values(page_config, section, "text")

	if text and exp_date then
		text = text:gsub("%%date", exp_date)
	end

	return text or ""
end

--Genrates pin code and if it exists generates another one
function generate_code(ifname)
	math.randomseed( os.time())
	local code = 1000

	for i=1, 10 do
		code = math.random(1000,9999)
		local res = getParam(string.format("grep -wc %d /etc/chhilli/%s/smsusers", code, ifname))

		if res == "0" then
			break
		end
	end

	return code
end

function print_html_error(error_html)
	print("Content-type: text/html\n\n")
	print(string.format([[<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"><html><head></head><body>%s</body></html>]], error_html))
end
