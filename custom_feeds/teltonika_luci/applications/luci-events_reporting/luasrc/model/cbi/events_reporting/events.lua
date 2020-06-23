--[[

LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: forwards.lua 8117 2011-12-20 03:14:54Z jow $
]]--

local ds = require "luci.dispatcher"
local utl = require "luci.util"

m = Map("events_reporting", translate("Events Reporting"))
--translate("Create rules for events reporting.")

--
-- Port Forwards
--

s = m:section(TypedSection, "rule", translate("Events Reporting Rules"))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.sortable  = true
s.extedit   = ds.build_url("admin/status/event/report/%s")
s.template_addremove = "events_reporting/cbi_addinput"
s.novaluetext = translate("There are no events reporting rules created yet")

function s.create(self, section)
	local e = m:formvalue("_newinput.event")
	local em = m:formvalue("_newinput.eventMark")
	local a = m:formvalue("_newinput.action")

	created = TypedSection.create(self, section)
	self.map:set(created, "event", e)
	self.map:set(created, "eventMark", em)
	self.map:set(created, "action", a)
end

function s.filter(self, sid)
	return (self.map:get(sid, "action") ~= "sendRMS" and self.map:get(sid, "action") ~= "sendRMS_SMTP")
end

function s.parse(self, ...)
	local delButtonFormString = "cbi.rts." .. self.config .. "."
	local delButtonPress = false
	local configName

	m.uci:foreach(self.config, "rule", function(x)
		if not delButtonPress then
			configName = x[".name"] or ""
			if luci.http.formvalue(delButtonFormString .. configName) then
				delButtonPress = true
			end
		end
	end)

	if delButtonPress then
		m.uci.delete(self.config, configName)
	end

	TypedSection.parse(self, ...)
	if created then
		m.uci:save(self.config)
		luci.http.redirect(ds.build_url("admin/status/event/report", created))
	else
		m.uci:save(self.config)
		m.uci.commit(self.config)
	end
end

src = s:option(DummyValue, "event", translate("Event type"),
	translate("Event type for which the rule is applied"))
src.rawhtml = true
src.width = "16%"

function src.cfgvalue(self, s)
	local z = self.map:get(s, "event")
	if z == "Config" then
		return translatef("Config change")
	elseif z == "DHCP" then
		return translatef("New DHCP client")
	--elseif z == "FW" then
	--	return translatef("FW upgrade")
	elseif z == "Mobile Data" then
		return translatef("Mobile data")
	elseif z == "Reboot" then
		return translatef("Reboot")
	elseif z == "SMS" then
		return translatef("SMS")
	elseif z == "SSH" then
		return translatef("SSH")
	elseif z == "Web UI" then
		return translatef("Web UI")
	elseif z == "WiFi" then
		return translatef("New WiFi client")
	elseif z == "SIM switch" then
		return translatef("SIM switch")
	elseif z == "Signal strength" then
		return translatef("Signal strength")
	elseif z == "GPS" then
		return translatef("GPS")
	elseif z == "Port" then
		return translatef("LAN port state")
	elseif z == "Backup" then
		return translatef("WAN Failover")
	elseif z == "Restore Point" then
		return translatef("Restore Point")
	else
	    return translatef("NA")
	end
end

src = s:option(DummyValue, "eventMark", translate("Event subtype"),
	translate("Event subtype for which the rule is applied"))
src.rawhtml = true
src.width = "20%"

function src.cfgvalue(self, s)
	local t = self.map:get(s, "eventMark")
	local z = self.map:get(s, "event")
	if t == "all" then
		return translatef("All")
-- Config
--	elseif t == "restore point" and z == "Config" then
--		return translatef("Restore point")
	elseif t == "open vpn" and z == "Config" then
		return translatef("OpenVPN")
	elseif t == "sms" and z == "Config" then
		return translatef("SMS")
	elseif t == "events reporting" and z == "Config" then
		return translatef("Events reporting")
	elseif t == "gps" and z == "Config" then
		return translatef("GPS")
	elseif t == "periodic reboot" and z == "Config" then
		return translatef("Periodic reboot")
	elseif t == "snmp" and z == "Config" then
		return translatef("SNMP")
	elseif t == "ping reboot" and z == "Config" then
		return translatef("Ping reboot")
	elseif t == "auto update" and z == "Config" then
		return translatef("Auto update")
	elseif t == "site blocking" and z == "Config" then
		return translatef("Site blocking")
	elseif t == "pptp" and z == "Config" then
		return translatef("PPTP")
	-- elseif t == "administration" and z == "Config" then
	-- 	return translatef("Administration")
	elseif t == "hotspot" and z == "Config" then
		return translatef("Hotspot")
	elseif t == "input/output" and z == "Config" then
		return translatef("Input/output")
	elseif t == "content blocker" and z == "Config" then
		return translatef("Content blocker")
	elseif t == "login page" and z == "Config" then
		return translatef("Login page")
	elseif t == "data limit" and z == "Config" then
		return translatef("Data limit")
	elseif t == "language" and z == "Config" then
	     	return translatef("Language")
	elseif t == "profile" and z == "Config" then
		return translatef("Profile")
	elseif t == "ddns" and z == "Config" then
		return translatef("DDNS")
	elseif t == "mobile traffic" and z == "Config" then
		return translatef("Mobile traffic")
	elseif t == "ipsec" and z == "Config" then
		return translatef("IPsec")
	elseif t == "access control" and z == "Config" then
		return translatef("Access control")
	elseif t == "dhcp" and z == "Config" then
		return translatef("DHCP")
	elseif t == "multiwan" and z == "Config" then
		return translatef("Multiwan")
	elseif t == "rs232/rs485" and z == "Config" then
		return translatef("RS232/RS485")
	elseif t == "vrrp" and z == "Config" then
		return translatef("VRRP")
	elseif t == "ssh" and z == "Config" then
		return translatef("SSH")
	elseif t == "network" and z == "Config" then
		return translatef("Network")
	elseif t == "sim switch" and z == "Config" then
		return translatef("SIM switch")
	elseif t == "wireless" and z == "Config" then
		return translatef("Wireless")
	elseif t == "firewall" and z == "Config" then
		return translatef("Firewall")
	elseif t == "ntp" and z == "Config" then
		return translatef("NTP")
	elseif t == "mobile" and z == "Config" then
		return translatef("Mobile")
	elseif t == "l2tp" and z == "Config" then
		return translatef("L2TP")
	elseif t == "other" and z == "Config" then
		return translatef("Other")
-- DHCP
	elseif t == "wifi" and z == "DHCP" then
		return translatef("Connected from WiFi")
	elseif t == "lan" and z == "DHCP" then
		return translatef("Connected from LAN")
-- FirstBoot
	elseif t == "start up" and z == "FirstBoot" then
		return translatef("Start up")
-- FW upgrade
--	elseif t == "file" and z == "FW" then
--		return translatef("From file")
--	elseif t == "server" and z == "FW" then
--		return translatef("From server")
-- Mobile Data
	elseif t == " connected" and z == "Mobile Data" then
		return translatef("Connected")
	elseif t == "disconnected" and z == "Mobile Data" then
		return translatef("Disconnected")
-- Reboot
	elseif t == "Boot start up, reason unknown" and z == "Reboot" then
		return translatef("After unexpected shut down")
	elseif t == "fw upgrade" and z == "Reboot" then
		return translatef("After FW upgrade")
	elseif t == "web ui" and z == "Reboot" then
		return translatef("From Web UI")
	elseif t == "sms" and z == "Reboot" then
		return translatef("From SMS")
	elseif t == "call" and z == "Reboot" then
		return translatef("From Call")
	elseif t == "input/output" and z == "Reboot" then
	       	return translatef("From input/output")
	elseif t == "ping reboot" and z == "Reboot" then
		return translatef("From ping reboot")
	elseif t == "periodic reboot" and z == "Reboot" then
		return translatef("From periodic reboot")
	elseif t == "from button" and z == "Reboot" then
		return translatef("From button")
-- 	elseif t == "factory reset button" and z == "Reboot" then
-- 		return translatef("After factory reset button")
-- 	elseif t == "restore" and z == "Reboot" then
-- 		return translatef("After restore")
-- SMS
-- 	elseif t == "sent" and z == "SMS" then
-- 		return translatef("SMS sent")
	elseif t == "received from" and z == "SMS" then
		return translatef("SMS received")
-- SSH
	elseif t == "succeeded" and z == "SSH" then
		return translatef("Successful authentication")
	elseif t == "bad" and z == "SSH" then
		return translatef("Unsuccessful authentication")
-- Web UI
	elseif t == "was successful" and z == "Web UI" then
		return translatef("Successful authentication")
	elseif t == "not successful" and z == "Web UI" then
		return translatef("Unsuccessful authentication")
-- WiFi
	elseif t == "connected" and z == "WiFi" then
		return translatef("Connected")
	elseif t == "disconnected" and z == "WiFi" then
		return translatef("Disconnected")
-- SIM switch
	elseif t == "SIM 1 to SIM 2" and z == "SIM switch" then
		return translatef("From SIM1 to SIM2")
	elseif t == "SIM 2 to SIM 1" and z == "SIM switch" then
		return translatef("From SIM2 to SIM1")
-- GPS
	elseif t == "left geofence" and z == "GPS" then
		return translatef("Left geofence")
	elseif t == "entered geofence" and z == "GPS" then
		return translatef("Entered geofence")
-- Signal strength
	elseif t == "Signal strength dropped below -113 dBm" and z == "Signal strength" then
		return translatef("Signal strength dropped below -113 dBm")
	elseif t == "Signal strength dropped below -98 dBm" and z == "Signal strength" then
		return translatef("Signal strength dropped below -98 dBm")
	elseif t == "Signal strength dropped below -93 dBm" and z == "Signal strength" then
		return translatef("Signal strength dropped below -93 dBm")
	elseif t == "Signal strength dropped below -75 dBm" and z == "Signal strength" then
		return translatef("Signal strength dropped below -75 dBm")
	elseif t == "Signal strength dropped below -60 dBm" and z == "Signal strength" then
		return translatef("Signal strength dropped below -60 dBm")
	elseif t == "Signal strength dropped below -50 dBm" and z == "Signal strength" then
		return translatef("Signal strength dropped below -50 dBm")
-- LAN Port cheker
	elseif t == "unplugged" and z == "Port" then
		return translatef("Unplugged")
	elseif t == "plugged in" and z == "Port" then
		return translatef("Plugged in")
--Backup
	elseif t == "main" and z == "Backup" then
		return translatef("Switched to main")
	elseif t == "backup" and z == "Backup" then
		return translatef("Switched to failover")
	elseif t == "download" and z == "Restore Point" then
		return translatef("Save")
	elseif t == "restore" and z == "Restore Point" then
		return translatef("Restore")
	else
		return translatef("N/A")
	end
end

src = s:option(DummyValue, "action", translate("Action"), translate("Action to perform when an event occurs"))
src.rawhtml = true
src.width = "18%"

function src.cfgvalue(self, s)
	local z = self.map:get(s, "action")
	if z == "sendSMS" then
		return translatef("Send SMS")
	elseif z == "sendEmail" then
		return translatef("Send email")
	else
	    return translatef("NA")
	end
end

en = s:option(Flag, "enable", translate("Enable"), translate("Make a rule active/inactive"))
en.rmempty = false
en.width = "18%"

return m
