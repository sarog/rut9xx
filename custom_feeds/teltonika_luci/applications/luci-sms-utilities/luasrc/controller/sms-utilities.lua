
module("luci.controller.sms-utilities", package.seeall)

function index()
	if  luci.tools.status.show_mobile() then
		entry({"admin", "services", "sms"}, alias("admin", "services", "sms", "sms-utilities"), _("SMS Utilities"), 85)

		entry({"admin", "services", "sms", "sms-utilities"},arcombine(cbi("sms-utilities/sms"), cbi("sms-utilities/sms-details")),_("SMS Utilities"), 1).leaf = true


		-- Do not show Call Utilities if using Huawei906s modem
		local utl = require "luci.util"
		local sys = require "luci.sys"		
		local moduleVidPid = utl.trim(sys.exec("uci get -q system.module.vid")) .. ":" .. utl.trim(sys.exec("uci get -q system.module.pid"))
		if moduleVidPid ~= "12D1:15C3" then
			entry({"admin", "services", "sms", "call-utilities"},arcombine(cbi("sms-utilities/call"), cbi("sms-utilities/call-details")),_("Call Utilities"), 2).leaf = true
		end
		entry({"admin", "services", "sms", "group"},arcombine(cbi("sms-utilities/group"), cbi("sms-utilities/group-details")), _("User Groups"), 3).leaf = true

		entry({"admin", "services", "sms","sms-manage"},  alias("admin", "services", "sms","sms-manage", "inbox"), _("SMS Management"), 4)
			entry({"admin", "services", "sms","sms-manage", "inbox"},  template("sms-utilities/sms-manage"), _("Read SMS"), 1).leaf = true
			entry({"admin", "services", "sms","sms-manage", "outbox"}, template("sms-utilities/sms-send"), _("Send SMS"), 2).leaf = true
			entry({"admin", "services", "sms","sms-manage", "configuration"}, cbi("sms-utilities/sim_configuration"), _("Storage"), 3).leaf = true

		entry({"admin", "services", "sms","remote_conf"},  alias("admin", "services", "sms","remote_conf","receive"), _("Remote Configuration"), 5)
			entry({"admin", "services", "sms","remote_conf","receive"},  cbi("sms-utilities/receive_configuration"), _("Receive"), 1).leaf =true
			entry({"admin", "services", "sms","remote_conf","send"},  cbi("sms-utilities/remote_configuration"), _("Send"), 2).leaf =true

		entry({"admin", "services", "sms","statistics"}, template("sms-utilities/sms_counter"), _("Statistics"), 6)

		entry({"admin", "services", "sms", "sms-init"}, call("sms_send"), nil).leaf = true
		entry({"admin", "services", "sms", "sms-del"}, call("sms_del"), nil).leaf = true
		entry({"admin", "services", "sms", "check_con"}, call("check_con"), nil).leaf = true
		entry({"admin", "services", "sms", "sms_stat"}, call("sms_status"), nil).leaf = true
	end
end

function sms_status()
	local tmpStatus = "/tmp/sms_status"
	local file = io.open(tmpStatus, "r")
	local output
	if file then
		output = file:read("*all")
		file:close()
		os.remove(tmpStatus)
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json(output)
end

function sms_send()
	local file = "/tmp/.smstext"
	local nw = require "luci.model.network"
	local function param(x)
		return luci.http.formvalue(x)
	end
	function sleep(n)
		os.execute("sleep " .. tonumber(n))
	end
	local num = param("cbi.number")
	local msg = param("cbi.message")
	if num and msg then
		local cod = nw.to_base64(msg)
		luci.sys.call("echo \""..cod.." \" >> "..file)
		luci.sys.call("gsmctl -S -b"..num.." >/tmp/response.log")
		os.remove(file)
	end

	luci.http.redirect(luci.dispatcher.build_url("/admin/services/sms/sms-manage/outbox"))
end

function sms_del()
	local file = "/tmp/response.log"
	local function param(x)
		return luci.http.formvalue(x)
	end
	local index_list = param("cbid.sms_del")
	if index_list then
		if type(index_list) == "table" then
			for _, index in ipairs(index_list) do
				--luci.sys.call("echo \"Table "..index.."\" >> /tmp/log.log")
				luci.sys.call("gsmctl -S -d "..index.." >"..file)
			end
		else
			--luci.sys.call("echo \"sms -d "..index_list..">/dev/null\" >> /tmp/log.log")
				luci.sys.call("gsmctl -S -d "..index_list.." >"..file)
		end
	end
	luci.http.redirect(luci.dispatcher.build_url("admin/services/sms/sms-manage"))
end

function check_con()
	local sys = require "luci.sys"
	local state
	local data = sys.exec("gsmctl -o")

	if data then
		state = data
	else
		state = false
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json(state)
end


