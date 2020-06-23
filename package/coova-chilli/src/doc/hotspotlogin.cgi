#!/usr/bin/lua

require "landing_page_functions"

local config = "coovachilli"
local page_config = "landingpage"
local loginpath = "/cgi/hotspotlogin.cgi"
--Debug variable must be global
debug_enable = 0
local userpassword
local post_length = tonumber(os.getenv("CONTENT_LENGTH")) or 0
local remote_addr = os.getenv("REMOTE_ADDR") or ""
local hotspot_section = get_section(config, remote_addr) or "hotspot1"
local params = {}
local names = get_names()
local username = ""
local mac = ""
local challenge = ""

if os.getenv ("REQUEST_METHOD") == "POST" and post_length > 0 then
	debug("Request method post, reading stdin")
	POST_DATA = io.read (post_length)  -- read stdin
	if POST_DATA then
		debug("Parsing data")
		params = parse(POST_DATA)
	else
		debug("Cant get form data")
	end
elseif os.getenv ("REQUEST_METHOD") == "GET" then
	debug("Request method get")
	if os.getenv("QUERY_STRING") then
		query = os.getenv("QUERY_STRING")
	end

	if query then
		debug("Parsing data")
		params = parse(query)
	else
		debug("Can't get query string")
	end

	mac = params[names['mac']] or "error"
	if not mac == "error" then
		if lengthvalidation(mac, 17, 17, "^[a-fA-F0-9-]+$") == false then
			print_html_error("<h1>MAC address validation failed!</h1>")
			os.exit(0)
		end
	end
end

--query and form values

mac = params[names['mac']] or "error"
debug("mac |"..mac.."|")
if not mac == "error" then
	if lengthvalidation(mac, 17, 17, "^[a-fA-F0-9-]+$") == false then
		print_html_error("<h1>MAC address validation failed!</h1>")
		os.exit(0)
	end
end

local res = alphanumeric_validation(params[names['res']]) or ""
local tel_num = params[names['TelNum']] or ""
username = alphanumeric_validation(params[names['UserName']]) or ""
local button = alphanumeric_validation(params[names['button']]) or ""
local send = alphanumeric_validation(params[names['send']]) or ""
local reason = alphanumeric_validation(params[names['reason']]) or ""
local sms = alphanumeric_validation(params[names['sms']]) or ""
local password = params[names['Password']] or ""
password = url_decode(password)

if res ~= "success" then
	challenge = params[names['challenge']] or "error"
	if lengthvalidation(challenge, 32, 32, "^[a-fA-F0-9]+$") == false then
		challenge = ""
	end
end

local uamip = params[names['uamip']] or "error"
if not uamip == "error" then
	if validate_ip(uamip) == false then
		print_html_error("<h1>IP address validation failed!</h1>")
		os.exit(0)
	end
end

local uamport = params[names['uamport']] or "error"
if not uamport == "error" then
	if validate_port(uamport) == false then
		print_html_error("<h1>Port number validation failed!</h1>")
		os.exit(0)
	end
end

local userurl = params[names['userurl']] or ""
local userurldecode = url_decode(userurl)
local res = alphanumeric_validation(params[names['res']]) or ""
local tos = params[names['agree_tos']] or "0"
local hotspot_number = string.match(hotspot_section, "%d+") or "1"
local session_section = "unlimited" .. hotspot_number
local ssid = get_wifi_ssid(hotspot_section) or ""
local is_restricted = uci:get("hotspot_scheduler", hotspot_section, "restricted") or 0 --the restriction flag
local mac_pass = uci:get(config, hotspot_section, "mac_pass_enb") or "0"
local auth_mode = uci:get(config, hotspot_section, "mode") or ""
local page_title = uci:get("landingpage", "general", "title") or ""
local tos_enabled = uci:get(config, hotspot_section, "tos_enb") or "0" --get_values("terms", "enabled") or "0"
local path = uci:get("landingpage", "general", "loginPage") or "/etc/chilli/www/hotspotlogin.tmpl"

local hs_protocol = uci:get(config, hotspot_section, "protocol") or "http"
local hs_http_def

if hs_protocol == "https" then
	hs_http_def = { "0.0.0.0:444" }
else
	hs_protocol = "http"
	hs_http_def = { "0.0.0.0:81" }
end

local hs_http_port = uci:get("uhttpd", "hotspot", "listen_" .. hs_protocol) or hs_http_def

hs_http_port = string.gsub(hs_http_port[1], ".*:", "")

local page
local reached = false
local tos_accepted = true
local replace_tags = {
	pageTitle = page_title
}

if auth_mode == "sms" or auth_mode == "mac" then
	reached = check_limit(uamip, mac, session_section, config)
end

if tos_enabled == "1" then
 	if button and button ~= "" and tos ~= "1" then
		debug("Terms of servise not aceepted")
		tos_accepted = false
 		button = ""
 		res = "failed"
 	end
end

if auth_mode == "extrad" then
	debug("External radius authentication")
	local auth_proto = uci:get(config, hotspot_section, "auth_proto") or "pap"

	if auth_proto == "pap" then
		userpassword = 1
	end
elseif auth_mode == "intrad" then
	debug("Internal radius authentication")
		userpassword = 1
end

if ((button and button ~= "") or res == "wispr" and username ~= "") and is_restricted ~= "1" then
	local uamsecret = uci:get(config, hotspot_section, "uamsecret")
	print("Content-type: text/html\n\n")
	hexchal = fromhex(challenge)

	if uamsecret then
		debug("Uamsecret \""..uamsecret.."\" defined")
		newchal  = md5.sum(hexchal..""..uamsecret)
 	else
		debug("Uamsecret not defined")
 		newchal  = hexchal
 	end

 	if ntresponse == 1 then
		debug("Encoding plain text into NT-Password ")
		--Encode plain text into NT-Password
		--response = chilli_response -nt "$challenge" "$uamsecret" "$username" "$password"
		logonUrl = "http://"..uamip..":"..uamport.."/logon?username="..username.."&ntresponse="..response
 	elseif userpassword == 1 then
		debug("Encoding plain text password with challenge")
		--Encode plain text password with challenge
		--(which may or may not be uamsecret encoded)

		--If challange isn't long enough, repeat it until it is
		while string.len(newchal) < string.len(password) do
			newchal = newchal..""..newchal
		end
		local result = ""
		local index = 1

		while index <= string.len(password) do
			result = result .. char(bit.bxor(string.byte(password, index), string.byte(newchal, index)))
			index = index + 1
		end

		pappassword = tohex(result)
		logonUrl = "http://"..uamip..":"..uamport.."/logon?username="..username.."&password="..pappassword

	else
		debug("Generating a CHAP response with the password and the challenge (which may have been uamsecret encoded)")
		response = md5.sumhexa("\0"..password..""..newchal)
		logonUrl = "http://"..uamip..":"..uamport.."/logon?username="..username.."&response="..response.."&"..names["userurl"].."="..userurl
	end

	print ([[<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
	<html>
		<head>
			<title>]] .. escapeHTML(page_title) .. [[ Login</title>
			<link rel="stylesheet" href="/luci-static/resources/loginpage.css">
			<meta http-equiv="Cache-control" content="no-cache">
			<meta http-equiv="Pragma" content="no-cache">
			<meta http-equiv='refresh' content="0;url=']] .. logonUrl .. [['>
		</head>
	<body >
		<div style="width:100%;height:100%;margin:auto;">
			<div style="text-align: center;position: absolute;top: 50%;left: 50%;height: 30%;width: 50%;margin: -15% 0 0 -25%;">
				<div style="width: 280px;margin: auto;">
					<small><img src="/luci-static/default/wait.gif"/> logging...</small>
				</div>
			</div>
		</div>
	</body>
	<!--
	<?xml version="1.0" encoding="UTF-8"?>
	<WISPAccessGatewayParam
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	xsi:noNamespaceSchemaLocation="http://www.acmewisp.com/WISPAccessGatewayParam.xsd">
	<AuthenticationReply>
	<MessageType>120</MessageType>
	<ResponseCode>201</ResponseCode>
	<LoginResultsURL>]] .. logonUrl .. [[</LoginResultsURL>
	</AuthenticationReply>
	</WISPAccessGatewayParam>
	-->
	</html>
	]])
        os.exit(0)
end

if send and send ~= "" and tel_num then
	local ifname = get_ifname(uamip)
	local pass = generate_code(ifname) or "0000"

	tel_num = tel_num:gsub("%%2B", "+")
	if lengthvalidation(tel_num, 3, 16, "^+?[0-9]+$") == false then
		res = "notyet"
		sms = "error"
	else
		local exists = getParam("grep \"" .. tel_num .. "\" /etc/chilli/" .. ifname .. "/smsusers")
		local user = string.format("%s", pass)
		local uri = os.getenv("REQUEST_URI")
		local message = string.format("%s Password - %s  \n Link - " .. hs_protocol .. "://%s:%s%s?challenge=%s&uamport=%s&uamip=%s&userurl=%s&UserName=%s&button=1", tel_num, pass, uamip, hs_http_port, uri, challenge, uamport, uamip, userurl, pass)
		local smsotp_mesg = string.format("%s;%s", tel_num, pass)
		message = getParam(string.format("/usr/sbin/gsmctl -Ss %s", escape(message)))

		if message == "OK" then
			os.execute("echo \""..smsotp_mesg.."\" >> /tmp/smsotp.log")
			sms = "sent"
			if exists then
				os.execute("sed -i 's/" ..exists.. "/" ..user.. "/g' /etc/chilli/" .. ifname .. "/smsusers")
			else
				os.execute("echo \"" ..user.. "\" >>/etc/chilli/" .. ifname .. "/smsusers")
			end
		else
			res = "notyet"
			sms = "error"
		end
	end
end

--Default: It was not a form request
local result = check_result(res)
--Otherwise it was not a form request
--Send out an error message
if result == 0 then
	section = "warning"
--If login successful, not logged in yet, requested a success pop up window
elseif result == 1 or result == 4 or result == 12 then
	local web_page = uci:get(config, hotspot_section, "web_page")
	section = "success"
	link_tag = [[<a href='http://]] .. uamip .. [[:]] .. uamport .. [[/logoff'>]]
	replace_tags.loginLogout = make_link(page_config, "logout_link", link_tag)
	replace_tags.loginLogoutClass = "logout_link"

	if userurldecode and userurldecode ~= "" then
		if web_page == "link" then
			link_tag = [[<a href=']] .. url_quote_remove(userurldecode) .. [['> ]]
			replace_tags.requestedWeb = [[<br>]] .. make_link(page_config, "requested_web", link_tag)
		elseif web_page == "auto" or web_page == "custom" then
			if web_page == "custom" then
				userurldecode = uci:get(config, hotspot_section, "cust_address")
				its_logout = false
			else
				local uamlogoutip = uci:get(config, hotspot_section, "uamlogoutip")
				local its_logout = find(userurldecode, uamlogoutip)
			end

			if not its_logout then
				print("Content-type: text/html\n\n")
				print([[<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
					<html>
						<head>
							<meta http-equiv='refresh' content='0;url=]] .. url_quote_remove(userurldecode) .. [['>
						</head>
						<body >
						</body>
					</html>]])
				os.exit(0)
			end
		end
	end
--If logout successful, logout pop up window
elseif result == 3 or result == 13 then
	section = "logout"
	link_tag = [[<a href='http://]].. uamip .. [[:]] .. uamport .. [[/prelogin'>]]
	replace_tags.loginLogout = make_link(page_config, "login_link", link_tag)
	replace_tags.loginLogoutClass = "login_link"
--If logout successful, not logged in yet
elseif result == 2 or result == 5 then
	replace_tags.formHeader = [[<form name="myForm" method="post" action="]] .. loginpath .. [[">
			<INPUT TYPE="hidden" NAME="challenge" VALUE="]] .. escapeHTML(challenge) .. [[">
			<INPUT TYPE="hidden" NAME="]] .. escapeHTML(names["uamip"]) .. [[" VALUE="]] .. escapeHTML(uamip) .. [[">
			<INPUT TYPE="hidden" NAME="]] .. escapeHTML(names["uamport"]) .. [[" VALUE="]] .. escapeHTML(uamport) .. [[">
			<INPUT TYPE="hidden" NAME="]] .. escapeHTML(names["userurl"]) .. [[" VALUE="]] .. urlencode(userurldecode) .. [[">
			<INPUT TYPE="hidden" NAME="res" VALUE="]] .. escapeHTML(res) .. [[">]]
	replace_tags.formFooter = [[</form>]]

	debug("authmode:" ..auth_mode)
	if auth_mode == "sms" and result ~= 2 then
		section = "password"

		if sms == "notsent" then
			section = "phone"
		elseif sms == "error" then
			section = "error"
		end
	elseif result == 2 then
		section = "failed"
		debug("tos_enabled " .. tos_enabled)
		if tos_enabled == "1" and not tos_accepted and
			(not reason or reason == "")
		then
			debug("tos error")
			section = "terms"
			replace_tags.statusTitle = get_values(page_config, "welcome", "title")
			replace_tags.statusContent = get_values(page_config, section, "warning")
		end

		if reason then
			debug("reason " .. reason)
			if reason == "blocked" then
				section = "data_limit"
			elseif reason == "timelimit" then
				section = "time_limit"
			end

			if reason == "blocked" or section == "time_limit" then
                local display_limit = get_values(page_config, section, "enabled")
                local expiration = format_date(params[names['expiration']])

                if display_limit then
                    replace_tags.statusContent = get_values(page_config, section, "text")
                    replace_tags.dateUse = make_date(page_config, "limit_expiration", expiration)
                end
            end
		end
	elseif result == 5 then
		section = "welcome"
	end

	if auth_mode == "sms" then
		replace_tags.inputUsername = [[<input type="hidden" name="UserName" value="-">]]

		if sms == "notsent" or sms == "error" then
			if not reached then
				local label = get_values(page_config, "tel_number", "text") or ""
				replace_tags.inputPassword = [[<label class="cbi-value-tel tel_number">]] .. escapeHTML(label) .. [[</label><input id="focus_password" class="cbi-input-password" type="text" name="TelNum" pattern="[0-9+]{4,20}">]]
			end
		elseif not reached then
			local label = get_values(page_config, "pass", "text") or ""
			replace_tags.inputPassword = [[<label class="cbi-value-password pass">]] .. escapeHTML(label) .. [[</label><input id="focus_password" class="cbi-input-password" type="text" name="UserName">]]
		end
	elseif auth_mode == "mac" then
		if mac_pass == "1" and not reached then
			local label = get_values(page_config, "pass", "text") or ""
			replace_tags.inputPassword = [[<label class="cbi-value-password pass">]] .. escapeHTML(label) .. [[</label><input id="focus_password" class="cbi-input-password" type="password" name="Password">]]
		else
			replace_tags.loginClass = "hidden_box"
			replace_tags.statusContent = ""
			replace_tags.inputPassword = [[<input id="focus_password" class="cbi-input-password" type="password" name="Password" value="-">]]
		end
	else
		local u_label = get_values(page_config, "username", "text") or ""
		local p_label = get_values(page_config, "pass", "text") or ""
		replace_tags.inputUsername = [[<label class="cbi-value-title1 username">]] .. escapeHTML(u_label) .. [[</label><input class="cbi-input-user" type="text" name="UserName">]]
		replace_tags.inputPassword = [[<label class="cbi-value-password pass">]] .. escapeHTML(p_label) .. [[</label><input id="focus_password" class="cbi-input-password" type="password" name="Password">]]
	end

	if tos_enabled == "1" and not reached then --add terms of service
		if (auth_mode == "sms" and (sms ~= "notsent" and sms ~= "error")) or (auth_mode ~= "sms")then
			local link_tag = [[<a href="tos.lua" target="_blank" "style="text-decoration: underline;">]]
			local terms_link = make_link(page_config, "terms", link_tag)

			replace_tags.inputTos = [[
				<input type="checkbox" name="agree_tos" value="1"> ]] .. terms_link

			if (auth_mode == "mac" and mac_pass ~= "1") or auth_mode == "sms" then
				replace_tags.statusContent = get_values(page_config, "terms", "warning")
			end
		end


	end

	if is_restricted == "1" then
		replace_tags.submitButton = [[Access restricted!]]
	else
		if not reached then
			if auth_mode == "sms" and (sms == "notsent" or sms == "error") then
				local value = get_values(page_config, "send", "text")
				replace_tags.submitButton = [[<input type="submit" value="]] .. escapeHTML(value) .. [[" class="cbi-button cbi-button-apply3 send" name="send">]]
				local uri = os.getenv("REQUEST_URI")
				local link_addr = string.format(hs_protocol .. "://%s:%s%s?res=notyet&uamip=%s&%s=%s&challenge=%s&%s=%s",
								uamip, hs_http_port, uri, uamip, urlencode(names["uamport"]), urlencode(uamport), urlencode(challenge), urlencode(names["userurl"]), urlencode(userurl))
				replace_tags.smslink = [[If you already have password tap <a id="link" href="]] .. link_addr .. [[">here</a>]]
			else
				local value = get_values(page_config, "login", "text")
				replace_tags.submitButton = [[<input type="submit" value="]] .. escapeHTML(value) .. [[" class="cbi-button cbi-button-apply3 login" name="button">]]

				if auth_mode == "sms" then
					local uri = os.getenv("REQUEST_URI")
					local link_addr = string.format(hs_protocol .. "://%s:%s%s?res=notyet&uamip=%s&%s=%s&challenge=%s&%s=%s&sms=notsent",
									uamip, hs_http_port, uri, uamip, urlencode(names["uamport"]), urlencode(uamport), urlencode(challenge), urlencode(names["userurl"]), urlencode(userurl))
					replace_tags.smslink = [[If you don't have password tap <a id="link" href="]] .. link_addr .. [[">here</a> ]]
				end
			end
		end
	end
end

if get_values(page_config, "link", "enabled") == "1" then --add link
	link_tag = [[<a id="link" href="]] .. get_values(page_config, "link", "url") .. [[">]]
	replace_tags.link = make_link(page_config, "link", link_tag)
end

if not reached then
	replace_tags.statusContent = replace_tags.statusContent or get_values(page_config, section, "text")
else
	left = count_date(session_section, config)

	if reached == 1 or reached == 2 then
		section = "data_limit"
		local display_data_limit = get_values(page_config, section, "text") or ""
		replace_tags.statusContent = display_data_limit
		replace_tags.dateUse = make_date(page_config, "limit_expiration", left.date)
	elseif reached == 3 then
		section = "time_limit"
		local display_time_limit = get_values(page_config, section, "text") or ""
		replace_tags.statusContent = display_time_limit
		replace_tags.dateUse = make_date(page_config, "limit_expiration", left.date)
	end
	debug("replace_tags.statusContent " .. replace_tags.statusContent)
end

replace_tags.statusTitle = replace_tags.statusTitle or get_values(page_config, section, "title")
replace_tags.statusTitleId = section .. "_title"
replace_tags.statusContentClass = section .. "_text"

if path then
	local file = assert(io.open(path, "r"))
	template = file:read("*all")
	file:close()
	page = replace(template, replace_tags)
end

-- HTTP header
print [[
Content-Type: text/html; charset=utf-8
]]
--Print all page
print(page)
