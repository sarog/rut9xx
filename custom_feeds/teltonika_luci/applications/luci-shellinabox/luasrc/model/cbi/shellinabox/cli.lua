m = Map("cli", translate("Command line interface"), 
	translate(""))
m.addremove = false
local sys = require "luci.sys"
local ut = require "luci.util"
local dsp = require "luci.dispatcher"

sc = m:section(NamedSection, "status","status", translate("CLI Configuration"))
enb_block = sc:option(Flag, "enable", translate("Enable"), translate(""))
enb_block.rmempty = false
pt = sc:option(Value, "port", translate("Port"), translate("Port to listen for cli access."))
pt.datatype = "port"
function pt:validate(Values)
	local old_port = ut.trim(sys.exec("uci get cli.status.port"))
	local flag = m:formvalue("cbid.cli.status.enable")
	local old_flag = ut.trim(sys.exec("uci get cli.status.enable"))
	local restarted = false
	
	if Values ~= old_port then
		if ut.trim(sys.exec("netstat -ln | grep ':".. Values .." ' | grep 'LISTEN'")) == "" then
			return Values
		else
			m.message = translate("err: This port is already in use!")
		end
	else
		return Values
	end
end

return m
