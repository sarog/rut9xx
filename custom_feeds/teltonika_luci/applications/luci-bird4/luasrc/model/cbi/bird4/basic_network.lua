local sys = require "luci.sys"
local dsp = require "luci.dispatcher"
local utl = require "luci.util"
local uci = require "luci.model.uci".cursor()

local VPN_INST, TMODE


if arg[1] then
	VPN_INST = arg[1]
else
	return nil
end

function getParam(string)
		local h = io.popen(string)
		local t = h:read()
		h:close()
		return t
	end
local mode, o

function  split_by_word(config, section, option, order)
	local uci_l = require "luci.model.uci".cursor()
	local values={}
	if config and section and option and order then
		local val = uci_l:get(config, section, option)
		if val then
			for v in val:gmatch("[%w+%.]+") do 
				table.insert(values, v) 
			end
		end
	end
	
	return values[order]
end

local net_ip = getParam("uci get bird4.\"".. VPN_INST .."\".ip")
local net_area = getParam("uci get bird4.\"".. VPN_INST .."\".area")
m = Map("bird4", translatef("Network: %s", net_ip:gsub("^%l", string.upper)), "")
m.redirect = dsp.build_url("admin/network/routes/dynamic_routes/basic",net_area)
if m.uci:get("bird4", arg[1]) ~= "ospf_network" then
	luci.http.redirect(dsp.build_url("admin/network/routes/dynamic_routes/basic",net_area))
  	return
end


local sub = m:section( NamedSection, VPN_INST, "ospf_network", translate("Main Settings"), "")
sub.anonymous = true
sub.addremove = false


hid = sub:option(Flag, "hidden", translate("Hidden"), translate(""))
hid.rmempty = false
hid.default = "0"

sum = sub:option(Flag, "summary", translate("Summary"), translate(""))
sum.default = "0"

cost = sub:option(Value, "cost", translate("Cost"), translate(""))
cost.datatype = "range(0,500)"
cost.default = "101"

return m
