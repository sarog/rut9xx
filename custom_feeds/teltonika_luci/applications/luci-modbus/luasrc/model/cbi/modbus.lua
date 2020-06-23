local fw = require "luci.model.firewall"
local sys = require "luci.sys"
local uci = require("uci").cursor()

local deathTrap = { }

function parseFunctionFactory(flag1, flag2, errstr)
	return function(self, section, novld)
		local b1 = flag1 == nil or flag1:formvalue(section) == "1"
		local b2 = flag2 == nil or flag2:formvalue(section) == "1"
		local v = self:formvalue(section)
		if b1 and b2 and (v == nil or v == "") then
			self:add_error(section, "invalid", errstr)
		end
		Value.parse(self, section, novld)
	end
end

m = Map("modbus", translate("Modbus TCP slave"))
m:chain("firewall")
fw.init(m.uci)

s = m:section(NamedSection, "modbus", "modbus", translate(""), "")

enabled = s:option(Flag, "enabled", "Enable", translate("Enable Modbus TCP"))
enabled.default = "0"
enabled.rmempty = false

port = s:option(Value, "port", "Port", translate("Port number"))
port.datatype = "port"
port.default = "502"
port.parse = parseFunctionFactory(enabled, nil, "Port cannot be empty")

dev_id = s:option(Value, "device_id", "Device ID", translate("Modbus slave ID that this device will respond to (1-255, set 0 to respond to any ID)"))
dev_id.datatype = "range(0,255)"
dev_id.default = "1"
dev_id.parse = parseFunctionFactory(enabled, nil, "Device ID cannot be empty")

allow_ra = s:option(Flag, "allow_ra", "Allow Remote Access", translate("Allow access through WAN"))
allow_ra.default = "0"
allow_ra.rmempty = false

function allow_ra.write(self, section)
	local fval = self:formvalue(section)
	local fport = port:formvalue(section)
	local needsPortUpdate = false
	local fwRuleInstName = "nil"

	if not deathTrap[1] then
		deathTrap[1] = true
	else
		return
	end

	fval = fval and "1" or "0"

	m.uci:foreach("firewall", "rule", function(z)
		if z.name == "Enable_MODBUSD_WAN" then
			fwRuleInstName = z[".name"]
			if z.dest_port ~= fport then
				needsPortUpdate = true
			end
			if z.enabled ~= fval then
				needsPortUpdate = true
			end
		end
	end)

	if needsPortUpdate == true then
		m.uci:set("firewall", fwRuleInstName, "dest_port", fport)
		m.uci:set("firewall", fwRuleInstName, "enabled", fval)
		m.uci:save("firewall")
	end

	if fwRuleInstName == "nil" then
		local wanZone = fw:get_zone("wan")
		if not wanZone then
			m.message = "Could not add firewall rule"
			return
		end
		local fw_rule = {
			name = "Enable_MODBUSD_WAN",
			target = "ACCEPT",
			proto = "tcp",
			dest_port = fport,
			enabled = fval
		}
		wanZone:add_rule(fw_rule)
		m.uci:save("firewall")
	end
end

function allow_ra.cfgvalue(self, section)
	local fwRuleEn = false

	m.uci:foreach("firewall", "rule", function(z)
		if z.name == "Enable_MODBUSD_WAN" and z.enabled == "1" then
			fwRuleEn = true
		end
	end)

	return fwRuleEn and self.enabled or self.disabled
end

enable_custom_register_block = s:option(Flag, "clientregs", "Enable custom register block", translate("Allow custom register block"))
enable_custom_register_block.default = "0"
enable_custom_register_block.rmempty = false

custom_register_block_file = s:option(Value, "regfile", "Register file path", translate("Path to file in which the custom register block will be stored. Files inside /tmp or /var are stored in RAM. They vanish after reboot, but do not degrade flash memory. Files elsewhere are stored in flash memory. They remain after reboot, but degrade flash memory (severely, if operations are frequent)."))
custom_register_block_file.default = "/tmp/regfile"
custom_register_block_file:depends("clientregs", "1")
custom_register_block_file.parse = parseFunctionFactory(enabled, enable_custom_register_block, "Register file path cannot be empty")

custom_register_block_start = s:option(Value, "regfilestart", "First register number", translate("First register in custom register block (1025-65536)"))
custom_register_block_start.default = "1025"
custom_register_block_start.datatype = "range(1025, 65536)"
custom_register_block_start:depends("clientregs", "1")
custom_register_block_start.parse = parseFunctionFactory(enabled, enable_custom_register_block, "First register number cannot be empty")

custom_register_block_size = s:option(Value, "regfilesize", "Register count", translate("Register count in custom register block (1-64512)"))
custom_register_block_size.default = "128"
custom_register_block_size.datatype = "range(1, 64512)"
custom_register_block_size:depends("clientregs", "1")
custom_register_block_size.parse = parseFunctionFactory(enabled, enable_custom_register_block, "Register count cannot be empty")

return m
