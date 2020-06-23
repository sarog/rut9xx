local uci = require "uci".cursor()
local dsp = require "luci.dispatcher"
local sys = require "luci.sys"
local ut = require "luci.util"

arg[1] = arg[1] or ""

function rule_check()
	local sport = uci:get("load_balancing", arg[1], "src_port") or nil
	local dport = uci:get("load_balancing", arg[1], "dest_port") or nil
	local proto = uci:get("load_balancing", arg[1], "proto") or nil
	if proto and proto == "all" and (sport or dport) then
		return 1
	end
	return 0
end

function rule_warn(check)
	if check == 1 then
		return "wrn: No specific protocol was selected. Source and destination port will be ignored."
	else
		return nil
	end
end

function cbi_add_policy(field)
	uci:foreach("load_balancing", "policy", function (section)
		field:value(section[".name"])
	end)
end

function cbi_add_protocol(field)
	local protos = ut.trim(sys.exec("cat /etc/protocols | grep '	# ' | awk -F' ' '{print $1}' | grep -vw -e 'ip' -e 'tcp' -e 'udp' -e 'icmp' -e 'esp' | grep -v 'ipv6' | sort | tr '\n' ' '"))
	for p in string.gmatch(protos, "%S+") do
		field:value(p)
	end
end

m5 = Map("load_balancing", translate("Load Balancing Rule Configuration - ") .. arg[1])
	m5.message = (rule_warn(rule_check()) or nil)
	m5.redirect = dsp.build_url("admin", "network", "balancing", "configuration")

function valid_port_number(port)
	local nPort = tonumber(port)
	return (nPort and nPort >= 0 and nPort <= 65535)
end

function in_range(left, right)
	local nLeft = tonumber(left)
	local nRight = tonumber(right)
	return (nLeft and nRight and nLeft <= nRight and nLeft >= 0 and nRight <= 65535)
end

function validate_port(fvalue, section)
	if tonumber(fvalue) and valid_port_number(fvalue) then
		return fvalue
	elseif fvalue:find(":") and not tonumber(fvalue) then
		local splitString = fvalue:split(":")
		if splitString[1] and splitString[2] and tonumber(splitString[1]) and tonumber(splitString[2]) then
			if in_range(splitString[1], splitString[2]) then
				return fvalue
			end
		end
	else
		local correct = true
		for key, value in pairs(fvalue:split(",")) do
			if not valid_port_number(value) or not tonumber(value) then
				correct = false
			end
		end
		if correct then
			return fvalue
		end
	end
	m5.message = "err: " .. section .. " is invalid"
	return nil
end

mwan_rule = m5:section(NamedSection, arg[1], "rule", "")
	mwan_rule.addremove = false
	mwan_rule.dynamic = false


src_ip = mwan_rule:option(Value, "src_ip", translate("Source address"),
	translate("Supports CIDR notation (eg \"192.168.100.0/24\") without quotes"))
src_ip.datatype = "ipaddr"

src_port = mwan_rule:option(Value, "src_port", translate("Source port"),
	translate("May be entered as a single or multiple port(s) (eg \"22\" or \"80,443\") or as a portrange (eg \"1024:2048\") without quotes"))

	function src_port.validate(self, fvalue)
		return validate_port(fvalue, "Source port")
	end

dest_ip = mwan_rule:option(Value, "dest_ip", translate("Destination address"),
	translate("Supports CIDR notation (eg \"192.168.100.0/24\") without quotes"))
dest_ip.datatype = "ipaddr"
dest_ip.default = "0.0.0.0/0"

dest_port = mwan_rule:option(Value, "dest_port", translate("Destination port"),
	translate("May be entered as a single or multiple port(s) (eg \"22\" or \"80,443\") or as a portrange (eg \"1024:2048\") without quotes"))

	function dest_port.validate(self, fvalue)
		return validate_port(fvalue, "Destination port")
	end

proto = mwan_rule:option(Value, "proto", translate("Protocol"),
	translate("View the contents of /etc/protocols for protocol descriptions"))
	proto.default = "all"
	proto.rmempty = false
	proto:value("all")
	proto:value("ip")
	proto:value("tcp")
	proto:value("udp")
	proto:value("icmp")
	proto:value("esp")
	cbi_add_protocol(proto)

use_policy = mwan_rule:option(ListValue, "use_policy", translate("Policy assigned"))
	cbi_add_policy(use_policy)

return m5
