
module("luci.controller.admin.fw_upgrade", package.seeall)

function index()

	entry({"admin", "system", "flashops"}, alias("admin", "system", "flashops","upgrade"), _("Firmware"), 5)
	entry({"admin", "system", "flashops","upgrade"}, call("action_flashops"), _("Firmware"), 1).leaf = true
	entry({"admin", "system", "flashops","config"}, cbi("auto_update"), _("FOTA"), 2).leaf = true
	entry({"admin", "system", "flashops","auto"}, call("download_fw"), nil, nil)
	entry({"admin", "system", "flashops","check"}, call("check_for_update"), nil, nil)
	entry({"admin", "system", "flashops","download"}, call("start_download"), nil, nil)
	entry({"admin", "system", "flashops","check_status"}, call("check_status"), nil, nil)

end

function is_connection_available()
	local download_error = luci.sys.call("/sbin/rut_fota -i")
	if not (tonumber(download_error) == 0) then
		return tonumber(download_error)
	end
	return 0
end

function render_flashops_and_throw_error(redirect)
	local download_error	= is_connection_available()
	local upgrade_avail 	= nixio.fs.access("/lib/upgrade/platform.sh")
	local error_message 	= "Failed to download firmware."

	if download_error == 0 then
		return
	end

	if download_error == 255 and (redirect == 1) then
		error_message = error_message .. " Connection dropped."
	end

	if download_error == 255 and (redirect == 0) then
		error_message = error_message .. " Failed to establish connection."
	end

	if download_error == 9 then
		error_message = error_message .. " Failed to locate file."
	end

	if download_error == 15 then
		error_message = error_message .. " MIM attack detected."
	end

	luci.template.render("admin_system/flashops", {
		upgrade_avail 	= upgrade_avail,
		download_error 	= 1, -- Download error set to true for WebUI
		error_message 	= error_message .. " (" .. "Error: " .. download_error .. ")"
	})
	return
end

function start_download()
	a=luci.http.formvalue("status") or ""
	luci.sys.call("rm -f /tmp/firmware.img")
	luci.sys.call("/sbin/rut_fota -f &")
end

function check_status()
	local fw = tonumber(luci.sys.exec("uci -q get rut_fota.config.file_to_download") or 0)
	local number = tonumber(luci.sys.exec("ls -al /tmp/firmware.img | awk -F ' ' '{print $5}'") or 0)
	local size = "0 %"
	if fw~= nil and number~=nil then
		if tonumber(fw) > 0 and tonumber(number) > 0 then
			number = tonumber(number)*100/tonumber(fw)
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

function check_for_update()
	local auto_update_err_code 	= luci.sys.call("/sbin/rut_fota -i")
	local remote_fw_version 	= luci.sys.exec("uci get -q rut_fota.config.fw_info") or ""

	local err_code = 0
	if tonumber(auto_update_err_code) ~= 0 then
		err_code = 1
		remote_fw_version = "No access to server."
	else
		if remote_fw_version == "" or remote_fw_version:find("N/A") then
			err_code = 2
			remote_fw_version = "No update available."
		end
	end

	local rv = {
		remote_fw_version = remote_fw_version,
		err_code = err_code
	}

	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
	return
end

function download_fw()

	local redirect = 0
	if luci.http.formvalue("redirect") then
		redirect = 1
	end

	if (redirect == 1) then
		render_flashops_and_throw_error(redirect)
		return
	end

	if luci.http.formvalue("step") then
		local step = tonumber(luci.http.formvalue("step") or 1)
		if step == 1 then
			luci.template.render("admin_system/download", {
				download 		= 1,
				keep			= (not not luci.http.formvalue("keep")),
				keep_network	= (not not luci.http.formvalue("keep_network")),
				keep_3g			= (not not luci.http.formvalue("keep_3g")),
				keep_lan		= (not not luci.http.formvalue("keep_lan")),
				keep_ddns		= (not not luci.http.formvalue("keep_ddns")),
				keep_wireless	= (not not luci.http.formvalue("keep_wireless")),
				keep_firewall	= (not not luci.http.formvalue("keep_firewall")),
				keep_openvpn	= (not not luci.http.formvalue("keep_openvpn"))
			})
		else
			local upgrade_avail = nixio.fs.access("/lib/upgrade/platform.sh")
			luci.template.render("admin_system/flashops", {
				download 		= 1,
				upgrade_avail 	= upgrade_avail
			})
		end
	else
		local upgrade_avail = nixio.fs.access("/lib/upgrade/platform.sh")
		luci.template.render("admin_system/flashops", {
			download 		= 1,
			upgrade_avail 	= upgrade_avail
		})
	end
end

--FIXME: action_flashops: lines - 400+, quality - not found
function action_flashops()
	local sys = require "luci.sys"
	local fs  = require "luci.fs"
	local eventlog = require'tlt_eventslog_lua'

	local upgrade_avail 		= nixio.fs.access("/lib/upgrade/platform.sh")
	local reset_avail   		= os.execute([[grep '"rootfs_data"' /proc/mtd >/dev/null 2>&1]]) == 0
	local restore_cmd 			= "tar -xzC/ >/dev/null 2>&1"
	local backup_cmd  			= "sysupgrade --create-backup - 2>/dev/null"
	local trouble_backup_cmd  	= "troubleshoot.sh; cat /tmp/troubleshoot.tar.gz  - 2>/dev/null"
	local image_tmp   			= "/tmp/firmware.img"

	-- After finishing flashing/erasing procedure 'reboot -f' command will be
	-- applied. So Telit modem won't boot up properly. Here we will do force modem shutdown.
	local function shutdown_telit()
		x = uci.cursor()
		--pid = x:get("system", "module", "pid") or ""
		--vid = x:get("system", "module", "vid") or ""
		--if pid == "0021" and vid == "1BC7" then
			luci.sys.call("/etc/init.d/modem stop")
		--end
	end

	local function fw_validate()
		return (luci.sys.exec("/sbin/fw_validate.sh %q 2> /dev/null" % image_tmp))
	end

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

	if luci.http.formvalue("backup") then
		--
		-- Assemble file list, generate backup
		--
		local reader = ltn12_popen(backup_cmd)
		luci.http.header('Content-Disposition', 'attachment; filename="backup-%s-%s.tar.gz"' % {
			luci.sys.hostname(), os.date("%Y-%m-%d")})
		luci.http.prepare_content("application/x-targz")
		luci.ltn12.pump.all(reader, luci.http.write)
	elseif luci.http.formvalue("trouble_backup") then
		--
		-- Assemble file list, generate troubleshoot_backup
		--
		local reader = ltn12_popen(trouble_backup_cmd)
		luci.http.header('Content-Disposition', 'attachment; filename="trouble_backup-%s-%s.tar.gz"' % {
			luci.sys.hostname(), os.date("%Y-%m-%d")})
		luci.http.prepare_content("application/x-tar")
		luci.ltn12.pump.all(reader, luci.http.write)

	elseif luci.http.formvalue("restore") then
		--
		-- Unpack received .tar.gz
		--
		local upload = luci.http.formvalue("archive")
		if upload and #upload > 0 then
			luci.template.render("admin_system/applyreboot")
			luci.sys.reboot()
		end
	elseif luci.http.formvalue("image") or luci.http.formvalue("step") then
		--
		-- Initiate firmware flash
		--
		t = {}
		validate_result = fw_validate()
		for k, v in string.gmatch(validate_result, "([%w%p]+)=([%w%p]+)") do
			t[k] = v
		end

		IS_FW_VERSION_NEW = tonumber(t["FW_IS_NEW"])
		local step = tonumber(luci.http.formvalue("step") or 1)
		if step == 1 then
			fork_exec("if grep -q /overlay /etc/config/fstab ; then sed -i 's/var iDuration = 160/var iDuration = 400/' /usr/lib/lua/luci/view/admin_system/applyflashing.htm; fi")
			if t["IMAGE_SUPPORTED"] == "1" then
				if(IS_FW_VERSION_NEW == 1) then
					local download = tonumber(luci.http.formvalue("download") or 0)
					luci.template.render("admin_system/upgrade", {
						checksum	= t["IMAGE_CHECKSUM"],
						storage		= tonumber(t["STORAGE_SIZE"]),
						download	= download,
						size		= tonumber(t["SIZE"]),
						keep		= (not not luci.http.formvalue("keep")), --keep all jei saugom viska, palikt tuscia jei ne
						keep_network	= (not not luci.http.formvalue("keep_network")),
						keep_3g		= (not not luci.http.formvalue("keep_3g")),
						keep_lan	= (not not luci.http.formvalue("keep_lan")),
						keep_ddns	= (not not luci.http.formvalue("keep_ddns")),
						keep_wireless	= (not not luci.http.formvalue("keep_wireless")),
						keep_firewall	= (not not luci.http.formvalue("keep_firewall")),
						keep_openvpn	= (not not luci.http.formvalue("keep_openvpn")),
                                                is_fw_version_new = IS_FW_VERSION_NEW
					})
				else
					luci.template.render("admin_system/upgrade", {
						checksum	= t["IMAGE_CHECKSUM"],
						storage		= tonumber(t["STORAGE_SIZE"]),
						download	= download,
						size		= tonumber(t["SIZE"]),
						keep		= "none", --keep all jei saugom viska, palikt tuscia jei ne
						keep_network	= (not not luci.http.formvalue("keep_network")),
						keep_3g		= (not not luci.http.formvalue("keep_3g")),
						keep_lan	= (not not luci.http.formvalue("keep_lan")),
						keep_ddns	= (not not luci.http.formvalue("keep_ddns")),
						keep_wireless	= (not not luci.http.formvalue("keep_wireless")),
						keep_firewall	= (not not luci.http.formvalue("keep_firewall")),
						keep_openvpn	= (not not luci.http.formvalue("keep_openvpn")),
                                                is_fw_version_new = IS_FW_VERSION_NEW
					})
				end
			else
				nixio.fs.unlink(image_tmp)
				luci.template.render("admin_system/flashops", {
					reset_avail   = reset_avail,
					upgrade_avail = upgrade_avail,
					image_invalid = true
				})
			end
		--
		-- Start sysupgrade flash
		--
		elseif step == 2 then
			if(IS_FW_VERSION_NEW == 1) then
				keep = luci.http.formvalue("keep")
				if(keep~="1") then
					keep = "0"
				end
			else
				keep = "0"
			end
			download	= luci.http.formvalue("download")
			keep_network	= luci.http.formvalue("keep_network")
			keep_3g		= luci.http.formvalue("keep_3g")
			keep_lan	= luci.http.formvalue("keep_lan")
			keep_ddns	= luci.http.formvalue("keep_ddns")
			keep_wireless	= luci.http.formvalue("keep_wireless")
			keep_firewall	= luci.http.formvalue("keep_firewall")
			keep_openvpn	= luci.http.formvalue("keep_openvpn")

			local redirect = ""
			--If true redirect to configurated IP address
			if keep_network ~= "" or keep_lan ~= "" then
				redirect = "1"
			end

			if redirect == "" then
				local sProtocol = "http"
				local sIP = "192.168.1.1"
				local sPort = "80"

				local sFile, sBuf, sLine, sPortHttp, sPortHttps
				local fs = require "nixio.fs"

				sFile = "/rom/lib/functions/uci-defaults.sh"
				if fs.access(sFile) then
					for sLine in io.lines(sFile) do
						if sLine:find("set network.lan.ipaddr='") and sLine:find("#") ~= 1 then
							sBuf = sLine:match("(%d+.%d+.%d+.%d+)")
							if sBuf ~= nil then
								sIP = sBuf
								break

							end
						end
					end
				end

				sFile = "/rom/etc/config/uhttpd"
				if fs.access(sFile) then
					for sLine in io.lines(sFile) do
						if sLine:find("list listen_http\t") and sLine:find("#") ~= 1 then
							sBuf = string.match(sLine:match("(:%d+)"), "(%d+)")
							if sBuf ~= nil then
								sPortHttp = sBuf
							end
						end

						if sLine:find("list listen_https\t") and sLine:find("#") ~= 1 then
							sBuf = string.match(sLine:match("(:%d+)"), "(%d+)")
							if sBuf ~= nil then
								sPortHttps = sBuf
							end
						end
					end

					if sPortHttp ~= nil and sPortHttp ~= "0" then
						sPort = sPortHttp
						sProtocol = "http"
						elseif sPortHttps ~= nil and sPortHttps ~= "0" then
							sPort = sPortHttps
							sProtocol = "https"
					end
				end

				sBufAddr = "\"" .. sProtocol .. "://" .. sIP .. ":" .. sPort .. "\""
			end
			luci.template.render("admin_system/applyflashing", {
				title = luci.i18n.translate("Upgrading..."),
				msg  = luci.i18n.translate("The system is upgrading now."),
				msg1  = luci.i18n.translate("<b>DO NOT POWER OFF THE DEVICE!</b>"),
				msg2  = luci.i18n.translate("It might be necessary to change your computer\'s network settings to reach the device again, depending on your configuration."),
				addr = sBufAddr,
				keep_s = keep,
				keep_n = keep_network,
				keep_3 = keep_3g,
				keep_l = keep_lan,
				keep_d = keep_ddns,
				keep_w = keep_wireless,
				keep_f = keep_firewall,
				keep_o = keep_openvpn,
				download = download
			})
		elseif step == 3 then
			local keep              = (luci.http.formvalue("keep_s") == "1") and "" or "-n"
			local keep_network	= (luci.http.formvalue("keep_n") == "1") and "network dhcp simcard teltonika uhttpd multiwan" or ""
			local keep_3g		= (luci.http.formvalue("keep_3") == "1") and "sim1 sim2 ppp" or ""
			local keep_lan		= (luci.http.formvalue("keep_l") == "1") and "lan" or ""
			local keep_ddns		= (luci.http.formvalue("keep_d") == "1") and "ddns" or ""
			local keep_wireless	= (luci.http.formvalue("keep_w") == "1") and "wireless" or ""
			local keep_firewall	= (luci.http.formvalue("keep_f") == "1") and "firewall" or ""
			local keep_openvpn	= (luci.http.formvalue("keep_o") == "1") and "openvpn" or ""
			local download		= (luci.http.formvalue("download") == "1") and "download" or ""
			check_pre_post=""
			if tonumber(download) == 1 then
				t = {requests = "insert", table = "EVENTS", type="FW", text="Upgrade from server"}
				eventlog:insert(t)
				check_pre_post="-k"
			elseif tonumber(download) == 0 then
				t = {requests = "insert", table = "EVENTS", type="FW", text="Upgrade from file"}
				eventlog:insert(t)
			end
			local dateold = luci.util.trim(luci.sys.exec("date +%s"))
			local datenew = dateold + 20 --per kiek laiko perraso fw tiek reikia pridet
			datenew = tostring(datenew)
			luci.sys.exec("date +%s -s @".. datenew)
			luci.sys.call("touch /etc/init.d/luci_fixtime")
				--workaround reboot irasymas cia ne sys upgrade, nes ten neissisaugo patachintas failas
				t = {requests = "insert", table = "EVENTS", type="Reboot", text="Request after FW upgrade"}
				eventlog:insert(t)
				local dateold = luci.util.trim(luci.sys.exec("date +%s"))
				local datenew = dateold + 105 --per kiek laiko perraso fw tiek reikia pridet
				datenew = tostring(datenew)
				luci.sys.exec("date +%s -s @".. datenew)
			luci.sys.call("touch /etc/init.d/luci_fixtime")
			luci.sys.call("uci set system.device_info.reboot=1")
			luci.sys.call("uci commit system")

			--useful for debugging folowing conditions
			--luci.sys.call("/usr/bin/eventslog insert \"Reboot\" \". %s . %s . %s . %s . %s . %s .\"" %{ luci.http.formvalue("keep_s"), keep, tostring((luci.http.formvalue("keep_s") == "1")) , keep_3g, keep_lan, keep_network })

-- 			if download == "download" then
-- 				download = "-k"
-- 			end

			if (IS_FW_VERSION_NEW == 1) then
				luci.sys.call("echo kazkas > /etc/uci-defaults/99_touch-firstboot")
			else
				keep = "-n"
			end
			local cmd1 = "killall dropbear uhttpd;"
			local cmd2 = "sleep 1; "
			local cmd3 = "/sbin/sysupgrade %s %s %q" % { keep, check_pre_post, image_tmp }
			fork_exec(cmd1 .. cmd2 .. cmd3)
		end
	elseif reset_avail and luci.http.formvalue("reset") then
		--
		-- Reset system
		--
		luci.template.render("admin_system/applyreboot", {
			title = luci.i18n.translate("Erasing..."),
			msg   = luci.i18n.translate("The system is erasing the configuration partition now and will reboot itself when finished."),

		})
		--shutdown_telit()
		--fork_exec("killall dropbear uhttpd; sleep 1; mtd -r erase rootfs_data")
		fork_exec("killall dropbear; sleep 1; echo y | firstboot; reboot -o")

	elseif reset_avail and luci.http.formvalue("user_reset") then 

		--
		-- Reset system to user defaults
		--
		luci.template.render("admin_system/applyreboot", {
			title = luci.i18n.translate("Erasing..."),
			msg   = luci.i18n.translate("The system is erasing the configuration partition now and will reboot itself when finished."),
		})
		luci.sys.exec("/sbin/user_defaults; reboot")
	else
		--
		-- Overview
		--
		luci.template.render("admin_system/flashops", {
			reset_avail   = reset_avail,
			upgrade_avail = upgrade_avail
		})
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

function mysplit(inputstr, sep)
        if sep == nil then
                sep = "%s"
        end
        local t={} ; i=1
        for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
                t[i] = str
                i = i + 1
        end
        return t
end

function is_int(n)
	return (type(n) == "number") and (math.floor(n) == n)
end
