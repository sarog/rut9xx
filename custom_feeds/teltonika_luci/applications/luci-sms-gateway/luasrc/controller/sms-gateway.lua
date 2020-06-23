module("luci.controller.sms-gateway", package.seeall)

function index()
	local show = require("luci.tools.status").show_mobile()
	if  show then
		entry({"admin", "services", "sms_gateway"}, alias("admin", "services", "sms_gateway", "post_get"), _("SMS Gateway"), 85)
		entry({"admin", "services", "sms_gateway","post_get"}, cbi("sms_gateway/post_get"), _(" Post/Get"), 1).leaf = true
		entry({"admin", "services", "sms_gateway","email_to_sms"}, cbi("sms_gateway/pop3_ets"), _("Email To SMS"), 2).leaf = true
		entry({"admin", "services", "sms_gateway", "scheduled_messages"}, arcombine(cbi("sms_gateway/scheduled_messages"), cbi("sms_gateway/scheduled_messages-details")), _("Scheduled SMS"),4).leaf = true
		entry({"admin", "services", "sms_gateway","auto_reply"}, cbi("sms_gateway/auto_reply"), _("Auto Reply"), 6).leaf = true
		entry({"admin", "services", "sms_gateway","sms_forwarding"}, alias("admin", "services", "sms_gateway","sms_forwarding","forwarding_http"), _("SMS Forwarding"), 7)
		entry({"admin", "services", "sms_gateway","sms_forwarding","forwarding_http"}, cbi("sms_gateway/sms_forwarding"), _("SMS Forwarding To HTTP"), 1).leaf = true
		entry({"admin", "services", "sms_gateway","sms_forwarding","forwarding_sms"}, cbi("sms_gateway/forwarding_to_sms"), _("SMS Forwarding To SMS"), 2).leaf = true
		entry({"admin", "services", "sms_gateway","sms_forwarding","forwarding_smtp"}, cbi("sms_gateway/forwarding_to_smtp"), _("SMS Forwarding To Email"), 3).leaf = true
		entry({"admin", "services", "sms_gateway","smpp"}, cbi("sms_gateway/smpp"), _("SMPP"), 8).leaf = true
		entry({"admin", "services", "sms_gateway","sms_counter"}, call("sms_counter")).leaf = true
	end
end

function sms_counter()
	local sys = require "luci.sys"
	step = tonumber(luci.http.formvalue("step"))
	function get_values()
		local values = sys.exec("/sbin/sms_counter.lua value both")
		local line_counter = 1
		send = {}
		recieved = {}
		if values then
			for line in values:gmatch("[^\r\n]+") do
				send[line_counter] = string.gsub(line," .*","")
				if tonumber(send[line_counter]) == nil then
					send[line_counter] = nil
				end
				recieved[line_counter] = string.gsub(line,".* ","")
				if tonumber(recieved[line_counter]) == nil then
					recieved[line_counter] = nil
				end
				line_counter = line_counter + 1
			end
		end
		for i=1,2 do
			send[i] = send[i] or "0"
			recieved[i] = recieved[i] or "0"
		end
	end
	if step then
		luci.sys.exec("/sbin/sms_counter.lua reset SLOT" .. step)
	end
	get_values()
	rv = {
			send = send,
			recieved = recieved
	}
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end
