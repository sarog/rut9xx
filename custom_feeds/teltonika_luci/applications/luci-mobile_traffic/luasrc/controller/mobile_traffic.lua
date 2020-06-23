
module("luci.controller.mobile_traffic", package.seeall)

local luasql = require "lsqlite3"
local utl = require "luci.util"

function index()
	local show = require("luci.tools.status").show_mobile()
	if show then
		--3G data usage page--
		entry({"admin", "status", "usage"}, call("go_to"), _("Mobile Traffic"), 8)

		entry({"admin", "status", "usage", "day"}, template("mobile_traffic/day_data_usage"),
			_("Today"), 1).leaf = true
		entry({"admin", "status", "usage", "usage_day"}, call("data_current")).leaf = true

		entry({"admin", "status", "usage", "week"}, template("mobile_traffic/week_data_usage"),
			_("Current Week"), 2).leaf = true
		entry({"admin", "status", "usage", "usage_week"}, call("data_days")).leaf = true

		entry({"admin", "status", "usage", "month"}, template("mobile_traffic/data_usage"),
			_("Current Month"), 3).leaf = true
		entry({"admin", "status", "usage", "usage_month"}, call("data_days")).leaf = true
		
		entry({"admin", "status", "usage", "limit"}, template("mobile_traffic/data_usage_limit"),
			_("Data Limit Period"), 5).leaf = true
		entry({"admin", "status", "usage", "usage_limit"}, call("data_limit")).leaf = true

		entry({"admin", "status", "usage", "year"}, template("mobile_traffic/year_data_usage"),
			_("Total"), 6).leaf = true
		entry({"admin", "status", "usage", "usage_month"}, call("data_days")).leaf = true
		
		entry({"admin", "status", "usage", "delete_all_data"}, call("reset_all_data")).leaf = true
	end
end

function go_to()
	luci.http.redirect(luci.dispatcher.build_url("admin", "status", "usage", "day").."/")
end

function data_limit()
	local datalimit = utl.trim(luci.sys.exec("uci -q get mdcollectd.config.datalimit")) or "0"
	local path  = luci.dispatcher.context.requestpath
	local sim = path[#path]
	sim = sim == "sim1" and "1" or sim == "sim2" and "0" or sim 
	local limit_period = ""
	local limit_start = "0"
	local limit_code = "4"
	local dbPath = "/var/"
	local dbName = "mdcollectd.db"
	local dbFullPath = dbPath .. dbName 
	local data = { }
	local query
	
	--month 1, week 2, day 3, off 4 this code for data limit graphs
	--FIXME: uci library? Never heard of it.
	if (sim == "sim1" or sim == "1") and datalimit ~= "0"  then
		limit_period = utl.trim(luci.sys.exec("uci -q get data_limit.limit.prim_conn_period"))
		if 	limit_period == "month" then
			limit_code = "1"
			limit_start = utl.trim(luci.sys.exec("uci -q get data_limit.limit.prim_conn_day"))
		elseif limit_period == "week" then
			limit_code = "2"
			limit_start = utl.trim(luci.sys.exec("uci -q get data_limit.limit.prim_conn_weekday"))
		elseif  limit_period == "day" then
			limit_code = "3"
			limit_start = utl.trim(luci.sys.exec("uci -q get data_limit.limit.prim_conn_hour"))
		end
	elseif (sim == "sim2" or sim == "0") and datalimit ~= "0" then
		limit_period = utl.trim(luci.sys.exec("uci -q get data_limit.limit.sec_conn_period"))
		if 	limit_period == "month" then
			limit_code = "1"
			limit_start = utl.trim(luci.sys.exec("uci -q get data_limit.limit.sec_conn_day"))
		elseif limit_period == "week" then
			limit_code = "2"
			limit_start = utl.trim(luci.sys.exec("uci -q get data_limit.limit.sec_conn_weekday"))
		elseif  limit_period == "day" then
			limit_code = "3"
			limit_start = utl.trim(luci.sys.exec("uci -q get data_limit.limit.sec_conn_hour"))
		end
	end
	--////////////////////////////////////////////////////////////////////
	
	if limit_code == "3" then
		local start_hour =tonumber(limit_start)
		if fileExists(dbPath, dbName) then
			local db = luasql.open(dbFullPath)
			local time = os.time()
			local year, month, day, hourr = tonumber(os.date("%Y", time)), tonumber(os.date("%m", time)), tonumber(os.date("%d", time)), tonumber(os.date("%H", time))
			if hourr < start_hour then
				day = tonumber(os.date("%d", time-(60*60*24)))
			end
			local timestamp = os.time{ year = year, month = month, day = day, hour = start_hour, min = 00, sec = 00 }
			
			query = string.format("SELECT * from current WHERE sim=%s AND time >= %s;", sim, timestamp)
			
			local stmt = db:prepare(query)
			local count = 0
			luci.http.prepare_content("application/json")

			if stmt then
				luci.http.write("[")
				for row in db:nrows(query) do
					count = count+1
					if count > 1 then
						luci.http.write(string.format(","))
					end
					luci.http.write(string.format("[ %s, 0, %s, %s, %s, %s ]", row.time,  row.rx, row.tx, limit_code, limit_start))

				end
				luci.http.write("]")
			end
			db:close()
		end
	else
		luci.http.prepare_content("application/json")
		if fileExists(dbPath, dbName) then
			local db = luasql.open(dbFullPath)
			if sim == "all" then
				query = string.format("SELECT * from days")
			else
				query = string.format("SELECT * from days WHERE sim=%s", sim)
			end
			local stmt = db:prepare(query)
			local count = 0

			if stmt then
				for row in db:nrows(query) do
					if #data > 0 then
						same_day = os.date("%d", data[#data].time) == os.date("%d", row.time)
						sim = data[#data].sim ~= row.sim
						if same_day and sim then
							data[#data].rx = data[#data].rx + row.rx
							data[#data].tx = data[#data].tx + row.tx
						else
							table.insert(data, row) 
						end
					else
						table.insert(data, row)
					end
				end
				if data then
					luci.http.write("[")
					for id, row in ipairs(data) do
						count = count+1
						if count > 1 then
							luci.http.write(string.format(","))
						end
						luci.http.write(string.format("[ %s, 0, %s, %s, %s, %s ]", row.time,  row.rx, row.tx, limit_code, limit_start))
					end
					luci.http.write("]")
				end
			end
			db:close()
		end
	end
end

function data_days()
	local path  = luci.dispatcher.context.requestpath
	local sim = path[#path]
	sim = sim == "sim1" and "1" or sim == "sim2" and "0" or "all"

	local dbPath = "/var/"
	local dbName = "mdcollectd.db"
	local dbFullPath = dbPath .. dbName 
	local data = { }
	local oldest_sim1 = 0
	local oldest_sim2 = 0
	local query
	luci.http.prepare_content("application/json")
	if fileExists(dbPath, dbName) then
		local db = luasql.open(dbFullPath)
		if sim == "all" then
			query = string.format("SELECT * from days")
		else
			query = string.format("SELECT * from days WHERE sim=%s", sim)
		end
		local stmt = db:prepare(query)
		local count = 0

		if stmt then
			for row in db:nrows(query) do
				if row.sim == "1" and oldest_sim1 < row.time then
					oldest_sim1 = row.time
				elseif row.sim == "2" and oldest_sim2 < row.time then
					oldest_sim2 = row.time
				end
				if #data > 0 then
					same_day = os.date("%d", data[#data].time) == os.date("%d", row.time)
					different_sim = data[#data].sim ~= row.sim
					if same_day and different_sim then
						data[#data].rx = data[#data].rx + row.rx
						data[#data].tx = data[#data].tx + row.tx
					else
						table.insert(data, row) 
					end
				else
					table.insert(data, row)
				end
			end
			if data then
				luci.http.write("[")
				for id, row in ipairs(data) do
					count = count+1
					if count > 1 then
						luci.http.write(string.format(","))
					end
					luci.http.write(string.format("[ %s, 0, %s, %s ]", row.time,  row.rx, row.tx))

				end
				luci.http.write("]")
			end
		end
		db:close()
	end
end

function reset_all_data(table_name, sim)
	sim = sim == "sim1" and "1" or sim == "sim2" and "0" or sim

	local dbPath = "/var/"
	local dbName = "mdcollectd.db"
	local dbFullPath = dbPath .. dbName
	local queries = {}
	local write_ok = 0
	local query
	luci.http.prepare_content("application/json")
	if fileExists(dbPath, dbName) then
		if sim == "all" then
			table.insert(queries, string.format("DELETE FROM %s", table_name))
			if table_name == "days" then
				table.insert(queries, string.format("DELETE FROM current"))
			end
		else
			table.insert(queries, string.format("DELETE FROM %s WHERE sim=%s", table_name, sim))
			if table_name == "days" then
				table.insert(queries, string.format("DELETE FROM current WHERE sim=%s", sim))
			end
		end

		for _, query in ipairs(queries) do
			local db = luasql.open(dbFullPath)
			local stmt = db:prepare(query)
			if stmt then
				stmt:step()
				stmt:finalize()
				db:close()
				write_ok = write_ok + 1
			else
				db:close()
				write_ok = write_ok + 1
			end
		end
	end

	if write_ok == 2 or (write_ok == 1 and table_name == "current") then
		luci.http.write("[1]")
	else
		luci.http.write("[0]")
	end

end

function data_current()
	local path  = luci.dispatcher.context.requestpath
	local sim = path[#path]
	sim = sim == "sim1" and "1" or sim == "sim2" and "0" or sim 
	
	local dbPath = "/var/"
	local dbName = "mdcollectd.db"
	local dbFullPath = dbPath .. dbName 
	local query
	luci.http.prepare_content("application/json")

	if fileExists(dbPath, dbName) then
		local db = luasql.open(dbFullPath)
		local time = os.time()
		local year, month, day = tonumber(os.date("%Y", time)), tonumber(os.date("%m", time)), tonumber(os.date("%d", time))
		local timestamp = os.time{ year = year, month = month, day = day, hour = 00, min = 00, sec = 00 }
		
		if sim == "all" then
			query = string.format("SELECT * from current WHERE time >= %s;", timestamp)
		else
			query = string.format("SELECT * from current WHERE sim=%s AND time >= %s;", sim, timestamp)
		end
		local stmt = db:prepare(query)
		local count = 0
		luci.http.prepare_content("application/json")

		if stmt then
			luci.http.write("[")
			for row in db:nrows(query) do
				count = count+1
				if count > 1 then
					luci.http.write(string.format(","))
				end
				luci.http.write(string.format("[ %s, 0, %s, %s ]", row.time,  row.rx, row.tx))

			end
			luci.http.write("]")
		end
		db:close()
	end
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
