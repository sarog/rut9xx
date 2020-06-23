local uci = require("uci").cursor()
local utl = require("luci.util")
local network, dhcp, section

network = Map("network", translate("Step - LAN"), translate("Here we will setup the basic settings of a typical LAN configuration. The wizard will cover 2 basic configurations: static IP address LAN and DHCP client."))

-- Required for custom footer
network.wizStep = 3

section = network:section(NamedSection, "lan", "interface", translate("General configuration"))
	section.addRemove = false

local ipaddr, netmask

ipaddr = section:option(Value, "ipaddr", translate("IP address"), translate("Address that the router uses on the LAN network"))
	ipaddr.datatype = "ip4addr"

netmask = section:option(Value, "netmask", translate("Netmask"), translate("A mask used to define how large the LAN network is"))
	netmask.datatype = "ip4addr"

dhcp = Map("dhcp", translate(""), translate(""))

section = dhcp:section(NamedSection, "lan", "dhcp", translate(""), translate(""))
	section.addRemove = false
	section.hide_error = true

local ignore, start, limit, time, letter, startip, endip

ignore = section:option(Flag, "ignore", translate("Enable DHCP"), translate("Enable or disable DHCP server functionality"))
	ignore.rmempty = false
	
	function ignore.cfgvalue(self, section)
		local value = self.map:get(section, "ignore")
		if value == "1" then return "0" end
		return "1"
	end

	function ignore.write(self, section, value)
		if value == "0" then
			self.map:set(section, "ignore", "1")
			return
		end
		self.map:set(section, "ignore", "0")
	end

start = section:option(Value, "start", translate("Start"), translate("Lowest leased address as offset from the network address"))
	start.datatype = "or(uinteger,ip4addr)"
	start.default = 100
	start:depends("ignore", "1")

limit = section:option(Value, "limit", translate("Limit"), translate("Maximum number of leased addresses"))
	limit.datatype = "uinteger"
	limit.default = "150"
	limit:depends("ignore", "1")

time = section:option(Value, "time", translate("Lease time"), translate("Expire time for leased addreses. Minimum value is 2 minutes."))
	time.datatype = "uinteger"
	time.rmempty = true
	time.displayInline = true
	time.default = "12"
	time:depends("ignore", "1")

letter = section:option(ListValue, "letter", translate(""), translate(""))
	letter.rmempty = true
	letter.displayInline = true
	letter.template = "admin_network/lvalue_lease"
	letter:value("h", translate("Hours"))
	letter:value("m", translate("Minutes"))
	letter:depends("ignore", "1")

local range_start, range_end

range_start = section:option(DummyValue, "_start", translate("Start IP address:"), translate(""))
	range_start.default = "-"
	range_start.template = "cfgwzd-module/dvalue_start"
	range_start:depends("ignore", "1")

	function range_start.write(self, section, value)
		return
	end

range_end = section:option(DummyValue, "_end", translate("End IP address:"), translate(""))
	range_end.default = "-"
	range_end.template = "cfgwzd-module/dvalue_end"
	range_end:depends("ignore", "1")

	function range_end.write(self, section, value)
		return
	end

if network:formvalue("cbi.wizard.skip") then
	luci.http.redirect(luci.dispatcher.build_url("/admin/status/overview"))
	return
end

-- Because of the custom redirection, we need to do the validation ourselves
function validate_lan()
	local ipaddr = network:formvalue("cbid.network.lan.ipaddr")
	local netmask = network:formvalue("cbid.network.lan.netmask")
	if not ipaddr or #ipaddr == 0 or not netmask or #netmask == 0 then
		network.message = "err: IP address or netmask is invalid."
		return false
	end
	return true
end

function validate_leasetime()
	local leasetime = dhcp:formvalue("cbid.dhcp.lan.time")
	local letter = dhcp:formvalue("cbid.dhcp.lan.letter")
	if not letter or #letter == 0 or not leasetime or #leasetime == 0
		or (letter == "h" and tonumber(leasetime) and tonumber(leasetime) <= 0)
		or (letter == "m" and tonumber(leasetime) and tonumber(leasetime) < 2) then
		dhcp.message = "err: Lease time is invalid. Minimum is 2 minutes"
		return false
	end
	return true
end

function toBits(num)
	local bits = 8
	local t = {}
	for b = bits, 1, -1 do
		rest = math.fmod(num, 2)
		t[b] = rest
		num = (num - rest) / 2
	end
	if num == 0 then
		return table.concat(t)
	end
end

function count_zero_bytes(ipString)
	local o1, o2, o3, o4 = ipString:match("(%d+)%.(%d+)%.(%d+)%.(%d+)")
	local bytes = (toBits(tonumber(o1))) .. (toBits(tonumber(o2))) .. (toBits(tonumber(o3))) .. (toBits(tonumber(o4)))
	local count = 0
	for eachMatch in bytes:gmatch("0") do
		count = count + 1
	end
	return count
end

function validate_ip_range()
	local mask = luci.http.formvalue("cbid.network.lan.netmask") or "255.255.255.0"
	local start = dhcp:formvalue("cbid.dhcp.lan.start")
	local limit = dhcp:formvalue("cbid.dhcp.lan.limit")
	local max_range = math.pow(2, count_zero_bytes(mask))
	if not start or #start == 0 or (tonumber(start) and tonumber(start)) == 0 then
		dhcp.message = "err: Invalid start."
		return false
	elseif not limit or #limit == 0 or (tonumber(limit) and tonumber(limit)) == 0 then
		dhcp.message = "err: Invalid limit."
		return false
	elseif not max_range or (tonumber(limit) and tonumber(start) and (tonumber(limit) + tonumber(start)) > (max_range - 1)) then
		dhcp.message = "err: Invalid DHCP lease address start or limit. Out of range."
		return false
	end
	return true
end

function commit_leasetime_changes()
	local letter = luci.http.formvalue("cbid.dhcp.lan.letter") or "h"
	local time = luci.http.formvalue("cbid.dhcp.lan.time") or "12"
	uci:set("dhcp", "lan", "time", time)
	uci:set("dhcp", "lan", "letter", letter)
	uci:set("dhcp", "lan", "leasetime", time .. "" .. letter)
	uci:commit("dhcp")
end

function validate_sections()
	if not validate_lan() then
		return false
	end
	if luci.http.formvalue("cbid.dhcp.lan.ignore") == "1" then
		if not validate_leasetime() or not validate_ip_range() then
			return false
		end
	end
	return true
end

if network:formvalue("cbi.wizard.next") then
	if validate_sections() then
		commit_leasetime_changes()
		local o = section:option(DummyValue, "_dummy", translate(""), translate(""))
			o.template = "cfgwzd-module/redirect"
	else
		network.save = false
		dhcp.save = false
	end
end

return network, dhcp
