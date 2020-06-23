local m, o, s_config, s_devices, mac, pw, name, wake, wake_all

local ds = require "luci.dispatcher"
local uci = require "luci.model.uci".cursor()
local utl = require "luci.util"
local sys = require "luci.sys"
local fs  = require "nixio.fs"

require("teltonika_lua_functions")

local has_ewk = fs.access("/usr/bin/etherwake")
local has_wol = fs.access("/usr/bin/wol")

m = Map("etherwake", translate("Wake on LAN"))

if has_ewk and has_wol then
	bin = s_config:option(ListValue, "binary", translate("WoL program"),
		translate("Sometimes only one of both tools work. If one of fails, try the other one"))
	bin:value("/usr/bin/etherwake", translate("Etherwake"))
	bin:value("/usr/bin/wol", translate("WoL"))
end

s_config = m:section(NamedSection, "setup", "etherwake", translate("Wake on LAN configuration"))
s_config.anonymous = true
s_config.addremove = false

function s_config:filter(value)
  return value ~= "pathes" and value
end

o = s_config:option(Flag, "broadcast", translate("Broadcast"), translate("Send to broadcast address."))
wake_all = s_config:option(Button, "wake-all", " ")
wake_all.inputtitle = translate("Wake all devices")

function wake_all.write(self, section, value)
	local message = "scs: WOL Packets sent."
	uci:foreach("etherwake", "target", function(s)
		sys.exec("/etc/init.d/etherwake start " .. escape_shell(s["name"]))
	end)
end

s_devices = m:section(TypedSection, "target", translate("Devices to wake up"))
s_devices.template    = "cbi/tblsection"
s_devices.addremove   = true
s_devices.anonymous   = true
s_devices.novaluetext = translate("There are no devices configured.")
s_devices.hide_error  = true

name = s_devices:option(Value, "name", translate("Name"), translate("Name of the device."))
name.default = "Device_name"

mac = s_devices:option(Value, "mac", translate("MAC"), translate("MAC address of the device you want to wake up."))
mac.default = "11:22:33:44:55:66"

pw = s_devices:option(Value, "password", translate("Password"), translate("Send given SecureON password when waking the host. Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:;[]"))
pw.datatype = "password"

o = s_devices:option(Flag, "wakeonboot", translate("Wake on Boot"), translate("Send WOL packet upon bootup process."))

wake = s_devices:option(Button, "wake", translate("Wake device"))

name.rmempty = false
mac.rmempty = false

function name.validate(self, value, section)
	local temporary = value
	if not value or #value == 0 then
		m.message = "err: \'Name\' is a required field."
		return nil
	elseif string.match(temporary:gsub('_',''), "%W") then
		m.message = "err: \'Name\' contains invalid characters. (only alphanumeric and underscores are allowed)"
		return nil
	end
	return value
end

function mac.validate(self, value, section)
	if value and #value == 17 and string.match(value, "%x+:%x+:%x+:%x+:%x+:%x+") then
		return value
	else
		m.message = "err: \'" .. (value or "") .. "\' is an invalid MAC address. (Format is AA:00:04:00:BB:FF)"
		return nil
	end
end

-- Wake a single device
function wake.write(self, section, value)
	local target = luci.http.formvalue("cbid.etherwake." .. section .. ".name")
	local macaddr = luci.http.formvalue("cbid.etherwake." .. section .. ".mac")
	local message
	if target and macaddr then
		message = "scs: WOL Packet sent."
		sys.exec("/etc/init.d/etherwake start " .. escape_shell(target))
	else 
		message = "err: No target or MAC address specified"
	end
	m.message = message
end

return m
