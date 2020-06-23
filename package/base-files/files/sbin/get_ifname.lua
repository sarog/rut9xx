#!/usr/bin/env lua
-- Skriptas reikalingas Android APP
local sys = require "luci.sys"
local utl = require "luci.util"

function failture()
	local out =
	[[ Wrong argument
		get_ifname [SSID]
	]]
	print(out)
	os.exit()
end

function start()

	if arg[1] then
        ifname = os.execute("lua -e 'require \"teltonika_lua_functions\"; print(get_ifname(\"".. arg[1].. "\"))'") 
        return ifname
	else
		failture()
	end
end

start()
