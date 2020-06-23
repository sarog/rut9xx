#!/usr/bin/lua

require "landing_page_functions"

local config = "coovachilli"
local page_config = "landingpage"
local hotspot_section
debug_enable = 0

uci:foreach(config, "general",
	function(s)
		if not hotspot_section then
			hotspot_section = s[".name"]
			debug("section: " .. hotspot_section)
		end
end)

local hotspot_section = hotspot_section and hotspot_section or  "hotspot1"
local auth_mode = uci:get(config, hotspot_section, "mode")
local page_title = uci:get("landingpage", "general", "title") or ""
local tos_enabled = uci:get(config, hotspot_section, "tos_enb") or "0"
local mac_pass = uci:get(config, hotspot_section, "mac_pass_enb") or "0"
local path = uci:get("landingpage", "general", "loginPage") or "/etc/chilli/www/hotspotlogin.tmpl"
local page
local replace_tags = {
	pageTitle = page_title
}


replace_tags.formHeader = [[<form name="myForm" method="post">]]
replace_tags.formFooter = [[</form>]]


debug("authmode:" ..auth_mode)
if auth_mode == "sms" then
	section = "phone"
else
	section = "welcome"
end

if auth_mode == "sms" then
	local label = get_values(page_config, "tel_number", "text") or ""
	replace_tags.inputUsername = [[<input type="hidden" name="UserName" value="-">]]
	replace_tags.inputPassword = [[<label class="cbi-value-tel tel_number">]] .. label .. [[</label><input id="focus_password" class="cbi-input-password" type="text" name="TelNum" pattern="[0-9+]{4,20}">]]
elseif auth_mode == "mac" then
	local label = get_values(page_config, "pass", "text") or ""

	if mac_pass == "1" then
		replace_tags.inputPassword = [[<label class="cbi-value-password pass">]] .. label .. [[</label><input id="focus_password" class="cbi-input-password" type="password" name="Password">]]
	else
		replace_tags.loginClass = "hidden_box"
		replace_tags.statusContent = ""
	end
else
	local p_label = get_values(page_config, "pass", "text") or ""
	local u_label = get_values(page_config, "username", "text") or ""
	replace_tags.inputUsername = [[<label class="cbi-value-title1 username">]] .. u_label .. [[</label><input class="cbi-input-user" type="text" name="UserName">]]
	replace_tags.inputPassword = [[<label class="cbi-value-password pass">]] .. p_label .. [[</label><input id="focus_password" class="cbi-input-password" type="password" name="Password">]]
end


if tos_enabled == "1" then --add terms of service
	if mac_pass ~= "1" and  auth_mode == "mac" then
		replace_tags.statusContent = get_values(page_config, page_config, "terms", "warning")
	end

	local link_tag = [[<a href="" onclick=open_tos(); "style="text-decoration: underline;">]]
	local terms_link = make_link(page_config, "terms", link_tag)

	replace_tags.inputTos = [[
		<script>
			function open_tos() {
				window.open('tos.lua', 'popup','width=700,height=500,scrollbars=yes,resizable=no,toolbar=no, directories=no,location=no,menubar=no,status=no,left=250,top=50');
			}
		</script>
		<input  type="checkbox" name="agree_tos" value="1"> ]] .. terms_link
end

if auth_mode == "sms" then
	local value = get_values(page_config, "send", "text") or "Send"
	local uri = os.getenv("REQUEST_URI")
	replace_tags.submitButton = [[<input type="submit" value="]] .. value .. [[" class="cbi-button cbi-button-apply3 send" name="send">]]
	replace_tags.smslink = [[If you already have password tap <a id="link" href="#">here</a>]]
else
	local value = get_values(page_config, "login", "text") or "Login"
	replace_tags.submitButton = [[<input type="submit" value="]] .. value .. [[" class="cbi-button cbi-button-apply3 login" name="button">]]

	if auth_mode == "sms" then
		local uri = os.getenv("REQUEST_URI")
		replace_tags.smslink = [[If you don't have password tap <a id="link" href="#">here</a> ]]
	end
end



if get_values(page_config, "link", "enabled") == "1" then --add link
	link_tag = [[<a id="link" href="]]  .. get_values(page_config, "link", "url") .. [[">]]
	replace_tags.link = make_link(page_config, "link", link_tag)
end

debug(section)
replace_tags.statusTitle = replace_tags.statusTitle or get_values(page_config, section, "title")
replace_tags.statusContent = replace_tags.statusContent or get_values(page_config, section, "text")
replace_tags.statusTitleId = section .. "_title"
replace_tags.statusContentId = section .. "_text"

debug(replace_tags.statusContent)

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
