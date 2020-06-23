module("luci.controller.restore-point", package.seeall)

local uci = require "luci.model.uci".cursor()
local ds = require "luci.dispatcher"
local sys = require "luci.sys"
local utl = require "luci.util"
	eventlog = require'tlt_eventslog_lua'

function index()
	entry( {"admin", "system", "restorepoint"}, alias("admin", "system", "restorepoint", "create"), _("Restore Point"), 5)
	entry( {"admin", "system", "restorepoint", "create"}, template("restore-point/create-restore-point"), _("Create"), 1).leaf = true
	entry( {"admin", "system", "restorepoint", "load"}, template("restore-point/load-restore-point"), _("Load"), 2).leaf = true
	entry( {"admin", "system", "restorepoint", "apply"}, call("apply"))
end
function apply()
	local image_tmp  = "/tmp/restore_point.rp"
	local sys = require "luci.sys"
	local fs  = require "luci.fs"
	local restore_cmd = "tar -xzC/ >/dev/null 2>&1"
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

	local step = tonumber(luci.http.formvalue("step") or 1)
	if step == 1 then
		if luci.http.formvalue("load-restore-point-button") then
			filelink = luci.http.formvalue("cbid.system.restore.rp")
		elseif luci.http.formvalue("load-button") then
			filelink = image_tmp
		end
		checksum = luci.sys.exec("/sbin/restore-point -g " .. filelink .. " 2>/dev/null;")
        sn = luci.sys.exec("/sbin/restore-point -s " .. filelink .. " >/dev/null 2>/dev/null; echo $?")
		valid = luci.sys.exec("/sbin/restore-point -m " .. filelink .. " >/dev/null 2>/dev/null; echo $?")
		luci.template.render("restore-point/restore", {
				filelink = filelink,
				checksum	= checksum,
                sn          = sn,
				valid		= valid
			})
	elseif step == 2 then
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
		sIP = utl.trim(sys.exec("uci -q get network.lan.ipaddr"))
		wan = utl.trim(sys.exec(". /lib/teltonika-functions.sh; tlt_get_wan_ipaddr"))
		if wan == "" then
			wan = 0
		end

		filelink = luci.http.formvalue("filelink")
		luci.template.render("restore-point/applyrestore", {
				filelink = filelink,
				title = luci.i18n.translate("Upgrading..."),
				msg  = luci.i18n.translate("The system is upgrading now."),
				msg1  = luci.i18n.translate("<b>DO NOT POWER OFF THE DEVICE!</b>"),
				msg2  = luci.i18n.translate("It might be necessary to change your computer\'s network settings to reach the device again, depending on your configuration."),
				sProtocol = sProtocol,
				sIP = sIP,
				sPort = sPort,
				wan = wan
			})
	elseif step == 3 then
		t = {requests = "insert", table = "EVENTS", type="Restore point", text="FW was restored."}
		eventlog:insert(t)
		filelink = luci.http.formvalue("filelink")
		fork_exec("/etc/init.d/uhttpd stop; sleep 1; /sbin/restore-point -r "..filelink.." reboot; if [ $? -ne 0 ]; then /etc/init.d/dropbear start; /etc/init.d/uhttpd start; fi")
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
