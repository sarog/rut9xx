local ds = require "luci.dispatcher"
local ft = require "luci.tools.gps"
local fs = require "nixio.fs"

map = Map("gps")

--[[
--  GPS AVL settings section
--]]
avl_settings = map:section(NamedSection, "avl", nil, translate("AVL Server Settings"))

enabled = avl_settings:option(Flag, "enabled",
			      translate("Enabled"),
			      translate("Enable NMEA forwarding to remote server in AVL protocol"))
enabled.rmempty = false

hostname = avl_settings:option(Value, "hostname",
			      translate("Hostname"),
			      translate("Hostname of the remote server"))
hostname.rmempty = false
hostname.datatype = "or (hostname, ipaddr)"

port = avl_settings:option(Value, "port",
			      translate("Port"),
			      translate("Port of the remote server"))
port.rmempty = false
port.datatype = "port"

proto = avl_settings:option(ListValue, "proto",
			      translate("Protocol"),
			      translate("Protocol to be used for sending AVL to the remote server"))
proto.rmempty = false
proto:value("tcp", "TCP")
proto:value("udp", "UDP")

connection_contain = avl_settings:option(Flag, "con_cont",
                                          translate("Don't Contain Connection"),
                                          translate("Handle each AVL packet iteration as a new connection"))

connection_contain.rmempty = false

--[[
--  AVL MAIN rule section
--]]
main_rule = map:section(TypedSection, "avl_rule_main", translate("Main Rule"))
main_rule.template 	= "cbi/tblsection"
main_rule.anonymous	= true
main_rule.extedit	= ds.build_url("admin/services/gps/avl/%s")

function main_rule.cfgsections(self)
        local sections = {}

        self.map.uci:foreach(self.map.config, "section",
                function (section)
                        name = self:checkscope(section[".name"])
                        if name and name:match("avl_rule_main")  then
                                table.insert(sections, section[".name"])
                        end
                end)

        return sections
end

src = main_rule:option(DummyValue, "priority", translate("Rule priority"), translate("Specifies main rule priority"))
src.rawhtml = true
src.width   = "10%"
function src.cfgvalue(self, s)
        local z = self.map:get(s, "priority")
        if z == "low" then
                return translatef("Low")
        elseif z == "high" then
                return translatef("High")
	elseif z == "panic" then
		return translatef("Panic")
	elseif z == "security" then
		return translatef("Security")
        else
            return translatef("N/A")
        end
end

src = main_rule:option(DummyValue, "collect_period", translate("Collect period"), translate("Period (in seconds) for data collection"))
src.rawhtml = true
src.width   = "10%"

src = main_rule:option(DummyValue, "saved_records", translate("Min saved records"), translate("Minimal amount of coordinates registered, to send them to server immediately (even if  Send period have not passed yet)"))
src.rawhtml = true
src.width   = "10%"

src = main_rule:option(DummyValue, "send_period", translate("Send period"), translate("Period (in seconds) for sending collected data to server"))
src.rawhtml = true
src.width   = "10%"

ft.opt_enabled(main_rule, Flag, translate("Enable"), translate("Check to enable this gps rule")).width = "18%"

--[[
--  AVL Rules section
--]]
rules = map:section(TypedSection, "avl_rule", translate("Secondary Rules"))
rules.template 	= "cbi/tblsection"
rules.addremove = true
rules.anonymous	= true
rules.sortable 	= true
rules.extedit	= ds.build_url("admin/services/gps/avl/%s")
rules.template_addremove = "gps/cbi_add_gps_rule"
rules.novaluetext = translate("There are no gps rules created yet")

function rules.create(self, section)
	local wan = map:formvalue("_newgps.wan_status")
	local din = map:formvalue("_newgps.din_status")
	local priority = map:formvalue("_newgps.priority")

	created = TypedSection.create(self, section)
	self.map:set(created, "wan_status", wan)
	self.map:set(created, "din_status", din)
	self.map:set(created, "priority", priority)

	self.map:set(created, "enabled", "0")
	self.map:set(created, "collect_period", "25")
	self.map:set(created, "distance", "200")
	self.map:set(created, "angle", "30")
	self.map:set(created, "saved_records", "20")
	self.map:set(created, "send_period", "50")
end

function rules.parse(self, ...)
        TypedSection.parse(self, ...)
        if created then
                map.uci:save("gps")
                luci.http.redirect(ds.build_url("admin/services/gps/avl", created))
        end
end

src = rules:option(DummyValue, "wan", translate("Wan"), translate("Specifies wich WAN needs to be in use for this configuration to apply"))
src.rawhtml = true
src.width   = "10%"
function src.cfgvalue(self, s)
	local z = self.map:get(s, "wan_status")
	
	if not z then
		return translatef("N/A")
	end

        if string.find(z, "mobile") then
                return translatef("Mobile")
        elseif string.find(z, "wired") then
                return translatef("Wired")
        elseif string.find(z, "wifi") then
                return translatef("WiFi")
        else
            return translatef("N/A")
        end
end

src = rules:option(DummyValue, "type", translate("Type"), translate("Specifies type/state of WAN which is needed for configuration to apply"))
src.rawhtml = true
src.width   = "10%"
function src.cfgvalue(self, s)
        local z = self.map:get(s, "wan_status")

	if not z then
		return translatef("N/A")
	end

        if string.find(z, "home") then
                return translatef("Home")
        elseif string.find(z, "roaming") then
                return translatef("Roaming")
        elseif string.find(z, "both") then
                return translatef("Both")
        else
            return translatef("-")
        end
end

src = rules:option(DummyValue, "din_status", translate("Digital isolated input"), translate("Specifies digital isolated input state which is needed for configuration to apply"))
src.rawhtml = true
src.width   = "10%"
function src.cfgvalue(self, s)
        local z = self.map:get(s, "din_status")
        if z == "low" then
                return translatef("Low")
        elseif z == "high" then
                return translatef("High")
        elseif z == "both" then
                return translatef("Both")
        else
            return translatef("N/A")
        end
end

src = rules:option(DummyValue, "priority", translate("Rule priority"), translate("Specifies new rule priority"))
src.rawhtml = true
src.width   = "10%"
function src.cfgvalue(self, s)
        local z = self.map:get(s, "priority")
        if z == "low" then
                return translatef("Low")
        elseif z == "high" then
                return translatef("High")
	elseif z == "panic" then
		return translatef("Panic")
	elseif z == "security" then
		return translatef("Security")
        else
            return translatef("N/A")
        end
end

src = rules:option(DummyValue, "collect_period", translate("Collect period"), translate("Period (in seconds) for data collection"))
src.rawhtml = true
src.width   = "10%"

src = rules:option(DummyValue, "saved_records", translate("Min saved records"), translate("Minimal amount of coordinates registered, to send them to server immediately (even if  Send period have not passed yet)"))
src.rawhtml = true
src.width   = "10%"

src = rules:option(DummyValue, "send_period", translate("Send period"), translate("Period (in seconds) for sending collected data to server"))
src.rawhtml = true
src.width   = "10%"


ft.opt_enabled(rules, Flag, translate("Enable"), translate("Check to enable this gps rule")).width = "18%"

local save = map:formvalue("cbi.apply")
if save then
        --Delete all usr_enable from gps config
        map.uci:foreach("gps", "avl_rule", function(s)
                gps_inst = s[".name"] or ""
                gpsEnable = map:formvalue("cbid.gps." .. gps_inst .. ".enabled") or "0"
                gps_enable = s.enabled or "0"
                if gpsEnable ~= gps_enable then
                        map.uci:foreach("gps", "avl_rule", function(a)
                                gps_inst2 = a[".name"] or ""
                                local usr_enable = a.usr_enable or ""
                                if usr_enable == "1" then
                                        map.uci:delete("gps", gps_inst2, "usr_enable")
                                end
                        end)
                end
        end)
        map.uci:save("gps")
        map.uci.commit("gps")
end

--[[
--  TAVL settings section
--]]

local cfg_in_out = map.uci:get("hwinfo", "hwinfo", "in_out") or "0"

tavl = map:section(TypedSection, "tavl", translate("TAVL Settings"))
o = tavl:option(Flag, "send_gsm_signal", "Send GSM signal", translate("Check to include GSM signal strength information in GPS data package to be sent to server"))

if cfg_in_out == "1" then
	o = tavl:option(Flag, "send_analog_input", "Send analog input", translate("Check to include analog input state in GPS data package to be sent to server"))
	o = tavl:option(Flag, "send_digital_input1", translatef("Send digital input (%d)", 1), translate("Check to include digital input #1 state in GPS data package to be sent to server"))
	o = tavl:option(Flag, "send_digital_input2", translatef("Send digital input (%d)", 2), translate("Check to include digital input #2 state in GPS data package to be sent to server"))
end

--[[
-- Only for facelift
--]]
if fs.access("/sys/bus/i2c/devices/0-0074/gpio") == nil then
	o = tavl:option(Flag, "send_digital_input3", translatef("Send digital input (%d)", 3), translate("Check to include digital input #3 state in GPS data package to be sent to server"))
end

return map
