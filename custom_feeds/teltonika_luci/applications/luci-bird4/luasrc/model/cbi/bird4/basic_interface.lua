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


local net_area = string.match(string.match(VPN_INST, "_%d+"), "%d+")
-- local net_area = getParam("uci get bird4.\"".. VPN_INST .."\".area")
-- os.execute("logger \"net_area == ".. net_area .."\"")
local m = Map("bird4", translatef("Interface: %s", VPN_INST:gsub("^%l", string.upper)), "")
m.redirect = dsp.build_url("admin/network/routes/dynamic_routes/basic",net_area)
if m.uci:get("bird4", arg[1]) ~= "ospf_interface" then
	luci.http.redirect(dsp.build_url("admin/network/routes/dynamic_routes/basic",net_area))
  	return
end


local interface = m:section( NamedSection, VPN_INST, "ospf_interface", translate("Main Settings"), "")
interface.anonymous = true
interface.addremove = false



cost = interface:option(Value, "cost", translate("Cost"), translate(""))
cost.datatype = "range(0,500)"
cost.default = "10"

hello = interface:option(Value, "hello", translate("Hello"), translate(""))
hello.datatype = "range(0,50)"
hello.default = "10"

poll = interface:option(Value, "poll", translate("Poll"), translate(""))
poll.datatype = "range(0,500)"
poll.default = "20"

retransmit = interface:option(Value, "retransmit", translate("Retransmit"), translate(""))
retransmit.datatype = "range(0,500)"
retransmit.default = "5"

priority = interface:option(Value, "priority", translate("Priority"), translate(""))
priority.datatype = "range(0,500)"
priority.default = "1"

wait = interface:option(Value, "wait", translate("Wait"), translate(""))
wait.datatype = "range(0,500)"
wait.default = "40"

dead_c = interface:option(Value, "dead count", translate("Dead count"), translate(""))
dead_c.datatype = "range(0,500)"
dead_c.default = "3"

dead = interface:option(Value, "dead", translate("Dead"), translate(""))
dead.datatype = "range(0,500)"
dead.default = "30"

rx = interface:option(ListValue, "rx buffer", translate("RX buffer"), translate(""))
rx:value("normal", translate("Normal"))
rx:value("large", translate("Large"))
rx.default = "normal"

tx = interface:option(Value, "tx length", translate("TX length"), translate(""))
tx.datatype = "range(0,500)"
tx.default = "100"

types = interface:option(ListValue, "type", translate("Type"), translate(""))
types:value("broadcast", translate("Broadcast"))
types:value("pointopoint", translate("Pointopoint"))
types:value("nonbroadcast", translate("Nonbroadcast"))
types:value("pointomultipoint", translate("Pointomultipoint"))
types:value("bcast", translate("bcast"))
types:value("ptp", translate("PTP"))
types:value("nbma", translate("NBMA"))
types:value("ptmp", translate("PTMP"))

auth = interface:option(ListValue, "authentication", translate("Authentication"), translate(""))
auth:value("none", translate("None"))
auth:value("simple", translate("Simple"))
auth:value("cryptographic", translate("Cryptographic"))

pass = interface:option(Value, "password", translate("Password"), translate(""))
pass:depends({authentication="simple"})
pass:depends({authentication="cryptographic"})
pass.password = true

return m
