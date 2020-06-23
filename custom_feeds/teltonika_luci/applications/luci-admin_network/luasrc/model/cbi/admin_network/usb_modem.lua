--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: ntpc.lua 6065 2010-04-14 11:36:13Z ben $
]]--

require("luci.fs")
require("luci.config")
require "teltonika_lua_functions"
local utl = require "luci.util"
local nw = require "luci.model.network"
local sys = require "luci.sys"
local m


	m = Map("network", translate("USB Modem Configuration"), translate(""))
	m.addremove = false

	s = m:section(NamedSection, "ppp_usb", translate("Mobile configuration"))

	
	o = s:option(Value, "apn", translate("APN"), translate("APN (Access Point Name) is a configurable network identifier used by a mobile device when connecting to a GSM carrier"))
	o = s:option(Value, "pincode", translate("PIN number"), translate("SIM card\\'s PIN (Personal Identification Number) is a secret numeric password shared between a user and a system that can be used to authenticate the user"))
	
	auth = s:option(ListValue, "auth_mode", translate("Authentication method"), translate("Authentication method that your GSM carrier uses to authenticate new connections on it\\'s network"))
		auth:value("chap", translate("CHAP"))
		auth:value("pap", translate("PAP"))
		auth:value("none", translate("None"))
		auth.default = "none"
		
	o = s:option(Value, "username", translate("Username"), translate("Your username that you would use to connect to your GSM carrier\\'s network"))
		o:depends("auth_mode", "chap")
		o:depends("auth_mode", "pap")
		o.default = "admin"

	o = s:option(Value, "password", translate("Password"), translate("Your password that you would use to connect to your GSM carrier\\'s network"))
		o:depends("auth_mode", "chap")
		o:depends("auth_mode", "pap")
		o.default = "admin01"
		o.password = true;
		
	o = s:option(ListValue, "service", translate("Service mode"), translate("Your network\\'s preference. If your local mobile network supports GSM (2G), UMTS (3G) or LTE (4G) you can specify to which network you prefer to connect to"))
		o:value("gprs-only", translate("2G only"))
		o:value("gprs", translate("2G preferred"))
		o:value("umts-only", translate("3G only"))
		o:value("umts", translate("3G preferred"))
		o:value("lte-only", translate("4G (LTE) only"))
		o:value("lte", translate("4G (LTE) preferred"))
		o:value("auto", translate("Automatic"))
		o.default = "lte"

	--o = s:option(Flag, "roaming", translate("Deny data roaming"), translate("Deny data connection on roaming"))
	--o = s:option(Flag, "pdptype", translate("Use IPv4 only"), translate("Specifies the type of packet data protocol"))

		
		
	
	
	
	
	return m

