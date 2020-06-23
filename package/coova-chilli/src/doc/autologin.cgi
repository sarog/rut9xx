#!/usr/bin/lua

require "landing_page_functions"

local username = "-"
local password = "-"
local config = "coovachilli"
local page_config = "landingpage"
debug_enable = 0
local userpassword
local post_length = tonumber(os.getenv("CONTENT_LENGTH")) or 0
local params = {}
local system = "cloud4wi"
local names = get_names(system)

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
end

--query and form values
local section = "hotspot1"
local res = params[names['res']] or ""
local reason = params[names['reason']] or ""
local reply = params[names['reply']] or ""
reply = url_decode(reply)
local uamip = params[names['uamip']] or ""
if validate_ip(uamip) == false then
	os.exit(0)
end
local uamport = params[names['uamport']] or ""
local userurl = params[names['userurl']] or ""
local userurldecode = url_decode(userurl)
local challenge = params[names['challenge']] or ""
local redirurl = params[names['redirurl']]
local redirurldecode = url_decode(redirurl)
local mac = params[names['mac']]
if lengthvalidation(mac, 17, 17, "^[a-zA-Z0-9-]+$") == false then
	os.exit(0)
end

uci:foreach(config, "general",
	function(s)
		local ip = string.match(s.net, "(%d+.%d+.%d+.%d+)")
		if uamip == ip then
			section = s[".name"]
		end
end)

local restricted = uci:get("hotspot_scheduler", section, "restricted") or "0"
local auth_mode = uci:get(config, section, "mode")
local addvert_address = uci:get(config, section, "addvert_address") or ""
local page_title = uci:get("landingpage", "general", "title") or ""
local path = uci:get("landingpage", "general", "loginPage") or "/etc/chilli/www/hotspotlogin.tmpl"
local hotspot_number = string.match(section, "%d+") or "1"
local session_section = "unlimited" .. hotspot_number
local reached = check_limit(uamip, mac, session_section, config) or false
local mode = uci:get(config, section, "mode") or ""

if res ~= "success" then
	local uamsecret = uci:get(config, section, "uamsecret")
	local timeout = reached and 4 or 0
	debug("Logging in...")
	print("Content-type: text/html\n\n")
	hexchal = fromhex(challenge)

	if uamsecret then
		newchal  = md5.sum(hexchal..""..uamsecret)
 	else
 		newchal  = hexchal
 	end

	debug("Generating a CHAP response with the password and the")
	--Generate a CHAP response with the password and the
	--challenge (which may have been uamsecret encoded)
	response = md5.sumhexa("\0"..password..""..newchal)
	logonUrl = string.format("http://%s:%s/logon?username=%s&response=%s&%s=%s", uamip, uamport, urlencode(username), urlencode(response), urlencode(names["userurl"]), urlencode(userurl))

	print ([[<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
	<html>
		<head>
			<title>]] .. escapeHTML(page_title) .. [[ Login</title>
			<link rel="stylesheet" href="/luci-static/resources/loginpage.css">
			<meta http-equiv="Cache-control" content="no-cache">
			<meta http-equiv="Pragma" content="no-cache">]])
			if restricted ~= "1" then
			    print([[<meta http-equiv='refresh' content="]] .. escapeHTML(timeout) .. [[;url=']] .. logonUrl .. [['>]])
			end
		print([[</head>
	<body >
		<div style="width:100%;height:100%;margin:auto;">
			<div style="text-align: center;position: absolute;top: 50%;left: 50%;height: 30%;width: 50%;margin: -15% 0 0 -25%;">
				<div style="width: 280px;margin: auto;">]])
				if restricted == "1" then
                    print("Access restricted!</br>")
				elseif reached and mode ~= "add" then
                  if reached == 3 then
                    print("Time limit reached </br>")
                  else
                    print("Data limit reached </br>")
                  end
				else
				    print ([[<small><img src="/luci-static/default/wait.gif"/> redirecting...</small>]])
				end
				print([[</div>
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
	<LoginResultsURL>]]..logonUrl..[[</LoginResultsURL>
	</AuthenticationReply>
	</WISPAccessGatewayParam>
	-->
	</html>
	]])
	os.exit(0)
else
	print("Content-type: text/html\n\n")
	print ([[<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
		<html>
			<head>
				<meta http-equiv="Cache-control" content="no-cache">
				<meta http-equiv="Pragma" content="no-cache">
				<meta http-equiv="refresh" content="0; url=]] .. addvert_address .. [[">
			</head>

			<body>
				<div style="width:100%;height:100%;margin:auto;">
					<div style="text-align: center;position: absolute;top: 50%;left: 50%;height: 30%;width: 50%;margin: -15% 0 0 -25%;">
						<div style="width: 280px;margin: auto;">
							<small><img src="/luci-static/default/wait.gif"/> redirecting...</small>
						</div>
					</div>
				</div>
			</body>

		</html>
		]])
	os.exit(0)
end
