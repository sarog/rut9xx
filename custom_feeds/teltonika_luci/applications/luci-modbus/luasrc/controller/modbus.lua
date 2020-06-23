module("luci.controller.modbus", package.seeall)

local uci = require "luci.model.uci".cursor()
local utl = require "luci.util"
local sys = require "luci.sys"

function index()
	local uci = require "luci.model.uci".cursor()
	local rs232 = uci:get("hwinfo", "hwinfo", "rs232") and uci:get("hwinfo", "hwinfo", "rs232") or "0"
	local rs485 = uci:get("hwinfo", "hwinfo", "rs485") and uci:get("hwinfo", "hwinfo", "rs485")  or "0"

	entry({"admin", "services", "modbus"}, alias("admin", "services", "modbus", "modbus"), _("Modbus"), 97)
	entry({"admin", "services", "modbus", "modbus" }, cbi("modbus"), _("Modbus TCP slave"), 1)
	entry({"admin", "services", "modbus", "modbus_master"}, arcombine(cbi("modbus_master"), cbi("slave_details")), _("Modbus TCP Master"), 30).leaf = true
	entry({"admin", "services", "modbus", "modbus_master_alarms"}, cbi("modbus_master_alarms"), _(NIL), 31).leaf = true
	entry({"admin", "services", "modbus", "modbus_master_alarm_details"}, cbi("alarm_details"), _(NIL), 32).leaf = true

	if rs232 == "1" or rs485 == "1" then
		entry({"admin", "services", "modbus", "modbus_serial_master"}, alias("admin", "services", "modbus", "modbus_serial_master", rs232 == "1" and "rs232" or "rs485"), _("Modbus Serial Master"), 35)
		entry({"admin", "services", "modbus", "modbus_serial_master_alarms"}, cbi("modbus_serial_master_alarms"), _(NIL), 31).leaf = true
		entry({"admin", "services", "modbus", "modbus_serial_master_alarm_details"}, cbi("serial_alarm_details"), _(NIL), 32).leaf = true
		entry({"admin", "services", "modbus", "serial_test"}, call("test_serial_request"), nil, nil).leaf = true

		if rs232 == "1" then
			entry({"admin", "services", "modbus", "modbus_serial_master", "rs232"}, arcombine(cbi("rs232"), cbi("rs232_edit")), _("RS232"), 33).leaf = true
		end

		if rs485 == "1" then
			entry({"admin", "services", "modbus", "modbus_serial_master", "rs485"}, arcombine(cbi("rs485"), cbi("rs485_edit")), _("RS485"), 34).leaf = true
		end

	end

	entry({"admin", "services", "modbus", "modbus_data_sender"}, arcombine(cbi("modbus_data_sender"), cbi("modbus_data_sender_details")), _("Modbus Data to Server"), 36).leaf = true
	entry({"admin", "services", "modbus", "test"}, call("test_request"), nil, nil).leaf = true
end

function copy_section(section_type, old_config, old_section, new_config, new_section)
	
	uci:foreach(old_config, section_type, function(s)
		if s[".name"] == old_section then
			for key, value in pairs(s) do
				if key and value and key ~= ".name" and key ~= ".index" then
					if type(value) == "table" then
						uci:set(new_config, new_section, tostring(key), value)
					else
						uci:set(new_config, new_section, tostring(key), tostring(value))
					end
				end
			end
		end
	end)
end

function test_serial_request()
	local cfg = luci.http.formvalue("cfg")
	local cmd = "/usr/sbin/modbus_serial_request_test"

	if cfg then
		luci.http.prepare_content("text/plain")
		cmd = cmd .. " " .. cfg
		local util = io.popen(cmd)
		if util then
			while true do
				local ln = util:read("*l")
				if not ln then break end
				luci.http.write(ln .. "\n")
			end
			util:close()
		end
	else
		luci.http.write("Unable to send request\n")
	end
end

function test_request()
	local arg = {}
	arg[1] = "/usr/sbin/modbus_tcp_test"
	arg[2] = "'" .. luci.http.formvalue("ip"):gsub("'", "") .. "'"
	arg[3] = "'" .. luci.http.formvalue("port"):gsub("'", "") .. "'"
	arg[4] = "'" .. luci.http.formvalue("timeout"):gsub("'", "") .. "'"
	arg[5] = "'" .. luci.http.formvalue("slave_id"):gsub("'", "") .. "'"
	arg[6] = "'" .. luci.http.formvalue("fc"):gsub("'", "") .. "'"
	arg[7] = "'" .. luci.http.formvalue("regaddr"):gsub("'", "") .. "'"
	arg[8] = "'" .. luci.http.formvalue("payload"):gsub("'", "") .. "'"
	arg[9] = "'" .. luci.http.formvalue("datatype"):gsub("'", "") .. "' 2>&1"
	local cmd = table.concat(arg, " ")

	luci.http.prepare_content("text/plain")
	local util = io.popen(cmd)
	if util then
		while true do
			local ln = util:read("*l")
			if not ln then break end
			luci.http.write(ln .. "\n")
		end
		util:close()
	else
		luci.http.write("Test failed")
	end
end
