--[[

LuCI - Configuration Bind Interface - Datatype Tests
(c) 2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

$Id: datatypes.lua 8557 2012-04-13 18:12:34Z soma $

]]--

local fs = require "nixio.fs"
local ip = require "luci.ip"
local math = require "math"
local util = require "luci.util"
local uci = require "luci.model.uci"
local librex = require "librex_lua"
local tonumber, type, unpack, select, ipairs = tonumber, type, unpack, select, ipairs
local _uci_real


module "luci.cbi.datatypes"

_uci_real  = _uci_real or uci.cursor()

_M['or'] = function(v, ...)
	local i
	for i = 1, select('#', ...), 2 do
		local f = select(i, ...)
		local a = select(i+1, ...)
		if type(f) ~= "function" then
			if f == v then
				return true
			end
			i = i - 1
		elseif f(v, unpack(a)) then
			return true
		end
	end
	return false
end

_M['and'] = function(v, ...)
	local i
	for i = 1, select('#', ...), 2 do
		local f = select(i, ...)
		local a = select(i+1, ...)
		if type(f) ~= "function" then
			if f ~= v then
				return false
			end
			i = i - 1
		elseif not f(v, unpack(a)) then
			return false
		end
	end
	return true
end

function neg(v, ...)
	return _M['or'](v:gsub("^%s*!%s*", ""), ...)
end

function list(v, subvalidator, subargs)
	if type(subvalidator) ~= "function" then
		return false
	end
	local token
	for token in v:gmatch("%S+") do
		if not subvalidator(token, unpack(subargs)) then
			return false
		end
	end
	return true
end

function bool(val)
	if val == "1" or val == "yes" or val == "on" or val == "true" then
		return true
	elseif val == "0" or val == "no" or val == "off" or val == "false" then
		return true
	elseif val == "" or val == nil then
		return true
	end

	return false
end

function uinteger(val)
	local n = tonumber(val)
	if n ~= nil and math.floor(n) == n and n >= 0 then
		return true
	end

	return false
end

function integer(val)
	local n = tonumber(val)
	if n ~= nil and math.floor(n) == n then
		return true
	end

	return false
end

function ufloat(val)
	local n = tonumber(val)
	return ( n ~= nil and n >= 0 )
end

function float(val)
	return ( tonumber(val) ~= nil )
end

function ipaddr(val)
	return ip4addr(val) or ip6addr(val)
end

function ip4addr(val)
	if val then
		return ip.IPv4(val) and true or false
	end

	return false
end

function ip4lan(val)
	if val then
		return ip.IPv4(val) and true or false
	end

	return false
end

function ip4prefix(val)
	val = tonumber(val)
	return ( val and val >= 0 and val <= 32 )
end

function ip6addr(val)
	if val then
		return ip.IPv6(val) and true or false
	end

	return false
end

function ip6prefix(val)
	val = tonumber(val)
	return ( val and val >= 0 and val <= 128 )
end

function port(val)
	val = tonumber(val)
	return ( val and val >= 0 and val <= 65535 )
end

function portrange(val)
	local p1, p2 = val:match("^(%d+)%-(%d+)$")
	if p1 and p2 and port(p1) and port(p2) then
		return true
	else
		return port(val)
	end
end

function macaddr(val)
	if val and val == "*:*:*:*:*:*" then
		return true
	end

	if val and val:match(
		"^[a-fA-F0-9]+:[a-fA-F0-9]+:[a-fA-F0-9]+:" ..
		 "[a-fA-F0-9]+:[a-fA-F0-9]+:[a-fA-F0-9]+$"
	) then
		local parts = util.split( val, ":" )

		for i = 1,6 do
			parts[i] = tonumber( parts[i], 16 )
			if parts[i] < 0 or parts[i] > 255 then
				return false
			end
		end

		return true
	end

	return false
end

function macaddr_range(val)
	local sep = "-"
	if val and val:match(sep) then
		local mac = util.split(val, "-")
		local left_mac = mac[1]
		left_mac = left_mac:lower()
		local right_mac = mac[2]
		right_mac = right_mac:lower()
		if left_mac and macaddr(left_mac) and right_mac and macaddr(right_mac) then
			local left_parts = util.split(left_mac, ":")
			local right_parts = util.split(right_mac, ":")

			for i = 1,6 do
				left_parts[i] = tonumber(left_parts[i], 16)
				right_parts[i] = tonumber(right_parts[i], 16)
				if i ~= 6 then
					if left_parts[i] < right_parts[i] then
						return true
					elseif left_parts[i] > right_parts[i] then
						return false
					end
				else
					if left_parts[i] < right_parts[i] then
						return true
					else
						return false
					end
				end
			end
		else
			return false
		end
	else
		return false
	end
end

function macaddr_cmp(mac1, mac2)
	if mac1 and mac2 then
		local mac1_parts = util.split(mac1, ":")
		local mac2_parts = util.split(mac2, ":")

		for i = 1,6 do
			mac1_parts[i] = tonumber(mac1_parts[i], 16)
			mac2_parts[i] = tonumber(mac2_parts[i], 16)
			if i ~= 6 then
				if mac1_parts[i] < mac2_parts[i] then
					return 1
				elseif mac1_parts[i] > mac2_parts[i] then
					return -1
				end
			else
				if mac1_parts[i] < mac2_parts[i] then
					return 1
				elseif mac1_parts[i] > mac2_parts[i] then
					return -1
				else
					return 0
				end
			end
		end
	else
		return false
	end
end

function macaddr2(val)
	return true
end

function string_not_empty(val)
	return true
end

function username_validate(val)
	return true
end

function password_validate(val)
	return true
end

function root_password_validate(val)
	return true
end

function root_password(val)
	if val and (#val >= 8) and (#val <= 32) and
		val:match("[a-z]") and
		val:match("[A-Z]") and
		val:match("[0-9]") then
		return true
	end
	return false
end

function url(val)
	local tmp_val
	if val and (#val < 254) then
		tmp_val = librex.match(val, "(^(?:http(s)?:\/\/)?[\\w.-]+(?:\\.[\\w\\.-]+)+[\\w-\\._~:\/?#[\\]@!\\$%&'\\(\\)\\*\\+,;=]+$)")
		if tmp_val ~= nil and tmp_val == val then
			return true
		end
	end
	return false
end

function hostname_extern(val)
	if val and (#val < 254) and (
		val:match("^[a-zA-Z0-9%-%.]*[%.][a-zA-Z0-9]+$") or
		ipaddr(val)) then
		return true
	end
	return false
end

function hostname(val)
	if val and (#val < 254) and (
		val:match("^[a-zA-Z]+$") or
		(val:match("^[a-zA-Z0-9][a-zA-Z0-9%-%.]*[a-zA-Z0-9]$") and
		val:match("[^0-9%.]"))
	) then
		return true
	end
	return false
end

function valid_hostname(val)
	local tmp_val
	if val and (#val < 254) then
		tmp_val = librex.match(val, "(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)+([a-zA-Z])+$)")
		if tmp_val ~= nil and tmp_val == val then
			return true
		end
	end
	return false
end

function host(val)
	return hostname(val) or ipaddr(val)
end

function network(val)
	return uciname(val) or host(val)
end

function tlt_hostname(val)
	if network(val) and #val < 33 then
		return true
	end

	return false
end

function wpakey(val)
	if #val == 64 then
		return (val:match("^[a-fA-F0-9]+$") ~= nil)
	else
		return (#val >= 8) and (#val <= 63)
	end
end

function wepkey(val)
	if val:sub(1, 2) == "s:" then
		val = val:sub(3)
	end

	if (#val == 10) or (#val == 26) then
		return (val:match("^[a-fA-F0-9]+$") ~= nil)
	else
		return (#val == 5) or (#val == 13)
	end
end

function string(val)
	return true		-- Everything qualifies as valid string
end

function directory( val, seen )
	local s = fs.stat(val)
	seen = seen or { }

	if s and not seen[s.ino] then
		seen[s.ino] = true
		if s.type == "dir" then
			return true
		elseif s.type == "lnk" then
			return directory( fs.readlink(val), seen )
		end
	end

	return false
end

function file( val, seen )
	local s = fs.stat(val)
	seen = seen or { }

	if s and not seen[s.ino] then
		seen[s.ino] = true
		if s.type == "reg" then
			return true
		elseif s.type == "lnk" then
			return file( fs.readlink(val), seen )
		end
	end

	return false
end

function device( val, seen )
	local s = fs.stat(val)
	seen = seen or { }

	if s and not seen[s.ino] then
		seen[s.ino] = true
		if s.type == "chr" or s.type == "blk" then
			return true
		elseif s.type == "lnk" then
			return device( fs.readlink(val), seen )
		end
	end

	return false
end

function uciname(val)
	return (val:match("^[a-zA-Z0-9_]+$") ~= nil)
end

function nospace(val)
	return (val:match("^[^ ]+$") ~= nil)
end

function fieldvalidation(val, valmat, minlen)
	if #val >= minlen then
		return (val:match(valmat) ~= nil)
	end
	return false
end

function lengthvalidation(val, min, max, regex)
	if #val >= min and #val <= max then
		if regex then
			return (val:match(regex) ~= nil)
		else
			return true
		end
	end
	return false
end

function range(val, min, max)
	val = tonumber(val)
	min = tonumber(min)
	max = tonumber(max)

	if val ~= nil and min ~= nil and max ~= nil then
		return ((val >= min) and (val <= max))
	end

	return false
end

function min(val, min)
	val = tonumber(val)
	min = tonumber(min)

	if val ~= nil and min ~= nil then
		return (val >= min)
	end

	return false
end

function max(val, max)
	val = tonumber(val)
	max = tonumber(max)

	if val ~= nil and max ~= nil then
		return (val <= max)
	end

	return false
end

function phonedigit(val)
	return (val:match("^[0-9\*#]+$") ~= nil)
end

function phonenumber(val)
	return (val:match("^-?[+0-9]+$") ~= nil)
end

function remote_net(val)
	local addr
	if val then
		local ipadresas = _uci_real:get("network", "lan", "ipaddr")
		local network= {}
		for elem in ipadresas:gmatch("%d+") do
			network[#network + 1] = elem
		end
		addr = network[1].."."..network[2].."."..network[3]..".0"
		if val ~= addr then
			return true
		end
	end
	return false
end

function number_list(val)
		if val:match("^%d*$") ~= nil then -- eg. 80 or empty(option All)
			return true
		elseif val:match("^[,%d+]+$") ~= nil then -- eg. 80,8,,080,...,21,,
			local i, name
			local parts = util.split( val, "," )
				for i, name in ipairs(parts) do
					if name == nil or name == "" then
						return false
					end
				end
			return true
		else
			return false
		end
end

function leasetime_check(val, config)
    local letter = _uci_real:get(config, "lan", "letter")

    if (letter == "m" and tonumber(val) < 2) or tonumber(val) < 1 then
        return false
    end
    return true
end

function secure_username_input(val)
	if #val >= 1 and #val <= 64 then
		tmp_val = librex.match(val, "^[a-zA-Z0-9!@#\\$%&\\*\\+-/\\\\ =\\?\\^_`\\[\\({\\|}\\)\\]~\",\\.;<>]+$")
		if tmp_val ~= nil and tmp_val == val then
			return true
		end
        end
        return false
end

function secure_input(val)
	if #val >= 1 and #val <= 64 then
		tmp_val = librex.match(val, "^[a-zA-Z0-9!@#\\$%&\\*\\+-/\\\\ =\\?\\^_`\\[\\({\\|}\\)\\]~\",\\.;<>]+$")
		if tmp_val ~= nil and tmp_val == val then
			return true
		end
        end
        return false
end

function geofencing(val, min, max)
	local string_val = val
	val = tonumber(val)
	min = tonumber(min)
	max = tonumber(max)

	if (float(val) and min and max) then
		local decimal_part, floating_part = string_val:match("([^.]*)%.([^.]*)")
		if (floating_part and floating_part:len() == 6) then
			if (val >= min and val <= max) then
				return true
			end
		end
	end
	return false
end

function password(val, allow_space)
	local match

	if allow_space ~= nil then
		match = val:match("^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. %-<>:;[%]]+$")
	else
		match = val:match("^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~.%-<>:;[%]]+$")
	end

	return match ~= nil and true or false
end

function string_length(val, min, max)
	if #val >= min then
		if max ~= nil then
			return #val <= max and true or false
		end
		return true
	end
	return false
end

