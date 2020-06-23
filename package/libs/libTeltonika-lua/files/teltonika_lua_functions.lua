require("uci")

local sqlite = require "lsqlite3"
interfaces = {
	{ifname="3g-ppp", genName="Mobile", type="3G"},
	{ifname="eth2", genName="Mobile", type="3G"},
	{ifname="usb0", genName="WiMAX", type="WiMAX"},
	{ifname="eth1", genName="Wired", type="vlan"},
	{ifname="wlan0", genName="WiFi", type="wifi"},
	{ifname="none", genName="Mobile bridged", type="3G"},
	{ifname="wwan0", genName="Mobile", type="3G"},
	{ifname="wm0", genName="WiMAX", type="WiMAX"},
	{ifname="wwan-usb0", genName="Mobile USB", type="3G"},
}

function getParam(string)
	local h = io.popen(string)
	local t = h:read()
	h:close()
	return t
end

function readFile(file)
	local f = io.open(file, "rb")
	local content = f:read("*all")
	f:close()
	return content
end


function fileExists(path, name)
	local string = "ls ".. path
	local h = io.popen(string)
	local t = h:read("*all")
	h:close()

	for i in string.gmatch(t, "%S+") do
		if i == name then
			return 1
		end
	end
end

function sleep(n)
	os.execute("sleep " .. tonumber(n))
end

function round(num, idp)
	local mult = 10^(idp or 0)
	return math.floor(num * mult + 0.5) / mult
end

--Database functions

function initDB(dbPath, dbName)
	if fileExists(dbPath, dbName) then
		db = sqlite.open(dbPath .. dbName)
		return db
	end
	return -1
end

function selectDB(query, db)
	local list = {}
	local stmt = db:prepare(query)

	if stmt then
		for row in db:nrows(query) do
			list[#list+1] = row
		end
	else
		print("Error: could not execute select table in database.")
	end

	if #list > 0 then
		return list
	end
end

function closeDB(db)
	db:close()
end

function get_sim()
	local sim = getParam("/sbin/gpio.sh get SIM")
	return sim
end

function restoreDB(from, to)
	--print(string.format("[restore DB] from: %s, to: %s", from, to))
	os.execute(string.format("gunzip -c %s > %s", from, to))
end

function get_wan_section(typ, param)
	local uci = uci.cursor()
	local interfaces, name, value
	if typ == "type" then
		if param == "mobile" then
			interfaces = { "3g-ppp", "eth2", "wwan0" }
		elseif param == "wired" then
			interfaces = { "eth1" }
		elseif param == "wifi" then
			interfaces = { "wlan0" }
		else
			return
		end
	elseif typ == "ifname" then
		interfaces = { param }
	else
		return
	end

	uci:foreach("network", "interface", function(s)
		name = s[".name"]
		if name:match("wan") then
			for n, i in ipairs(interfaces) do
				if s.ifname == i then
					value = name
					return
				end
			end
		end
	end)
	if value then return value end
end

function wan_section_enabled(typ, param)
	local uci = uci.cursor()
	local section = get_wan_section(typ, param)
	local enabled = uci:get("network", section, "enabled") or "1"
	
	return enabled
end

function get_wan_option(typ, param, option)
	local uci = uci.cursor()
	local section = get_wan_section(typ, param)
	local result = uci:get("network", section, option)
	
	return result
end

function debug(name, string, ...)
	os.execute(string.format("/usr/bin/logger -t \"%s\" \"%s\"", name, string.format(string, ...)))
end

function isMobile(wan)
	if wan == nil then
		return false
	end
	local uci = uci.cursor()
	local mobile_IF = uci:get("network", "ppp", "ifname") or ""
	local IF = uci:get("network", wan, "ifname") or ""

	if IF == "" or mobile_IF == "" then
		return false
	end
	return IF == mobile_IF
end

function get_active_connection()
	local uci = uci.cursor()
	local activeConnection = "wan"
	local cachefile = luci.fs.readfile("/tmp/.mwan/cache")
	local enabled = uci:get("network", "ppp", "enabled") or ""
	local wan_name = uci:get("network", "wan", "ifname") or ""
	local proto = uci:get("network", "ppp", "proto") or ""
	if cachefile == nil then
		if wan_name == "wwan-usb0" then
			return "ppp_usb"
		elseif enabled ~= "0" then
			if proto == "3g" then
				return "ppp"
			elseif proto == "qmi" then
				return "ppp"
			elseif proto == "qmi2" then
				return "ppp"
			else
				return "wan"
			end
		else
			return "wan"
		end
	end
	local _, _, wan_if_map = string.find(cachefile, "wan_if_map=\"([^\"]*)\"")
	local _, _, wan_fail_map = string.find(cachefile, "wan_fail_map=\"([^\"]*)\"")
	local wans_map = {}
	local fail_wans_map = {}
	
	for wanname, wanifname in string.gfind(wan_fail_map, "([^%[]+)%[([^%]]+)%]") do
		table.insert(fail_wans_map, wanname)
	end
	
	for wanname, wanifname in string.gfind(wan_if_map, "([^%[]+)%[([^%]]+)%]") do
		local fail = false
		
		for n, failname in ipairs(fail_wans_map) do
			if wanname == failname then
				fail = true
			end
		end
		
		if not fail then
			activeConnection = wanname
			break
		end
	end
	
	if isMobile(activeConnection) then
		if enabled ~= "0" then
			if proto == "3g" then
				return "ppp"
			elseif proto == "qmi" then
				return "ppp_dhcp"
			elseif proto == "ncm" then
				return "ppp"
			elseif proto == "qmi2" then
				return "ppp"
			else
				return activeConnection
			end
		end
	end

	return activeConnection
end

function get_active_connection_2()
	local uci = uci.cursor()
	local activeConnection = "wan"
	local cachefile = luci.fs.readfile("/tmp/.mwan/cache")
	local enabled = uci:get("network", "ppp", "enabled") or ""
	local wan_name = uci:get("network", "wan", "ifname") or ""
	local proto = uci:get("network", "ppp", "proto") or ""
	if cachefile == nil then
		if wan_name == "wwan-usb0" then
			return "ppp_usb"
		elseif enabled ~= "0" then
			if proto == "3g" then
				return "ppp"
			elseif proto == "qmi" then
				return "ppp"
			elseif proto == "qmi2" then
				return "ppp"
			else
				return "wan"
			end
		else
			return "wan"
		end
	end
	local _, _, wan_if_map = string.find(cachefile, "wan_if_map=\"([^\"]*)\"")
	local _, _, wan_fail_map = string.find(cachefile, "wan_fail_map=\"([^\"]*)\"")
	local wans_map = {}
	local fail_wans_map = {}
	
	for wanname, wanifname in string.gfind(wan_fail_map, "([^%[]+)%[([^%]]+)%]") do
		table.insert(fail_wans_map, wanname)
	end
	
	for wanname, wanifname in string.gfind(wan_if_map, "([^%[]+)%[([^%]]+)%]") do
		local fail = false
		
		for n, failname in ipairs(fail_wans_map) do
			if wanname == failname then
				fail = true
			end
		end
		
		if not fail then
			activeConnection = wanname
			break
		end
	end
	
	if isMobile(activeConnection) then
		if enabled ~= "0" then
			if proto == "3g" then
				return "ppp"
			elseif proto == "qmi" then
				return "ppp_dhcp"
			elseif proto == "ncm" then
				return "ppp"
			elseif proto == "qmi2" then
				return "ppp_4"
			else
				return activeConnection
			end
		end
	end

	return activeConnection
end



function wan_information(param)
	local uci = uci.cursor()
	local bus = require "ubus"
	local nfs = require "nixio.fs"
	local has_backupLink = nfs.access("/tmp/.mwan/cache")
	local activeConnection
	local _ubus = bus.connect()
	local info = {}

	if has_backupLink then
		activeConnection = get_active_connection_2()

	else
		activeConnection = "wan"

		if isMobile(activeConnection) then
			local enabled = uci:get("network", "ppp", "enabled") or ""
			local proto = uci:get("network", "ppp", "proto") or ""
			if enabled ~= "0" then
				if proto == "3g" then
					activeConnection = "ppp"
				elseif proto == "qmi" then
					activeConnection = "ppp_dhcp"
				elseif proto == "qmi2" then
					activeConnection = "ppp_4"
				end
			end
		end
	end

	if activeConnection then
		info = _ubus:call(string.format("network.interface.%s", activeConnection), "status", { })
	end
	
	return info
end

function get_ifname(ssid)
    local bus = require "ubus"
    local _ubus = bus.connect()
    local info = _ubus:call("network.wireless", "status", { })
    local interfaces = info.radio0.interfaces

    for i, net in ipairs(interfaces) do
        if net.config.ssid == ssid  then
            return net.ifname
        end
    end

end 

-- Compares two firmwares, both of which to be specified in format "RUT9XX_R_12.34.456".
-- Return:
-- -2 - error
-- -1 - f1 > f2
--  0 - f1 == f2
--  1 - f1 < f2
function compareFirmwares(f1, f2)
	-- verx[1] - client number
	-- verx[2] - major version number
	-- verx[3] - minor version number
	-- verx[4] - hotfix version number

	-- return 0 -  firmware up to date
	-- return 1 -  firmware needs update
	-- return -1 - firmware newer than fota firmware
	-- return -2 - clients version number dont match
	-- return -3 - local or remote firmware version could not be found/wrong version name

	local ver1 = {}
	local ver2 = {}

	for x in string.gmatch(f1:gsub("RUT%d+",""), "%d+") do
		table.insert(ver1, tonumber(x))
	end

	for x in string.gmatch(f2:gsub("RUT%d+",""), "%d+") do
		table.insert(ver2, tonumber(x))
	end
	--os.execute("logger t GG "..ver1[3])
	--os.execute("logger t GG "..ver2[3])
	if #ver1 < 3 or #ver2 < 3 then
		return -3
	end

	if ver1[1] ~= ver2[1] then
		return -2
	end

	if ver1[2] < ver2[2] then
		return 1
	elseif ver1[2] == ver2[2] then
		if ver1[3] < ver2[3] then
			return 1
		elseif ver1[3] == ver2[3] then
			if not ver1[4] and not ver2[4] then
				return 0
			elseif ver1[4] and ver2[4] then
				if ver1[4] < ver2[4] then
					return 1
				elseif ver1[4] == ver2[4] then
					return 0
				else
					return -1
				end
			elseif not ver1[4] and ver2[4] then
				return 1
			else
				return -1
			end
		else
			return -1
		end
	else
		return -1
	end
end
--
-- Compares local firmware with remote firmware.
-- See compareFirmwares(f1, f2) for more info.
function compareLocalFwWithRemoteFw()
	if not fileExists("/tmp", "version_remote") or not fileExists("/etc", "version") then
		return -2
	end

	local locVer = readFile("/etc/version")
	local remVer = readFile("/tmp/version_remote")

	return compareFirmwares(locVer, remVer)
end

function logical_file_exists(name)
	local f=io.open(name,"r")
	if f~=nil then io.close(f) return true else return false end
end

function string_to_hex(str)
    return (str:gsub('.', function (c)
        return string.format('%02X', string.byte(c))
    end))
end

function hex_to_string(str)
    return (str:gsub('..', function (cc)
        return string.char(tonumber(cc, 16))
    end))
end

function escapeHTML(str)
	if str ~= nil then
		if type(str) == "userdata" then
			str = tostring(str)
		end
		str = (string.gsub(str, "&", "&amp;"))
		str = (string.gsub(str, "<", "&lt;"))
		str = (string.gsub(str, ">", "&gt;"))
		str = (string.gsub(str, "\"", "&quot;"))
		str = (string.gsub(str, "'", "&#039;"))
	end
	return str
end

function escape_shell_inner(str)
        return (string.gsub(str, "'", "'\\''"))
end

function escape_shell(str)
        return "'" .. escape_shell_inner(str) .. "'"
end
