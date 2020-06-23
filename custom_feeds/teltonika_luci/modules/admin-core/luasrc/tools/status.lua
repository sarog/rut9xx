--[[
LuCI - Lua Configuration Interface

Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: status.lua 8049 2011-12-05 18:35:00Z jow $
]]--

module("luci.tools.status", package.seeall)

local uci = require "luci.model.uci".cursor()

function dhcp_leases()
	local rv = { }
	local nfs = require "nixio.fs"
	local leasefile = "/var/dhcp.leases"

	uci:foreach("dhcp", "dnsmasq",
		function(s)
			if s.leasefile and nfs.access(s.leasefile) then
				leasefile = s.leasefile
				return false
			end
		end)

	local fd = io.open(leasefile, "r")
	if fd then
		while true do
			local ln = fd:read("*l")
			if not ln then
				break
			else
				local ts, mac, ip, name = ln:match("^(%d+) (%S+) (%S+) (%S+)")
				if ts and mac and ip and name then
					rv[#rv+1] = {
						expires  = os.difftime(tonumber(ts) or 0, os.time()),
						macaddr  = mac,
						ipaddr   = ip,
						hostname = (name ~= "*") and name
					}
				end
			end
		end
		fd:close()
	end

	return rv
end

function wifi_networks()
	local rv = { }
	local ntm = require "luci.model.network".init()

	local dev
	for _, dev in ipairs(ntm:get_wifidevs()) do
		local rd = {
			up       = dev:is_up(),
			device   = dev:name(),
			name     = dev:get_i18n(),
			networks = { }
		}

		local net
		for _, net in ipairs(dev:get_wifinets()) do
			rd.networks[#rd.networks+1] = {
				name       = net:shortname(),
				link       = net:adminlink(),
				up         = net:is_up(),
				mode       = net:active_mode(),
				ssid       = net:active_ssid(),
				bssid      = net:active_bssid(),
				encryption = net:active_encryption(),
				frequency  = net:frequency(),
				channel    = net:channel(),
				signal     = net:signal(),
				quality    = net:signal_percent(),
				noise      = net:noise(),
				bitrate    = net:bitrate(),
				ifname     = net:ifname(),
				assoclist  = net:assoclist(),
				country    = net:country(),
				countryname = net:countryname(),
				txpower    = net:txpower(),
				txpoweroff = net:txpower_offset()
			}
			--luci.sys.exec("echo " .. rd.device .. "+"..#rd.networks.." >> /tmp/log.log")
		end

		rv[#rv+1] = rd
		return rv
	end

	return rv
end

function wifi_network(id)
	local ntm = require "luci.model.network".init()
	local net = ntm:get_wifinet(id)
	if net then
		local dev = net:get_device()
		if dev then
			return {
				id         = id,
				name       = net:shortname(),
				link       = net:adminlink(),
				up         = net:is_up(),
				mode       = net:active_mode(),
				ssid       = net:active_ssid(),
				bssid      = net:active_bssid(),
				encryption = net:active_encryption(),
				frequency  = net:frequency(),
				channel    = net:channel(),
				signal     = net:signal(),
				quality    = net:signal_percent(),
				noise      = net:noise(),
				bitrate    = net:bitrate(),
				ifname     = net:ifname(),
				assoclist  = net:assoclist(),
				country    = net:country(),
				countryname = net:countryname(),
				txpower    = net:txpower(),
				txpoweroff = net:txpower_offset(),
				device     = {
					up     = dev:is_up(),
					device = dev:name(),
					name   = dev:get_i18n()
				}
			}
		end
	end
	return { }
end

function switch_status(devs)
	local dev
	local switches = { }
	for dev in devs:gmatch("[^%s,]+") do
		local ports = { }
		local swc = io.popen("swconfig dev %q show" % dev, "r")
		if swc then
			local l
			repeat
				l = swc:read("*l")
				if l then
					local port, up = l:match("port:(%d+) link:(%w+)")
					if port then
						local speed  = l:match(" speed:(%d+)")
						local duplex = l:match(" (%w+)-duplex")
						local txflow = l:match(" (txflow)")
						local rxflow = l:match(" (rxflow)")
						local auto   = l:match(" (auto)")

						ports[#ports+1] = {
							port   = tonumber(port) or 0,
							speed  = tonumber(speed) or 0,
							link   = (up == "up"),
							duplex = (duplex == "full"),
							rxflow = (not not rxflow),
							txflow = (not not txflow),
							auto   = (not not auto)
						}
					end
				end
			until not l
			swc:close()

			local wan_port = io.popen("cat /sys/class/net/eth1/operstate", "r")
			if wan_port then
			  l = wan_port:read("*l")
			  ports[#ports+1] =  {
				  link = (l == "up")
			  }
			end
			wan_port:close()
		end
		switches[dev] = ports
	end
	return switches
end

function sms_get()
	local sms = io.popen("gsmctl -S -l all", "r")
	local sms_list = { }
	local n = 1
	local line = false
	if sms then
		local l
		repeat
			l = sms:read("*l")
			if l then
				local index = l:match("Index: (%d+)")
				local sms_date = l:match("Date: .+")
				local sender = l:match("Sender: +.*")
				local text = l:match("Text:.+")
				local status = l:match("Status: (%w+)")
				if index then
					sms_list[n] = {
						index = index
					}
				elseif sms_date then
					local pattern = "(%d+)-(%d+)-(%d+) (%d+):(%d+):(%d+)"
					sms_date = sms_date:gsub("Date: ", "")
					--sms_list[n].date = sms_date
					local runyear, runmonth, runday, runhour, runminute, runseconds = sms_date:match(pattern)
					local convertedTimestamp = os.time({year = runyear, month = runmonth, day = runday, hour = runhour, min = runminute, sec = runseconds})
					sms_list[n].date = convertedTimestamp
				elseif sender then
					sender = sender:gsub("Sender: ", "")
					sms_list[n].sender = sender
				elseif text then
					text = text:gsub("Text: ", "")
					sms_list[n].text = text
					line = true
				elseif status then
					sms_list[n].status =  status
				elseif l == "------------------------------" then
					n = n + 1
				elseif line then
					sms_list[n].text =sms_list[n].text.."<br>"..l
				end
			end
		until not l
		sms:close()
	end
	table.sort(sms_list, function(a,b) return a.date>b.date end)
	return sms_list
end

function show_mobile()
	local number = uci:get("hwinfo", "hwinfo", "mnf_code")

	if number then
		number = number:sub(7, 7)
		if number == "N" then
			return false
		end
	end

	return true
end

function gps_on()
	local is_hw = tonumber(uci:get("hwinfo", "hwinfo", "gps"))

	if is_hw == 1 then
		local is_on = tonumber(uci:get("gps", "gpsd", "enabled"))
		if is_on == 1 then
			return true
		end
	end
	return false
end
