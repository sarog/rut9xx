--[[
Teltonika R&D. ver 0.1
]]--

local fs = require "nixio.fs"
local fw = require "luci.model.firewall"
require("luci.fs")
require("luci.config")
eventlog = require'tlt_eventslog_lua'
local sys = require"luci.sys"
local util = require "luci.util"

m = Map("system", translate("WiMAX Administration Settings"),
	translate(""))

s3 = m:section(TypedSection, "_dummy", translate("WiMAX user Password"))
s3.addremove = false
s3.anonymous = true

pwW1 = s3:option(Value, "pwW1", translate("WiMAX password"), translate("Enter your WiMAX password"))
pwW1.password = true

pwW2 = s3:option(Value, "pwW2", translate("Confirm WiMAX password"), translate("Re-enter your WiMAX password"))
pwW2.password = true

function s3.cfgsections()
	return { "_pass" }
end


function m.on_commit(map)
	local vW1 = pwW1:formvalue("_pass")
	local vW2 = pwW2:formvalue("_pass")
	
	if vW1 and vW2 and #vW1 > 0 and #vW2 > 0 then
		if vW1 == vW2 then
			m.uci:set("teltonika", "sys", "WiMAXpass", vW1)
			m.uci:save("teltonika")
			m.uci:commit("teltonika")
			m.message = translate("scs: Password successfully changed!")
			t = {requests = "insert", table = "EVENTS", type="Config", text="WiMAX user password has been changed"}
			eventlog:insert(t)
		else
			m.message = translate("err: Given password confirmation did not match, password not changed!")
		end
	end
	
end

return m
