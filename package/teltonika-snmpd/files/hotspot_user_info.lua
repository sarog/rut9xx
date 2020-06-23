require "teltonika_lua_functions"
require "luci.sys"
local sqlite = require "lsqlite3"
local dbPath = "/var/"
local dbName = "hotspot.db"
local dbFullPath = dbPath .. "" .. dbName
local result
local method = tonumber(arg[1])
local hotspot = tonumber(arg[2])
local to_snmp = ""
local wlan
local routerTime = tonumber(os.time())


if hotspot == 1 then
	wlan = "wlan0"
elseif hotspot == 2 then
	wlan = "wlan0-1"
elseif hotspot == 3 then
	wlan = "wlan0-2"
elseif hotspot == 4 then
	wlan = "wlan0-3"
end

if fileExists(dbPath, dbName) then
	local db
	db = sqlite.open(dbPath .. dbName)
	if db then
		local query = "SELECT * FROM statistics WHERE session = 1 AND ifname = '" .. wlan .. "'"
		result = selectDB(query, db)
		closeDB(db)
	end
end

function convert_bytes(bytes)
	local divisor = 1024
	local metric = {"B", "KB", "MB", "GB", "TB"}
	local num = 1
	local result

	while bytes > divisor do
		bytes = bytes / divisor
		num = num + 1
	end

	bytes = round((math.floor(bytes * (10^3) + 0.5) / (10^3)),2)

	if num < 6 and num > 0 then
		result = bytes .. " " .. metric[num]
	end

	return result or bytes
end

if result then
	if method == 0 then
		for name, row in ipairs(result) do
			if luci.util.trim(luci.sys.exec("uci get coovachilli.hotspot"..hotspot..".mode")) == "mac" then
				if row.user == "" then
					to_snmp = to_snmp .. "- "
				else
					to_snmp = to_snmp .. row.user .. " "
				end
			else
				to_snmp = to_snmp .. row.user .. " "
			end
		end
		to_snmp = to_snmp .. "\n"
		for name, row in ipairs(result) do
			to_snmp = to_snmp .. row.ip .. " "
		end
		to_snmp = to_snmp .. "\n"
		for name, row in ipairs(result) do
			to_snmp = to_snmp .. row.mac .. " "
		end
		to_snmp = to_snmp .. "\n"
		for name, row in ipairs(result) do
			to_snmp = to_snmp .. os.date("%F %T", row.time) .. "_"
		end
		to_snmp = to_snmp .. "\n"
		for name, row in ipairs(result) do
			to_snmp = to_snmp .. os.date("!%X", (routerTime - row.time)) .. " "
		end
		to_snmp = to_snmp .. "\n"
		for name, row in ipairs(result) do
			to_snmp = to_snmp .. convert_bytes(row.input) .. "_"
		end
		to_snmp = to_snmp .. "\n"
		for name, row in ipairs(result) do
			to_snmp = to_snmp .. convert_bytes(row.output) .. "_"
		end
		to_snmp = to_snmp .. "\n"
		for name, row in ipairs(result) do
			to_snmp = to_snmp .. os.date("%F %T", routerTime) .. "_"
		end
	elseif method == 1 then
		for name, row in ipairs(result) do
			if luci.util.trim(luci.sys.exec("uci get coovachilli.hotspot"..hotspot..".mode")) == "mac" then
				if row.user == "" then
					to_snmp = to_snmp .. "- "
				else
					to_snmp = to_snmp .. row.user .. " "
				end
			else
				to_snmp = to_snmp .. row.user .. " "
			end
		end
	elseif method == 2 then
		for name, row in ipairs(result) do
			to_snmp = to_snmp .. row.ip .. " "
		end
	elseif method == 3 then
		for name, row in ipairs(result) do
			to_snmp = to_snmp .. row.mac .. " "
		end
	elseif method == 4 then
		for name, row in ipairs(result) do
			to_snmp = to_snmp .. os.date("%F %T", row.time) .. "_"
		end
	elseif method == 5 then
		for name, row in ipairs(result) do
			to_snmp = to_snmp .. os.date("!%X", (routerTime - row.time)) .. " "
		end
	elseif method == 6 then
		for name, row in ipairs(result) do
			to_snmp = to_snmp .. convert_bytes(row.input) .. "_"
		end
	elseif method == 7 then
		for name, row in ipairs(result) do
			to_snmp = to_snmp .. convert_bytes(row.output) .. "_"
		end
	elseif method == 8 then
		for name, row in ipairs(result) do
			to_snmp = to_snmp .. os.date("%F %T", routerTime) .. "_"
		end
	end
end

print(to_snmp)
return to_snmp
