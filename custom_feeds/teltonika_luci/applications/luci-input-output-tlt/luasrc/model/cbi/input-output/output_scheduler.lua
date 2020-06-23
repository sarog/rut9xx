local sys = require "luci.sys"
local trap = false

m = Map( "output_control", translate( "Output Scheduler" ), translate( "" ) )

is_4pin = m.uci:get("hwinfo", "hwinfo", "4pin_io") or "0"
is_io = m.uci:get("hwinfo", "hwinfo", "in_out") or "0"

s = m:section( NamedSection, "scheduler", "scheduler", translate("Configure Scheduled Outputs"), translate("" ))


enabled = s:option(Flag, "enabled", translate("Enable"), translate("Check to enable the output scheduler"))


gpio = s:option(ListValue, "gpio", translate("Output"), translate("Select which output will be configured"))
	if is_io == "1" then
		gpio:value("DOUT1", translate("Open collector output"))
		gpio:value("DOUT2", translate("Relay output"))
	end
	if is_4pin == "1" then
			gpio:value("DOUT3", translate("Digital 4PIN output"))
			if is_io == "1" then
				gpio:value("DOUT1_2", translate("Open collector and relay output"))
				gpio:value("DOUT1_3", translate("Open collector and 4PIN output"))
				gpio:value("DOUT2_3", translate("Relay and 4PIN output"))
			end
	end
	if is_io == "1" then
		gpio:value("all", translate("All"))
	end
function gpio.write() end
	
tbl = s:option( Value, "days")
tbl.template="input-output/scheduler"

function tbl.write(self, section, value)
	enabled = m:formvalue("cbid.output_control.scheduler.enabled") or "0"

	if not trap then
		local path = "/etc/scheduler/config"
		local days={"mon", "tue", "wed", "thu", "fri", "sat", "sun"}
		local script = "/sbin/gpio.sh"
		local pin = m.uci:get("output_control", "scheduler", "gpio")
		local line, cron
		local pin_state = {}
		pin_state["DOUT1"] = 0
		pin_state["DOUT2"] = 0
		pin_state["DOUT3"] = 0
		local set = {}
		local clear = {}
		local old_hr
		local empty = true
		local current_day = sys.exec("date +%a")
		local current_hour = tonumber(sys.exec("date +%H"))
		daysnr={[1] = "Mon\n", [2] = "Tue\n", [3] = "Wed\n", [4] = "Thu\n", [5] = "Fri\n", [6] = "Sat\n", [7] = "Sun\n"}
		file = io.open(path, "w+")
		os.execute('sed -i "/gpio.sh/d" /etc/crontabs/root')

		for i=1,7  do
			n = 1
			hours = string.sub(value, i*24-23, i*24)
			local day_of_week = daysnr[i]
			local hr_nr = 0
			
			for hr in hours:gmatch(".") do
				if day_of_week == current_day and hr_nr == current_hour and enabled == "1" then
					if hr == "1" then
						sys.exec('/sbin/gpio.sh set DOUT1')
						sys.exec('/sbin/gpio.sh clear DOUT2')
						sys.exec('/sbin/gpio.sh clear DOUT3')
					elseif hr == "2" then
						sys.exec('/sbin/gpio.sh clear DOUT1')
						sys.exec('/sbin/gpio.sh set DOUT2')
						sys.exec('/sbin/gpio.sh clear DOUT3')
					elseif hr == "3" then
						sys.exec('/sbin/gpio.sh clear DOUT1')
						sys.exec('/sbin/gpio.sh clear DOUT2')
						sys.exec('/sbin/gpio.sh set DOUT3')
					elseif hr == "4" then
						sys.exec('/sbin/gpio.sh set DOUT1')
						sys.exec('/sbin/gpio.sh set DOUT2')
						sys.exec('/sbin/gpio.sh clear DOUT3')
					elseif hr == "5" then
						sys.exec('/sbin/gpio.sh set DOUT1')
						sys.exec('/sbin/gpio.sh clear DOUT2')
						sys.exec('/sbin/gpio.sh set DOUT3')
					elseif hr == "6" then
						sys.exec('/sbin/gpio.sh clear DOUT1')
						sys.exec('/sbin/gpio.sh set DOUT2')
						sys.exec('/sbin/gpio.sh set DOUT3')
					elseif hr == "7" then
						sys.exec('/sbin/gpio.sh set DOUT1')
						sys.exec('/sbin/gpio.sh set DOUT2')
						sys.exec('/sbin/gpio.sh set DOUT3')
					elseif hr == "0" then
						sys.exec('/sbin/gpio.sh clear DOUT1')
						sys.exec('/sbin/gpio.sh clear DOUT2')
						sys.exec('/sbin/gpio.sh clear DOUT3')
					end
				end
				hr_nr = hr_nr + 1
				
				if hr == "1" then
					if is_io == "1" then table.insert(set, "DOUT1") end
					if is_io == "1" then table.insert(clear, "DOUT2") end
					if is_4pin == "1" then table.insert(clear, "DOUT3") end
				elseif hr == "2" then
					if is_io == "1" then table.insert(clear, "DOUT1") end
					if is_io == "1" then table.insert(set, "DOUT2") end
					if is_4pin == "1" then table.insert(clear, "DOUT3") end
				elseif hr == "3" then
					if is_io == "1" then table.insert(clear, "DOUT1") end
					if is_io == "1" then table.insert(clear, "DOUT2") end
					if is_4pin == "1" then table.insert(set, "DOUT3") end
				elseif hr == "4" then
					if is_io == "1" then table.insert(set, "DOUT1") end
					if is_io == "1" then table.insert(set, "DOUT2") end
					if is_4pin == "1" then table.insert(clear, "DOUT3") end
				elseif hr == "5" then
					if is_io == "1" then table.insert(set, "DOUT1") end
					if is_io == "1" then table.insert(clear, "DOUT2") end
					if is_4pin == "1" then table.insert(set, "DOUT3") end
				elseif hr == "6" then
					if is_io == "1" then table.insert(clear, "DOUT1") end
					if is_io == "1" then table.insert(set, "DOUT2") end
					if is_4pin == "1" then table.insert(set, "DOUT3") end
				elseif hr == "7" then
					if is_io == "1" then table.insert(set, "DOUT1") end
					if is_io == "1" then table.insert(set, "DOUT2") end
					if is_4pin == "1" then table.insert(set, "DOUT3") end
				elseif hr == "0" then
					if is_io == "1" then table.insert(clear, "DOUT1") end
					if is_io == "1" then table.insert(clear, "DOUT2") end
					if is_4pin == "1" then table.insert(clear, "DOUT3") end
				end
				
				if enabled == "1" then
					if old_hr ~= hr then
						cron_conf = io.open("/etc/crontabs/root", "a")

						for key, value in pairs(set) do
							if pin_state[value] ~= 1 then
								cron = string.format("0 %s * * %s %s set %s%s", n-1, days[i], script, value, "\n")
								cron_conf:write(cron)
								pin_state[value] = 1
								empty = false
								os.execute("logger " ..cron)
							end
						end
						
						for key, value in pairs(clear) do
							if pin_state[value] ~= 0 then
								cron = string.format("0 %s * * %s %s clear %s%s", n-1, days[i], script, value, "\n")
								cron_conf:write(cron)
								pin_state[value] = 0
								os.execute("logger " ..cron)
							end
						end
						cron_conf:close()
					end
				end
				n = n + 1
				old_hr = hr
				set = {}
				clear = {}
			end
			line = string.format("%s:%s%s", days[i], hours, "\n")
			--os.execute("logger " ..line)
			file:write(line)
		end
		file:close()
		trap = true
		if enabled == "1" then
			cron_conf = io.open("/etc/crontabs/root", "a")
			--check and clear output values on monday
			monday_val = string.sub(value, 1, 1)
			sunday_val = string.sub(value, #value, #value)

			if sunday_val == '1' then
				if monday_val == '2' or monday_val == '3' or monday_val == '6' or monday_val == '0' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT1", "\n")
					cron_conf:write(cron)
				end
			elseif sunday_val == '2' then
				if monday_val == '1' or monday_val == '3' or monday_val == '5' or monday_val == '0' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT2", "\n")
					cron_conf:write(cron)
				end
			elseif sunday_val == '3' then
				if monday_val == '1' or monday_val == '2' or monday_val == '4' or monday_val == '0' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT3", "\n")
					cron_conf:write(cron)
				end
			elseif sunday_val == '4' then	--check for outputs DOUT1 and DOUT2
				if monday_val == '1' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT2", "\n")
					cron_conf:write(cron)
				elseif monday_val == '2' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT1", "\n")
					cron_conf:write(cron)
				elseif monday_val == '3' or monday_val == '0' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT1", "\n")
					cron_conf:write(cron)
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT2", "\n")
					cron_conf:write(cron)
				elseif monday_val == '5' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT2", "\n")
					cron_conf:write(cron)
				elseif monday_val == '6' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT1", "\n")
					cron_conf:write(cron)
				end
			elseif sunday_val == '5' then	--check for outputs DOUT1 and DOUT3
				if monday_val == '1' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT3", "\n")
					cron_conf:write(cron)
				elseif monday_val == '2' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT1", "\n")
					cron_conf:write(cron)
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT3", "\n")
					cron_conf:write(cron)
				elseif monday_val == '3' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT1", "\n")
					cron_conf:write(cron)
				elseif monday_val == '4' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT3", "\n")
					cron_conf:write(cron)
				elseif monday_val == '6' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT1", "\n")
					cron_conf:write(cron)
				elseif monday_val == '0' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT1", "\n")
					cron_conf:write(cron)
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT3", "\n")
					cron_conf:write(cron)
				end
			elseif sunday_val == '6' then	--check for outputs DOUT2 and DOUT3
				if monday_val == '1' or monday_val == '0' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT2", "\n")
					cron_conf:write(cron)
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT3", "\n")
					cron_conf:write(cron)
				elseif monday_val == '2' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT3", "\n")
					cron_conf:write(cron)
				elseif monday_val == '3' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT2", "\n")
					cron_conf:write(cron)
				elseif monday_val == '4' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT3", "\n")
					cron_conf:write(cron)
				elseif monday_val == '5' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT2", "\n")
					cron_conf:write(cron)
				end
			elseif sunday_val == '7' then --check for outputs DOUT1 and DOUT2 and DOUT3
				if monday_val == '1' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT2", "\n")
					cron_conf:write(cron)
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT3", "\n")
					cron_conf:write(cron)
				elseif monday_val == '2' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT1", "\n")
					cron_conf:write(cron)
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT3", "\n")
					cron_conf:write(cron)
				elseif monday_val == '3' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT1", "\n")
					cron_conf:write(cron)
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT2", "\n")
					cron_conf:write(cron)
				elseif monday_val == '4' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT3", "\n")
					cron_conf:write(cron)
				elseif monday_val == '5' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT2", "\n")
					cron_conf:write(cron)
				elseif monday_val == '6' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT1", "\n")
					cron_conf:write(cron)
				elseif monday_val == '0' then
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT1", "\n")
					cron_conf:write(cron)
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT2", "\n")
					cron_conf:write(cron)
					cron = string.format("0 0 * * mon %s clear %s%s", script, "DOUT3", "\n")
					cron_conf:write(cron)
				end
			end
			cron_conf:close()
		else
			os.execute('sed -i "/gpio.sh/d" /etc/crontabs/root')
		end
	
		if empty then
			os.execute('sed -i "/gpio.sh/d" /etc/crontabs/root')
		end
	end
end

return m
