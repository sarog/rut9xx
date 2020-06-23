#!/usr/bin/env lua
require "teltonika_lua_functions"
require("uci")

local db
local uci = uci.cursor()
local sqlite = require "lsqlite3"
local dbPath = "/var/"
local dbName = "hotspot.db"
local dbg_level = 0
local dmn_name = "statistics"
local pidPath = "/var/run/" -- prefix for PID filename
local pidName = dmn_name ..".pid" -- PID filename
local pidFullPath = pidPath .. pidName -- full path of PID filename
local dbFullPath = dbPath .. "" .. dbName
local savePath = "/etc/chilli/" -- place to store database files
local dbSavePath = savePath .. dbName ..".gz" -- full path to save compressed database files
local db_records_limit = 1000
local table = "statistics"

function debug(level, string)
	if string then
		if dbg_level >= level then
			os.execute("/usr/bin/logger -t Hotspot \"" .. string .. "\"")
		end
	end
end

function print_table( t )
    local print_r_cache={}
    local function sub_print_r(t,indent)
        if (print_r_cache[tostring(t)]) then
            print(indent.."*"..tostring(t))
        else
            print_r_cache[tostring(t)]=true
            if (type(t)=="table") then
                for pos,val in pairs(t) do
                    if (type(val)=="table") then
                        print(indent.."["..pos.."] => "..tostring(t).." {")
                        sub_print_r(val,indent..string.rep(" ",string.len(pos)+8))
                        print(indent..string.rep(" ",string.len(pos)+6).."}")
                    elseif (type(val)=="string") then
                        print(indent.."["..pos..'] => "'..val..'"')
                    else
                        print(indent.."["..pos.."] => "..tostring(val))
                    end
                end
            else
                print(indent..tostring(t))
            end
        end
    end
    if (type(t)=="table") then
        print(tostring(t).." {")
        sub_print_r(t,"  ")
        print("}")
    else
        sub_print_r(t,"  ")
    end
    print()
end

function open_DB()
	local db
	if fileExists(dbPath, dbName) then
		debug(2, "Opening database...")
		db = sqlite.open(dbPath .. dbName)
	else
		if fileExists(savePath, dbName .. ".gz") then
			debug(2, "Restoring database")
			restoreDB(dbSavePath, dbFullPath)
			db = sqlite.open(dbPath .. dbName)
		else
			debug(2, "Creating database")
			db = sqlite.open(dbPath .. dbName)
			db:exec("CREATE TABLE statistics (time TIMESTAMP, end_time TIMESTAMP,  ip VARCHAR(50), mac VARCHAR(50), user VARCHAR(50), duration INT, input INT, output INT, session INT, ifname VARCHAR(50))")
		end
	end

	return db
end

function count_rows(db, table)
	local query = string.format("SELECT Count(*) FROM %s", table)
	local result = selectDB(query, db)
	return result[1]["Count(*)"]
end

function delete_old_rows(db, table, num)
	local query = string.format("DELETE FROM %s WHERE ROWID IN (SELECT ROWID FROM %s ORDER BY ROWID ASC LIMIT %d)", table, table, num)
	db:exec(query)
end

function insertDB(data)
	local row_num = count_rows(db, "statistics")
	if row_num >= db_records_limit then
		local delete_num = row_num - db_records_limit + 1
		debug(2, "Deleting " .. delete_num .. " old records")
		delete_old_rows(db, table, delete_num)
	end
	local stmt = db:prepare("INSERT INTO statistics VALUES (:time, :end_time, :ip , :mac , :user , :duration , :input , :output , :session, :ifname)")

	if stmt then
		local time = os.time()
		stmt:bind_names{
			time = time,
			end_time = 0,
			ip = data.ip,
			mac = data.mac,
			user = data.user,
			duration = data.duration:match("%d+"),
			input = data.input:match("%d+"),
			output = data.output:match("%d+"),
			session = data.loged,
			ifname = data.ifname
		}
		stmt:step()
		stmt:finalize()
	else
		debug(1, "Could not insert into database.")
	end
end

function saveDB(from, to)
	if fileExists(dbPath, dbName) then
		os.execute(string.format("gzip -c %s > %s.gz", from, from))
		debug(2, "Saving database from " .. from .. ".gz to " .. to)
		os.execute(string.format("mv %s.gz %s", from, to))
	else
		print("No database")
	end
end

function save()
	local db = open_DB(dbPath, dbName)
	if db then
		local query = string.format("SELECT rowid FROM statistics WHERE session='1'")
		local result = selectDB(query, db)
		if result then
			for i, n in pairs(result) do
				query = string.format("UPDATE statistics SET session=:session, end_time=:end_time WHERE rowid='%s'", n.rowid)
				local stmt = db:prepare(query)

				if stmt then
					stmt:bind_names{
						session = 0,
						end_time = os.time()
					}
					stmt:step()
					stmt:finalize()
				else
					debug(1, "Could not update database.")
				end
			end
		end
		closeDB(db)
	end
	saveDB(dbFullPath, dbSavePath)
end

function get_env(vars, force)
	local enviroment = {
		user = "USER_NAME", mac = "CALLING_STATION_ID",
		ip = "FRAMED_IP_ADDRESS", duration = "SESSION_TIME",
		input = "INPUT_OCTETS_SESSION", output = "OUTPUT_OCTETS_SESSION",
		ifname = "DHCPIF"
	}
	local value
	local results = {loged = "1"}
	local exists = true
	local dev = os.getenv("DEV")

	if vars then
		enviroment = vars
	end
	--Gaunam DEV ir pagal tai nustatom koks interface. tun0 = wlan0, tun1 = wlan0-1 ...
-- 	if dev then
-- 		local num = dev:match("%d")
-- 		if num then
-- 			if tonumber(num) > 0 then
-- 				results["ifname"] = "wlan0-" .. num
-- 			else
-- 				results["ifname"] = "wlan0"
-- 			end
-- 		end
-- 	end

	--Gaunam enviroment kintamuosius ir sudedam i masyva.
	for n, i in pairs(enviroment) do
		if i then
			value = os.getenv(i)

			if value then
				results[n] = value
				debug(3, string.format("%s %s", n, value))
			else
				debug(3, string.format("Cant get %s value.", n))
				exists = false
			end
		end
	end

	if results.input and tonumber(results.input) < 0 then
		results.input = tonumber(results.input) * -1
	end

	if results.output and tonumber(results.output) < 0 then
		results.output = tonumber(results.output) * -1
	end

	--Jei visi reikalingi env kintamieji gauti, grazinam masyva.
	if exists or force then
		return results
	end

end

-- Funkcija ivygdoma kai prie hotspot prisilogina vartotojas
function conup()
	debug(3, "Connection up")
	local results = get_env()

	--Jei gauti nev kintamieji i duomenu baze pridedame vartotoja
	if results then
		debug(1, "User " ..results.user .. " logged in")
		db = open_DB(dbPath, dbName)
		if db then
			debug(3, "Database opened")
			insertDB(results)
			closeDB(db)
		end
	end
end

--Funkcija suvygdoma kai hotspot vartotojas isloginamas
function condown()
	local results = get_env()
	local db = open_DB(dbPath, dbName)

	if results and db then
		local query = string.format("SELECT rowid FROM statistics WHERE session='1' AND ip='%s' AND mac='%s' AND ifname='%s'", results.ip, results.mac, results.ifname)
		local result = selectDB(query, db)

		if result then
			for i, n in pairs(result) do
				query = string.format("UPDATE statistics SET session=:session, end_time=:end_time, duration=:duration, input=:input, output=:output WHERE rowid='%s'", n.rowid)
				local stmt = db:prepare(query)

				if stmt then
					stmt:bind_names{
						session = 0,
						end_time = os.time(),
						duration = results.duration,
						input = results.input,
						output = results.output
					}
					stmt:step()
					stmt:finalize()
				else
					debug(1, "Could not update database.")
				end
			end
		end
	else
		debug(2, "Database update stoped.")
	end

	closeDB(db)
end

function update()
	debug(3, "Updating")
	local enviroment = {
		user = "USER_NAME", mac = "CALLING_STATION_ID",
		ip = "FRAMED_IP_ADDRESS", duration = "SESSION_TIME",
		input = "INPUT_OCTETS_SESSION", output = "OUTPUT_OCTETS_SESSION",
		ifname = "DHCPIF", localusers = "LOCALUSERS", start = "START_POINT",
		period = "DATA_INTERVAL", max_input = "CHILLISPOT_MAX_INPUT_OCTETS",
		max_output = "CHILLISPOT_MAX_OUTPUT_OCTETS"
	}
	local results = get_env(enviroment, true)
	local db = open_DB(dbPath, dbName)
	
	if results and db then
		query = string.format("UPDATE statistics SET  end_time=:end_time, duration=:duration, input=:input," ..
				" output=:output WHERE mac='%s' AND session=1", results.mac)
		debug(1, query)
		local stmt = db:prepare(query)

		if stmt then
			stmt:bind_names{
				end_time = os.time(),
				duration = results.duration,
				input = results.input,
				output = results.output
			}
			stmt:step()
			stmt:finalize()
		else
			debug(1, "Could not update database.")
		end

		closeDB(db)

		if results.localusers then
			debug(1, string.format("Local users check"))
			local max_input = results.max_input and tonumber(results.max_input) or 0
			local max_output = results.max_output and tonumber(results.max_output) or 0
			local user_data = getData(results.user, results.ifname, results.period, results.start)

			if (user_data[1].download and max_input > 0 and tonumber(user_data[1].download) >= max_input) or
					(user_data[1].upload and max_output > 0 and tonumber(user_data[1].upload) >= max_output) then
				debug(1,  string.format("Loging out user %s, mac %s. Reason limit reached.", results.user, results.mac))
				logout(results.mac, results.ifname)
			end

		end

		if not device_associated(results.mac, result.DHCPIF) then
			debug(1, string.format("Device %s is disassociated.", results.mac))
			logout(results.mac, results.ifname)
		end
	end
end

function device_associated(mac, ifname)
	local t
	local h
	
	if mac and ifname then
		mac = mac:gsub("-", ":")
		h = io.popen(string.format("iwinfo %s assoclist | grep %s", ifname, mac))
		t = h:read()
		
		h:close()
	end
	
	return t
end

function logout(mac_addr, ifname)
	local socket = string.format("/var/run/chilli.%s.sock", ifname)
	
	if mac_addr and socket then
		local command = string.format("/usr/sbin/chilli_query -s %s logout %s", socket, mac_addr)
		local res = io.popen(command)
		if res and res:read("*all") == "" then
			debug(1, string.format("Device %s logged out from coovachilli.", mac_addr))
			return  1
		end
	end
	
	return 0
end

--Funkcija tikrinanti ar vartotojas dar nevirsijo limito
function check_user(user, ifname, period, start)
	local result = getData(user, ifname, period, start)
	local output = ""
	output = string.format("%s:%s:", result[1].download or 0, result[1].upload or 0)
	print(output)
end

--Funkcija tikrinanti ar irenginys dar nevirsijo limito
function check_mac(mac, ifname, period, start)
	local result = getData(mac, ifname, period, start, "mac")
	local output = ""
	output = string.format("%s:%s:", result[1].download or 0, result[1].upload or 0)
	print(output)
end

--Funkcija tikrinanti ar irenginys dar nevirsijo limito
function check_mac_all(mac, ifname, period, start)
	local result = getData(mac, ifname, period, start, "mac")
	local output = ""
	output = string.format("%s:%s:%s:", result[1].download or 0, result[1].upload or 0, result[1].duration or 0)
	print(output)
end

function getData(user, ifname, period, start_point, mode)
	local db = open_DB()
	if db then
		local seconds
		local timestamp = os.time()
		local year, month, weekday, day, hour = os.date("%Y", timestamp), os.date("%m", timestamp), os.date("%w", timestamp), os.date("%d", timestamp), tonumber(os.date("%H", timestamp))
		local result
		year = tonumber(year)
		month = tonumber(month)
		weekday = tonumber(weekday)
		day = tonumber(day)
		hour = tonumber(hour)
		debug(4, string.format("year: %s, month %s, weekday %s, day %s, hour %s", year, month, weekday, day, hour))
		start_point = tonumber(start_point)
	
		if period == "3" then
			if start_point > day then
				month = month - 1
			end
			
			day = start_point
			hour = 0
		elseif period == "1" then
			if start_point > hour then
				day = day - 1
			end
			
			hour = start_point
		elseif period == "3" then
			if weekday ~= start_point then
				if start_point > weekday then
					day = day - (7 - start_point + weekday)
				else
					day = day - (weekday - start_point)
				end
			end
			hour = 0
		end
		
		debug(4, string.format("year: %s, month %s, weekday %s, day %s, hour %s", year, month, weekday, day, hour))
	
		local start = tonumber(os.time{year=year, month=month, day=day, hour=hour})
		debug(3, "Start time: (" ..os.date("%c", start).."), (" ..start.. ")")
		local query
		if mode and mode == "mac" then
			query = string.format("SELECT SUM(input) AS download, SUM(output) AS upload, SUM(duration) AS duration FROM statistics WHERE time>='%d' AND mac='%s' AND ifname='%s'", start, user, ifname)
		else
			query = string.format("SELECT SUM(input) AS download, SUM(output) AS upload, SUM(duration) AS duration  FROM statistics WHERE time>='%d' AND user='%s' AND ifname='%s'", start, user, ifname)
		end

		debug(4, query)

		result = selectDB(query, db)
		closeDB(db)
		return result
	end
end


function help()
	local out =
[[------------------------------------------------
---------statistics-----------------------------
------------------------------------------------

	unknown command line argument.

usage:
	statistics conup
	statistics condown
	statistics check [username] [wifi interface] [interval] [start_point]
	statistics check_mac [mac] [wifi interface] [interval] [start_point]
]]
	print(out)
end

if arg and #arg > 0 and #arg <= 5  then
	if arg[1] == "save" then save()
	elseif arg[1] == "conup" then conup()
	elseif arg[1] == "condown" then condown()
	elseif arg[1] == "update" then update()
	elseif arg[1] == "check" and arg[2] and arg[3] then check_user(arg[2], arg[3], arg[4], arg[5])
	elseif arg[1] == "check_mac" and arg[2] and arg[3] then check_mac(arg[2], arg[3], arg[4], arg[5])
	elseif arg[1] == "check_mac_all" and arg[2] and arg[3] then check_mac_all(arg[2], arg[3], arg[4], arg[5])
	elseif arg[1] == "assoc" and arg[2] and arg[3] then if device_associated(arg[2], arg[3]) then print("associated") else print("disassociated") end
	else help() end
else
	help()
end
