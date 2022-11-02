-- stream.lua
local utl = require "vuci.util"
local fs = require "nixio.fs"
local uci = require("vuci.uci").cursor()
local parser = require "tlt_parser_lua"
local md = require("vuci.modem")

local DEVICE_TYPE = 'Router'
local MODEM

function restart(r)
	local deviceId = r:value(2)
	c8y:send('108,'..deviceId..',SUCCESSFUL')
	utl.ubus("file", "exec", { command="/sbin/reboot"} )
end

function sysupgrade(r)
	local result
	local deviceId = r:value(2)
	local url = r:value(3)
	local username = uci:get("cot", "cumulocity", "username")
	local password = uci:get("cot", "cumulocity", "password")

	c8y:send('108,'..deviceId..',EXECUTING')

	result = os.execute("/usr/bin/curl -y 60 -u '"..username.."':'"..password.."' -o /tmp/cot_firmware.bin "..url) / 256
	if not result or result ~= 0 then
		srInfo('Failed to download firmware file')
		c8y:send('108,'..deviceId..',FAILED')
		fs.remove("/tmp/cot_firmware.bin")
		return
	end

	result = os.execute("/sbin/sysupgrade -T /tmp/cot_firmware.bin") / 256
	if not result or result ~= 0 then
		srInfo('Failed to verify firmware image file')
		c8y:send('108,'..deviceId..',FAILED')
		fs.remove("/tmp/cot_firmware.bin")
    else
		c8y:send('108,'..deviceId..',SUCCESSFUL')
		fork_exec("sleep 1; /etc/init.d/dropbear stop; /etc/init.d/uhttpd stop; sleep 1; /sbin/sysupgrade /tmp/cot_firmware.bin")
    end
end

function init()
	local DEVICE_NAME = uci:get("system", "system", "routername")
	local INTERVAL = uci:get("cot", "cumulocity", "interval")
	srInfo('*** Stream Init ***')

	-- set device name and type
	c8y:send('103,'..c8y.ID..','..DEVICE_NAME..','..DEVICE_TYPE)
	c8y:send('111,'..c8y.ID..','..INTERVAL)

	MODEM = uci:get("cot", "cumulocity", "modem") or ""
	if MODEM == "" then
		local modem_list = md:get_all_modems()
		if #modem_list > 0 then
			MODEM = modem_list[1].id
		end
	end

	-- set imei, cellid and iccid first time
	mobileDataStream()

	-- create timer which will stream mobile info data
	local m_timer = c8y:addTimer(10 * 1000, 'mobileDataStream')
	m_timer:start()

	-- register restart handler
	c8y:addMsgHandler(502, 'restart')
	c8y:addMsgHandler(503, 'sysupgrade')
	return 0
end

function mobileDataStream()
	if MODEM and #MODEM > 0 then
		local call = {modem = MODEM, signal="get_signal" }
		local results = parser:gsm(call)
		if results.signal ~= "N/A" then
			-- send mobile signal info
			c8y:send('105,'..c8y.ID..','.. string.gsub(results.signal, "%s", ""))
		else 
			c8y:send('105,'..c8y.ID..','.. string.gsub("0", "%s", ""))
		end
	else 
		c8y:send('105,'..c8y.ID..','.. string.gsub("0", "%s", ""))
	end
	local wantype = utl.ubus("file", "exec", { command="/sbin/wan_info", params={"state"} } )
	local wanip = utl.ubus("file", "exec", { command="/sbin/wan_info", params={"ip"} } )

	-- send wan info
	c8y:send('106,'..c8y.ID..','.. string.gsub(wanip.stdout, "%s", "") ..','.. string.gsub(wantype.stdout, "%s", ""))
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
