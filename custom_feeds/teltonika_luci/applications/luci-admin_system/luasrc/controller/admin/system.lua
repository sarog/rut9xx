
module("luci.controller.admin.system", package.seeall)
eventlog = require'tlt_eventslog_lua'

function index()
	local uci = require "luci.model.uci".cursor()
	entry({"admin", "system"}, alias("admin", "system", "admin"), _("System"), 50).index = true
	-- 	entry({"admin", "system", "system"}, cbi("admin_system/system"), _("System"), 1)
	entry({"admin", "system", "clock_status"}, call("action_clock_status"))

	entry({"admin", "system", "admin"},  alias("admin", "system", "admin", "general"), _("Administration"), 3)

	entry({"admin", "system", "admin", "general"}, cbi("admin_system/admin"), _("General"), 1).leaf = true
	entry({"admin", "system", "admin", "troubleshoot"}, cbi("admin_system/admin_troubleshoot"), _("Troubleshoot"), 2).leaf = true
	entry({"admin", "system", "admin", "backup"}, call("action_backup"), _("Backup"), 3).leaf = true
	entry({"admin", "system", "admin", "get_backup_info"}, call("check_user_backup"), nil, nil)
	entry({"admin", "system", "admin", "user_config"}, call("handle_user_configuration"), nil, nil)

	entry({"admin", "system", "admin", "auto"}, call("download_conf"), nil, nil)
	entry({"admin", "system", "admin", "check_status"}, call("check_status"), nil, nil)
	entry({"admin", "system", "admin", "download_backup"}, call("download_from_server"), nil, nil)
	entry({"admin", "system", "admin", "upgrade"}, call("apply_config"), nil, nil)
	entry({"admin", "system", "admin", "check_download"}, call("check_download_state"), nil, nil)
	entry({"admin", "system", "admin", "access_control"}, alias("admin", "system", "admin", "access_control", "general"), _("Access Control"), 5)

	entry({"admin", "system", "admin", "access_control", "general"}, cbi("admin_system/admin_access_control"), _("General"), 1).leaf = true
	entry({"admin", "system", "admin", "access_control", "safety"}, cbi("admin_system/safety"), _("Safety"), 2).leaf = true
	entry({"admin", "system", "admin", "diagnostics"}, template("admin_system/diagnostics"), _("Diagnostics"), 6).leaf = true
	entry({"admin", "system", "admin", "clonemac"}, template("admin_system/clonemac"), _("MAC Clone"), 7).leaf = true
	entry({"admin", "system", "admin", "overview"}, cbi("admin_system/overview_setup"), _("Overview"), 8).leaf = true
	entry({"admin", "system", "admin", "monitoring"}, cbi("admin_system/admin_access_control_remote"), _("RMS"), 9).leaf = true
	entry({"admin", "system", "admin", "root_ca"}, template("admin_system/root_ca"), _("Root CA"), 10).leaf = true

	entry({"admin", "system", "admin", "upload_ca"}, call("upload_cert_file"), nil, nil)

	local wimax_file
	local f=io.open("/tmp/run/wimax","r")
	if f~=nil then
		io.close(f)
		wimax_file = true
	else
		wimax_file = false
	end
	if(wimax_file) then
		entry({"admin", "system", "admin", "wimax"}, cbi("admin_system/wimax"), _("WiMAX"), 11).leaf = true
	end

	entry({"admin", "system", "admin", "xhr_the_data"}, call("get_page_status"), nil, nil)
	entry({"admin", "system", "admin", "xhr_gps_time"}, call("get_gps_time"), nil, nil)

	entry({"admin", "system", "trdownload"}, call("trdownload"))
	entry({"admin", "system", "trdownload1"}, call("trdownload", "topology"))
	entry({"admin", "system", "tcpdumppcap"}, call("tcpdumppcap"))
	entry({"admin", "system", "uhttpdcrt"}, call("uhttpd_crt_dl"))

	entry({"admin", "system", "reboot"}, call("action_reboot"), _("Reboot"), 9)


	--~ Page for forced password change
	local pass_change = uci:get("teltonika", "sys", "pass_changed")

	if pass_change == "0" then
		entry({"admin", "system", "password"}, cbi("admin_system/password"), _("Password"), 10)
	end

	entry({"admin", "system", "diag_ping"}, call("diag_ping"), nil).leaf = true
	entry({"admin", "system", "diag_nslookup"}, call("diag_nslookup"), nil).leaf = true
	entry({"admin", "system", "diag_traceroute"}, call("diag_traceroute"), nil).leaf = true
end

function is_connection_available()
	local auto_update_conf_err = luci.sys.call("/usr/sbin/auto_update_conf.sh check")
	if auto_update_conf_err == 0 then
		return 0
	end
	return auto_update_conf_err
end

function check_download_state()
	download_err = is_connection_available()
	local rv = {
		download_err = download_err
	}
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
	return
end

function render_backup_and_throw_error(redirect)
	local auto_update_conf_err 	= is_connection_available()
	local error_message 		= "Failed to download backup."

	if auto_update_conf_err == 0 then
		return
	end

	if auto_update_conf_err == 7 and redirect == 1 then
		error_message = error_message .. " Connection dropped."
	end

	if auto_update_conf_err == 7 and redirect == 0 then
		error_message = error_message .. " Failed to establish connection."
	end

	if auto_update_conf_err == 8 then
		error_message = error_message .. " Failed to locate configuration file."
	end

	if auto_update_conf_err == 14 then
		error_message = error_message .. " Potential MIM attack."
	end

	luci.template.render("admin_system/backup", {
		download_error 	= 1,
		error_message 	= error_message .. " (" .. "Erorr: " .. auto_update_conf_err .. ")"
	})
	return
end

function upload_cert_file()
	local path = "/etc/cacert.pem"
	luci.http.setfilehandler(
		function(meta, chunk, eof)
			if chunk then
				if not fp then
					fp = io.open(path, "w")
				end
				fp:write(chunk)
			end
			if eof then
				fp:close()
			end
		end)
	luci.sys.call("chmod 400 /etc/cacert.pem")
	luci.template.render("admin_system/root_ca", {})
end

function download_conf()

	if not (is_connection_available() == 0) then
		render_backup_and_throw_error(0)
		return
	end

	luci.sys.exec("sysupgrade --create-backup /tmp/backup.tar")
	luci.template.render("admin_system/download_backup", {})
end

function download_from_server()
	--	luci.sys.call("rm -f /tmp/config.tar.gz")
	luci.sys.call("/usr/sbin/auto_update_conf.sh download")
end

function apply_config()
	luci.sys.call("rm -rf /tmp/new_config_dir; mkdir /tmp/new_config_dir 2> /dev/null");
	luci.sys.call("tar -xzf /tmp/config.tar.gz -C /tmp/new_config_dir");

	local write_new_config_ok = check_backup()
	local upload = luci.http.formvalue("archive")
	if upload and #upload > 0 then
		if write_new_config_ok == 1 then
			luci.sys.exec("rm -f /tmp/new_config_dir/etc/config/hwinfo")
			luci.sys.exec("rm -f /tmp/new_config_dir/etc/inittab")
			luci.sys.exec("cp -rf /tmp/new_config_dir/etc /")
			local rms_status = tonumber(luci.sys.exec("uci get -q rms_connect_mqtt.rms_connect_mqtt.enable") or 0)
			luci.sys.call("/sbin/crt_rms r >/dev/null 2>/dev/null")
			if rms_status == 0 then
				luci.sys.call("uci set rms_connect_mqtt.rms_connect_mqtt.enable='0'")
				luci.sys.call("uci set rms_connect.rms_connect.enable='0'")
				luci.sys.call("uci set openvpn.teltonika_auth_service.enable='0'")
			else
				luci.sys.call("uci set rms_connect_mqtt.rms_connect_mqtt.enable='1'")
				luci.sys.call("uci set rms_connect.rms_connect.enable='1'")
				luci.sys.call("uci set openvpn.teltonika_auth_service.enable='1'")
			end
			luci.sys.exec("uci commit rms_connect")
			luci.sys.exec("uci commit rms_connect_mqtt")
			luci.sys.exec("uci commit openvpn")
			luci.sys.call("cp /rom/etc/uci-defaults/* /etc/uci-defaults/ 2> /dev/null")
			luci.template.render("admin_system/applyreboot")
			t = {requests = "insert", table = "EVENTS", type="Web UI", text="Configuration was restored from backup!"}
			eventlog:insert(t)
			luci.sys.reboot()
		else
			luci.sys.exec("rm -rf /tmp/new_config_dir/")
			local file_err_code = 1
			luci.template.render("admin_system/backup", {
				file_err_code = file_err_code
			})
		end
	else
		luci.template.render("admin_system/backup", {

		})
	end
end

function check_status()
	local conf = tonumber(luci.sys.exec("uci get -q auto_update.auto_update.config_size") or 0)
	local number = tonumber(luci.sys.exec("ls -al /tmp/config.tar.gz | awk -F ' ' '{print $5}'") or 0)
	local size = "0 %"
	if conf~= nil and number~=nil then
		if tonumber(conf) > 0 and tonumber(number) > 0 then
			number = tonumber(number)*100/tonumber(conf)
			if tonumber(number)==100 then
				size = "done"
			else
				number=string.format("%.0f",number)
				size = number.." %"
			end
		end
	end

	local rv = {
		uptime = size
	}
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
	return
end

function get_page_status()
	local uci = require("luci.model.uci").cursor()
	require "luci.fs"
	require "luci.tools.status"
	luci.util = require "luci.util"

	local statusValue = luci.http.formvalue("status")
	if statusValue == "2" then
		local connectionType = luci.http.formvalue("conn_type")
		local hostname = luci.http.formvalue("hostname")
		local port = luci.http.formvalue("port")
		local uciConnType = uci:get("rms_connect_mqtt", "rms_connect_mqtt", "enable")
		local uciHostname = uci:get("rms_connect_mqtt", "rms_connect_mqtt", "remote")
		local uciPort = uci:get("rms_connect_mqtt", "rms_connect_mqtt", "port")
		local hasChanges = false
		if connectionType ~= "" and connectionType ~= uciConnType then
			uci:set("rms_connect_mqtt", "rms_connect_mqtt", "enable", connectionType)
			if connectionType == "2" then
				uci:set("rms_connect", "rms_connect", "enable", "1")
				uci:set("openvpn", "teltonika_auth_service", "enable", "1")
			else
				uci:set("rms_connect", "rms_connect", "enable", connectionType)
				uci:set("openvpn", "teltonika_auth_service", "enable", connectionType)
			end
			hasChanges = true
		end
		if hostname ~= "" and hostname ~= uciHostname then
			uci:set("rms_connect_mqtt", "rms_connect_mqtt", "remote", hostname)
			uci:set("openvpn", "teltonika_auth_service", "remote", hostname)
			hasChanges = true
		end
		if port ~= "" and port ~= uciPort then
			uci:set("rms_connect_mqtt", "rms_connect_mqtt", "port", port)
			hasChanges = true
		end
		if hasChanges then
			uci:save("rms_connect_mqtt")
			uci:commit("rms_connect_mqtt")
			uci:save("openvpn")
			uci:commit("openvpn")
			uci:save("rms_connect")
			uci:commit("rms_connect")
		end
		luci.sys.exec("echo -ne '0\n0\n0\n\n' > /tmp/rms_data;/etc/init.d/rms_connect restart;sleep 2")
	end

	if statusValue == "1" or statusValue == "2" then
		local status, connection_state, router_ip, serial_nbr, lan_mac, next_try, is_connected, error, error_text, current_timestamp
		status = uci:get("rms_connect_mqtt", "rms_connect_mqtt", "enable")
		is_connected = luci.util.trim(luci.sys.exec("sed -n '2p' < /tmp/rms_data"))
		if is_connected == "" then
			is_connected = "0"
		end
		error = luci.util.trim(luci.sys.exec("sed -n '3p' < /tmp/rms_data"))
		if error == "" then
			error = "0"
		end
		error_text = luci.util.trim(luci.sys.exec("sed -n '4p' < /tmp/rms_data | tr '\n' ' '"))
		if error_text == "" then
			error_text = ""
		end

		router_ip = "N/A"
		serial_nbr = luci.util.trim(luci.sys.exec("mnf_info sn"))
		lan_mac = (string.upper(luci.util.trim(luci.sys.exec("mnf_info mac")))):gsub(("."):rep(2),"%1:"):sub(1,-2)
		next_try = luci.util.trim(luci.sys.exec("sed -n '1p' < /tmp/rms_data"))
		if next_try == "" or tonumber(next_try) == nil or tonumber(next_try) == 0 then
			next_try = os.time()
		end
		current_timestamp = os.time()
		next_try = (next_try) - current_timestamp
		if next_try < 0 then
			next_try = "00:00:00"
			error = "0"
		elseif next_try > 1400000000 then
			next_try = "--:--:--"
		else
			next_try = os.date("!%H:%M:%S", next_try)
		end
		if status == "1" or status == "2" then
			if is_connected == "1" then
				connection_state = "<span style=\"color: #009900;font-weight: bold\">Connected</span>"
			else
				if error == "1" then
					connection_state = "<span style=\"color: #b30000;font-weight: bold\">Failure</span>"
					if error_text:len() > 0 then
						connection_state = connection_state .. " (" .. error_text .. ")"
					end
				else
					next_try = "-1"
					connection_state = "<span style=\"color: #ffa31a;font-weight: bold\">Connecting</span>"
				end
			end
		end
		local rv = {
			status = status,
			connection_state = connection_state,
			router_ip = router_ip,
			serial_nbr = serial_nbr,
			lan_mac = lan_mac,
			next_try = next_try,
			is_connected = is_connected,
		}
		luci.http.prepare_content("application/json")
		luci.http.write_json(rv)

		return
	end

	local system, model = luci.sys.sysinfo()

end

function get_gps_time()
	local utl = require "luci.util"
	local sys = require "luci.sys"
	local gps_time = utl.trim(luci.sys.exec("gpsctl -e"))
	local gps_time_seconds = utl.trim(luci.sys.exec("gpsctl -f"))
	local status = "Update failed"

	if gps_time ~= "" and gps_time ~= "0" then
		luci.sys.exec("date -u -s '" .. gps_time .. "'")
		status = "Success!"
	end

	local rv = {
		status = status
	}
	-- prevent session timeoutby updating mtime
	local set = tonumber(gps_time_seconds)
	nixio.fs.utimes(luci.sauth.sessionpath .. "/" .. luci.dispatcher.context.authsession, set, set)
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end

function diag_ping()
	diag_command("ping -c 5 -W 1 %q 2>&1")
end

function diag_traceroute()
	diag_command("traceroute -q 1 -w 1 -n %q 2>&1")
end

function diag_nslookup()
	diag_command("nslookup %q 2>&1 localhost > /tmp/nslook; lncnt=`cat /tmp/nslook | wc -l`; cat /tmp/nslook | tail -n `expr $lncnt - 2`")
end

function diag_command(cmd)
	local path = luci.dispatcher.context.requestpath
	local addr = path[#path]

	if addr and addr:match("^[a-zA-Z0-9%-%.:_]+$") then
		luci.http.prepare_content("text/plain")

		local util = io.popen(cmd % addr)
		if util then
			while true do
				local ln = util:read("*l")
				if not ln then break end
				luci.http.write(ln)
				luci.http.write("\n")
			end

			util:close()
		end

		return
	end

	luci.http.status(500, "Bad address")
end

function action_clock_status()
	local set = tonumber(luci.http.formvalue("set"))
	if set ~= nil and set > 0 then
		local date = os.date("*t", set)
		if date then
			-- prevent session timeoutby updating mtime
			nixio.fs.utimes(luci.sauth.sessionpath .. "/" .. luci.dispatcher.context.authsession, set, set)

			luci.sys.call("date -s '%04d-%02d-%02d %02d:%02d:%02d'" %{
				date.year, date.month, date.day, date.hour, date.min, date.sec
			})
		end
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json({ timestring = os.date("%Y-%m-%d %H:%M:%S") })
end

function uhttpd_crt_dl()
	local uhttpd_crt_cmd = "cat /etc/uhttpd.crt 2>/dev/null"
	local reader = ltn12_popen(uhttpd_crt_cmd)
	luci.http.header('Content-Disposition', 'attachment; filename="uhttpd.crt"')
	luci.http.prepare_content("application/x-x509-user-cert")
	luci.ltn12.pump.all(reader, luci.http.write)
	t = {requests = "insert", table = "EVENTS", type="Web UI", text="Certificate was downloaded!"}
	eventlog:insert(t)
end

function tcpdumppcap()
	local mount = luci.util.trim(luci.sys.exec("uci -q get system.system.tcpdump_last_save"))
	local restore_cmd = "tar -xzC/ >/dev/null 2>&1"
	local tcpdump_cmd  = "/etc/init.d/tcpdebug stop &>/dev/null; tar -zcf - "..mount.."/tcpdebug.pcap; /etc/init.d/tcpdebug start &>/dev/null;"
	local fp
	luci.http.setfilehandler(
		function(meta, chunk, eof)
			if not fp then
				fp = io.popen(restore_cmd, "w")
			end
			if chunk then
				fp:write(chunk)
			end
			if eof then
				fp:close()
			end
		end
	)
	local reader = ltn12_popen(tcpdump_cmd)
	luci.http.header('Content-Disposition', 'attachment; filename="tcpdebug-%s-%s.tar.gz"' % {
		luci.sys.hostname(), os.date("%Y-%m-%d")})
	luci.http.prepare_content("application/x-tar")
	luci.ltn12.pump.all(reader, luci.http.write)
	t = {requests = "insert", table = "EVENTS", type="Web UI", text="TCP dump .pcap file was downloaded!"}
	eventlog:insert(t)

end

function trdownload(val)
	local include_topology = ""
	local restore_cmd = "tar -xzC/ >/dev/null 2>&1"
	if val == "topology" then
		include_topology = "topology"
	end
	local trouble_backup_cmd  = "troubleshoot.sh " .. include_topology .. "; cat /tmp/troubleshoot.tar.gz  - 2>/dev/null"
	local image_tmp   = "/tmp/firmware.img"
	local fp
	luci.http.setfilehandler(
		function(meta, chunk, eof)
			if not fp then
				if meta and meta.name == "image" then
					fp = io.open(image_tmp, "w")
				else
					fp = io.popen(restore_cmd, "w")
				end
			end
			if chunk then
				fp:write(chunk)
			end
			if eof then
				fp:close()
			end
		end
	)
	-- Assemble file list, generate troubleshoot_backup
	--
	local reader = ltn12_popen(trouble_backup_cmd)
	luci.http.header('Content-Disposition', 'attachment; filename="trouble_backup-%s-%s.tar.gz"' % {
		luci.sys.hostname(), os.date("%Y-%m-%d")})
	luci.http.prepare_content("application/x-tar")
	luci.ltn12.pump.all(reader, luci.http.write)
	t = {requests = "insert", table = "EVENTS", type="Web UI", text="Trobleshoot was downloaded!"}
	eventlog:insert(t)
end

function action_backup()
	local sys = require "luci.sys"
	local fs  = require "luci.fs"
	local restore_cmd = "tar -xzC/tmp/new_config_dir >/dev/null 2>&1"
	local backup_cmd  = "sysupgrade --create-backup-with-packages - 2>/dev/null"
	local this_device_code
	local this_device_code_len
	local device_code_in_new_config
	local device_code_in_new_config_len
	local fp
	local write_new_config_ok = 0

	luci.sys.call("rm -rf /tmp/new_config_dir; mkdir /tmp/new_config_dir 2> /dev/null");

	luci.http.setfilehandler(
		function(meta, chunk, eof)
			if not fp then
				fp = io.popen(restore_cmd, "w")
			end
			if chunk then
				fp:write(chunk)
			end
			if eof then
				fp:close()
				write_new_config_ok = check_backup()
			end
		end
	)

	if luci.http.formvalue("backup") then
		--
		-- Assemble file list, generate backup
		--
		reader = ltn12_popen(backup_cmd)
		luci.http.header('Content-Disposition', 'attachment; filename="backup-%s-%s.tar.gz"' % {
			luci.sys.hostname(), os.date("%Y-%m-%d")})
		luci.http.prepare_content("application/x-targz")
		luci.ltn12.pump.all(reader, luci.http.write)
		t = {requests = "insert", table = "EVENTS", type="Web UI", text="Backup was downloaded!"}
		eventlog:insert(t)

	elseif luci.http.formvalue("restore") then
		local upload = luci.http.formvalue("archive")
		if upload and #upload > 0 then
			if write_new_config_ok == 1 then
				new_ip_addr = luci.sys.exec("cat /tmp/new_config_dir/etc/config/network | grep -A7 \"interface 'lan'\" | grep ipaddr | cut -d' ' -f3")
				luci.sys.exec("rm -f /tmp/new_config_dir/etc/config/hwinfo")
				luci.sys.exec("rm -f /tmp/new_config_dir/etc/inittab")
				luci.sys.exec("cp -rf /tmp/new_config_dir/etc /")
				luci.sys.exec("cp -rf /tmp/new_config_dir/lib/uci/upload /lib/uci")
				luci.sys.call("/sbin/crt_rms w >/dev/null 2>/dev/null")
				--[[
				local rms_status = tonumber(luci.sys.exec("uci get rms_connect_mqtt.rms_connect_mqtt.enable") or 0)
				local rms_email = luci.sys.exec("uci get rms_connect_mqtt.rms_connect_mqtt.email") or "";
				luci.sys.call("/sbin/crt_rms r >/dev/null 2>/dev/null")
				if rms_status == 0 then
                                	luci.sys.call("uci set rms_connect.rms_connect.enable='0'")
                                	luci.sys.call("uci set openvpn.teltonika_auth_service.enable='0'")
                                else
                                	luci.sys.call("uci set rms_connect.rms_connect.enable='1'")
                                end
				]]--
				luci.sys.call("cp /rom/etc/uci-defaults/* /etc/uci-defaults/ 2> /dev/null");
				luci.sys.call("rm /etc/uci-defaults/99_touch-firstboot") 
				luci.sys.call("/etc/init.d/simpin reload >/dev/null 2>/dev/null")
				luci.template.render("admin_system/applyreboot", {ipaddr = new_ip_addr})
				t = {requests = "insert", table = "EVENTS", type="Web UI", text="Configuration was restored from backup!"}
				eventlog:insert(t)
				luci.sys.reboot()
			else
				luci.sys.exec("rm -rf /tmp/new_config_dir/")
				local file_err_code = 1
				luci.template.render("admin_system/backup", {
					file_err_code = file_err_code
				})
			end
		end
	elseif luci.http.formvalue("redirect") then
		render_backup_and_throw_error(1)
	else
		--
		-- Overview
		--
		luci.template.render("admin_system/backup", {
		})
	end


end

function check_backup()
	local write_new_config_ok = 1

	this_device_code = luci.util.trim(luci.sys.exec("uci get -q hwinfo.hwinfo.mnf_code"))
	this_device_code_len = string.len(this_device_code)

	device_code_in_new_config = luci.sys.exec("cat /tmp/new_config_dir/etc/config/system | grep device_code | cut -d' ' -f3")
	device_code_in_new_config = device_code_in_new_config:gsub("%\n", "")
	device_code_in_new_config = device_code_in_new_config:gsub("%'", "")
	device_code_in_new_config_len = string.len(device_code_in_new_config)

	this_device_code = string.sub(this_device_code, 1, 7)
	device_code_in_new_config = string.sub(device_code_in_new_config, 1, 7)

    this_device_fw_version = luci.util.trim(luci.sys.exec("cat /etc/version"))
    this_device_fw_version_len = string.len(this_device_fw_version)
	this_device_fw_version = this_device_fw_version:reverse()
	this_device_fw_version = this_device_fw_version:sub(1, this_device_fw_version:find("_") - 1)
	this_device_fw_version = this_device_fw_version:reverse()

    fw_version_in_new_config = luci.sys.exec("cat /tmp/new_config_dir/etc/config/system | grep device_fw_version | cut -d' ' -f3")
    fw_version_in_new_config = fw_version_in_new_config:gsub("%\n", "")
    fw_version_in_new_config = fw_version_in_new_config:gsub("%'", "")
    fw_version_in_new_config_len = string.len(fw_version_in_new_config)
	fw_version_in_new_config = fw_version_in_new_config:reverse()
	fw_version_in_new_config = fw_version_in_new_config:sub(1, fw_version_in_new_config:find("_") - 1)
	fw_version_in_new_config = fw_version_in_new_config:reverse()

	if this_device_code_len ~= 12 or device_code_in_new_config_len ~= 12 or this_device_code ~= device_code_in_new_config then
		write_new_config_ok = 0
	end

    if fw_version_in_new_config_len < 12 or this_device_fw_version < fw_version_in_new_config or fw_version_in_new_config < "00.03.726" then
            write_new_config_ok = 0
    end

	return write_new_config_ok
end

function check_user_backup()
	local fs = require "nixio.fs"
	local date_file = "/etc/default-config/config_date"
	local value = fs.readfile(date_file)

	local rv = {
		backup_status = value and os.date("%m/%d/%Y %H:%M", value) or "-"
	}
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end

function handle_user_configuration()
	local fs = require "nixio.fs"
	local tar_file = "/etc/default-config/config.tar.gz"

	if luci.http.formvalue("_remove") then
		if fs.access(tar_file) then
			if fs.unlink(tar_file) then
				local res = 1
				luci.http.redirect(luci.dispatcher.build_url("admin/system/admin/backup") .. "?res="..res)
			end
		end
			luci.http.redirect(luci.dispatcher.build_url("admin/system/admin/backup"))
	elseif luci.http.formvalue("_create") then
		if not fs.access("/etc/default-config") then
			fs.mkdir("/etc/default-config")
		end

		fs.writefile("/etc/default-config/config_date", os.time())
		luci.sys.exec("/sbin/sysupgrade --user-config %s 2>/dev/null" % tar_file)

		if fs.access(tar_file) then
			local res = 2
			luci.http.redirect(luci.dispatcher.build_url("admin/system/admin/backup") .. "?res="..res)
		end
	end
end

function action_passwd()
	local p1 = luci.http.formvalue("pwd1")
	local p2 = luci.http.formvalue("pwd2")
	local stat = nil

	if p1 or p2 then
		if p1 == p2 then
			stat = luci.sys.user.setpasswd("root", p1)
		else
			stat = 10
		end
	end

	luci.template.render("admin_system/passwd", {stat=stat})
end

function action_reboot()
	local reboot = luci.http.formvalue("reboot")

	if reboot then
		luci.template.render("admin_system/rebootreload", {reboot=reboot})
		luci.sys.reboot()
	else
		luci.template.render("admin_system/reboot", {reboot=reboot})
	end
end

function fork_exec(command)
	local pid = nixio.fork()
	if pid > 0 then
		return
	elseif pid == 0 then
		-- change to root dir
		nixio.chdir("/")

		-- patch stdin, out, err to /dev/null
		local null = nixio.open("/dev/null", "w+")
		if null then
			nixio.dup(null, nixio.stderr)
			nixio.dup(null, nixio.stdout)
			nixio.dup(null, nixio.stdin)
			if null:fileno() > 2 then
				null:close()
			end
		end

		-- replace with target command
		nixio.exec("/bin/sh", "-c", command)
	end
end

function ltn12_popen(command)

	local fdi, fdo = nixio.pipe()
	local pid = nixio.fork()

	if pid > 0 then
		fdo:close()
		local close
		return function()
			local buffer = fdi:read(2048)
			local wpid, stat = nixio.waitpid(pid, "nohang")
			if not close and wpid and stat == "exited" then
				close = true
			end

			if buffer and #buffer > 0 then
				return buffer
			elseif close then
				fdi:close()
				return nil
			end
		end
	elseif pid == 0 then
		nixio.dup(fdo, nixio.stdout)
		fdi:close()
		fdo:close()
		nixio.exec("/bin/sh", "-c", command)
	end
end
