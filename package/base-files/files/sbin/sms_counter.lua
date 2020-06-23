#!/usr/bin/env lua

local sys = require "luci.sys"

local uci = require "uci".cursor()
local sqlite = require "lsqlite3"
local db -- database identifier

local dbPath = "/log/" -- place for running database
local dbName = "log.db" -- database file name
local dbFullPath = "/log/log.db" -- full path to database
local table_name = "SMS_COUNT"
local table_name_sms = "SMS_TABLE"

function failure()
	local out =
	[[ Wrong argument
		send	SLOT (SLOT1,SLOT2)
		recieve	SLOT (SLOT1,SLOT2)
		reset	SLOT (SLOT1,SLOT2,both)
		reset_sw SLOT(SLOT1,SLOT2)
		value	SLOT (SLOT1,SLOT2,both)
		value_sw SLOT (SLOT1,SLOT2)
	]]
	print(out)
	db:close()
	os.exit()
end

function check_table()
	local query = "select count(*) as number from ".. table_name
-- 	local stmt = db:rows(query)

	--~ Papildomas tikrinimas ar sukurtos reiksmes. Kitaip veliau negali ju updatint
	for row in db:nrows(query) do
		if row.number == 0 then
			local query = "insert into SMS_COUNT (SLOT,SEND,RECIEVED) values ('SLOT1',0,0);insert into SMS_COUNT (SLOT,SEND,RECIEVED) values ('SLOT2',0,0)"
			db:exec(query)
			break
		end
	end

	--~ Papildymas sms counterio del sim switcho
	query = "select * from " .. table_name_sms
	stmt = db:prepare(query)
	if stmt == nil then
		query = "create table ".. table_name_sms .. " (ID INTEGER PRIMARY KEY AUTOINCREMENT, SIM char(15), SEND INTEGER, TIME INTEGER)"
		db:exec(query)
	end
end

function get_values(slot)
	if slot == "SLOT0" then
		slot = "SLOT2"
	end
	if slot == "SLOT1" or slot == "SLOT2" or slot =="both" then
		local query = ""
		if slot == "SLOT1" or slot == "SLOT2" then
			query = "select * from " .. table_name .." where SLOT='".. slot .."'"
		else
			query = "select * from " .. table_name
		end
		local list = {}
		for row in db:nrows(query) do
			list[#list+1] = row
		end
		if slot == "SLOT1" or slot == "SLOT2" then
			print(list[1].SEND .. " " .. list[1].RECIEVED )
		else
			print(list[1].SEND .. " " .. list[1].RECIEVED .. "\n" .. list[2].SEND .. " " .. list[2].RECIEVED)
		end
	else
		failure()
	end
end

function get_sim_number(slot)
	local sim = "SIM1"
	slot = (slot == "SLOT0" and "SLOT2" or slot)
	if ( slot ~= "SLOT2" and slot ~= "SLOT1" ) then
		failure()
		return
	end
	return (slot == "SLOT2" and "SIM2" or "SIM1")
end

function is_leap_year(year)
	return year % 4 == 0 and (year % 100 ~= 0 or year % 400 == 0)
end

function get_days_in_month(month, year)
	return month == 2 and is_leap_year(year) and 29
		or ("\31\28\31\30\31\30\31\31\30\31\30\31"):byte(month)
end

function get_values_sw(slot)
	local sim, query, period_start
	sim = get_sim_number(slot) or "SIM1"
	local period = uci:get("sim_switch", "rules", "period_sms_" .. (sim:lower() or "sim1")) or "month"
	local ct = os.time()
	local date = os.date("%Y %m %d %H"):split(" ")
	local current_time = { year = date[1], month = date[2], day = date[3], hour = date[4] }
	if ( period == "month" ) then
		local start_day = uci:get("sim_switch", "rules", "sms_day_" .. (sim:lower() or "sim1")) or "1"
		if tonumber(start_day) > tonumber(current_time["day"]) then
			if ((tonumber(current_time["month"]) - 1) <= 0) then
				current_time["year"] = (tonumber(current_time["year"]) - 1)
				current_time["month"] = (tonumber(current_time["month"]) + 12)
			end
			period_start = os.time({ year = current_time["year"], month = (tonumber(current_time["month"]) - 1),
				day = start_day, hour = 0, min = 0, sec = 1 }) or current_time
		else
			period_start = os.time({ year = current_time["year"], month = current_time["month"],
				day = start_day, hour = 0, min = 0, sec = 1 }) or current_time
		end
	elseif ( period == "week" ) then
		local start_day = uci:get("sim_switch", "rules", "sms_weekday_" .. (sim:lower() or "sim1")) or "1"
		-- current date (day) - current weekday + offset (as user can set start of the week to be anything in config)
		local start_of_week = ( tonumber( current_time["day"] ) or 0) - ( tonumber(os.date("%w")) ) + (start_day or 0)
		start_of_week = start_of_week == 0 and 1 or start_of_week
		if start_of_week < 0 then
			period_start = os.time({ year = current_time["year"], month = (tonumber(current_time["month"]) - 1),
				day = get_days_in_month(tonumber(current_time["month"])), hour = 0, min = 0, sec = 1 }) or current_time
		elseif start_of_week > tonumber(current_time["day"]) then
			period_start = os.time({ year = current_time["year"], month = (tonumber(current_time["month"]) - 1),
				day = start_of_week, hour = 0, min = 0, sec = 1 }) or current_time
		else
			period_start = os.time({ year = current_time["year"], month = current_time["month"],
				day = start_of_week, hour = 0, min = 0, sec = 1 }) or current_time
		end
	elseif ( period == "day" ) then
		local startHour = uci:get("sim_switch", "rules", "sms_hour_" .. (sim:lower() or "sim1")) or "1"
		if tonumber(startHour) > tonumber(current_time["hour"]) then
			period_start = os.time({year = current_time["year"], month = current_time["month"],
				day = (tonumber(current_time["day"]) - 1), hour = startHour}) or current_time
		else
			period_start = os.time({year = current_time["year"], month = current_time["month"],
				day = current_time["day"], hour = startHour}) or current_time
		end
	end
	query = "SELECT * FROM " .. table_name_sms .. " WHERE SEND=1 and TIME>=" .. period_start .. " and TIME<=" .. ct .. " and SIM='" .. (sim or "SIM1") .. "'"
	local count = 0
	for row in db:nrows(query) do
		count = count + 1
	end
	print(count)
	return
end

function send(slot)
	if slot == "SLOT0" then
		slot = "SLOT2"
	end
	if slot == "SLOT1" or slot == "SLOT2" then
		local query = "select SEND from ".. table_name .." where SLOT='".. slot .."'"
		for row in db:nrows(query) do
				row.SEND = row.SEND + 1
				local query = "update " .. table_name .." set SEND=".. row.SEND .." where SLOT='".. slot .."'"
				db:exec(query)
		end
		--~ inserts sent sms from sim and time
		if slot == "SLOT1" then
			query = "INSERT INTO SMS_TABLE (SIM, SEND, TIME) VALUES ('SIM1', 1, "..os.time().."); "
		else
			query = "INSERT INTO SMS_TABLE (SIM, SEND, TIME) VALUES ('SIM2', 1, "..os.time().."); "
		end
		db:exec(query)
	else
		failure()
	end
end

function recieve(slot)
	if slot == "SLOT0" then
		slot = "SLOT2"
	end
	if slot == "SLOT1" or slot == "SLOT2" then
		local query = "select RECIEVED from SMS_COUNT where SLOT='".. slot .."'"
		for row in db:nrows(query) do
				row.RECIEVED = row.RECIEVED + 1
				local query = "update SMS_COUNT set RECIEVED=".. row.RECIEVED .." where SLOT='".. slot .."'"
				db:exec(query)
		end
		--~ inserts sent sms from sim and time
		if slot == "SLOT1" then
			query = "INSERT INTO SMS_TABLE (SIM, SEND, TIME) VALUES ('SIM1', 0, "..os.time().."); "
		else
			query = "INSERT INTO SMS_TABLE (SIM, SEND, TIME) VALUES ('SIM2', 0, "..os.time().."); "
		end
		db:exec(query)
	else
		failure()
	end

end

function reset(slot)
	if slot == "SLOT0" then
		slot = "SLOT2"
	end
	if slot == "SLOT1" or slot == "SLOT2" or slot =="both" then
		if slot == "SLOT1" or slot == "both" then
			local query = "update " .. table_name .." set SEND=0 where SLOT='".. slot .."'; update " .. table_name .." set RECIEVED=0 where SLOT='".. slot .."'"
			db:exec(query)
		end
		if slot == "SLOT2" or slot == "both" then
			local query = "update " .. table_name .." set SEND=0 where SLOT='".. slot .."'; update " .. table_name .." set RECIEVED=0 where SLOT='".. slot .."'"
			db:exec(query)
		end
	else
		failure()
	end
end

function reset_sw(slot)
	local sim, query
	sim = get_sim_number(slot)
	query = "update " .. table_name_sms .. " set SEND=0 where SIM='" .. sim .. "'"
	db:exec(query)
end

function start()
	if arg[1] and arg[2] then
		db = sqlite.open(dbFullPath)
		check_table()
		if arg[1] == "send" then
			send(arg[2])
		elseif arg[1] == "recieve" then
			recieve(arg[2])
		elseif arg[1] == "reset" then
			reset(arg[2])
		elseif arg[1] == "reset_sw" then
			reset_sw(arg[2])
		elseif arg[1] == "value" then
			get_values(arg[2])
		elseif arg[1] == "value_sw" then
			get_values_sw(arg[2])
		else
			failure()
		end
		db:close()
	else
		failure()
	end
end

start()
