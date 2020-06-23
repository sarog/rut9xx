--[[
LuCI - Lua Configuration Interface

Copyright 2009 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: p2pblock.lua 5448 2009-10-31 15:54:11Z jow $
]]--

local sys = require "luci.sys"

m = Map("freifunk_p2pblock", translate("P2P-Block"),
	translate("P2P-Block is a greylisting mechanism to block various peer-to-peer protocols for non-whitelisted clients."))

s = m:section(NamedSection, "p2pblock", "settings", translate("Settings"))
s.anonymous = true
s.addremove = false

en = s:option(Flag, "_enabled", translate("Enable P2P-Block"))
en.rmempty = false

function en.cfgvalue()
	return ( sys.init.enabled("freifunk-p2pblock") and "1" or "0" )
end

function en.write(self, section, val)
	if val == "1" then
		sys.init.enable("freifunk-p2pblock")
	else
		sys.init.disable("freifunk-p2pblock")
	end
end

s:option(Value, "portrange", translate("Portrange"))

s:option(Value, "blocktime", translate("Block Time"),
	translate("seconds"))

s:option(DynamicList, "whitelist", translate("Whitelisted IPs"))

l7 = s:option(MultiValue, "layer7", translate("Layer7-Protocols"))
l7.widget = "checkbox"
l7:value("aim", translate("AIM Chat"))
l7:value("bittorrent", translate("Bittorrent"))
l7:value("edonkey", translate("eDonkey, eMule, Kademlia"))
l7:value("fasttrack", translate("Fasttrack Protocol"))
l7:value("ftp", translate("File Transfer Protocol"))
l7:value("gnutella", translate("Gnutella"))
l7:value("http", translate("Hypertext Transfer Protocol"))
l7:value("ident", translate("Ident Protocol"))
l7:value("irc", translate("Internet Relay Chat"))
l7:value("jabber", translate("Jabber/XMPP"))
l7:value("msnmessenger", translate("MSN Messenger"))
l7:value("ntp", translate("Network Time Protocol"))
l7:value("pop3", translate("POP3 Protocol"))
l7:value("smtp", translate("SMTP Protocol"))
l7:value("ssl", translate("SSL Protocol"))
l7:value("vnc", translate("VNC Protocol"))

ipp2p = s:option(MultiValue, "ipp2p", translate("IP-P2P"))
ipp2p.widget = "checkbox"
ipp2p:value("edk", translate("eDonkey, eMule, Kademlia"))
ipp2p:value("kazaa", translate("KaZaA, FastTrack"))
ipp2p:value("gnu", translate("Gnutella"))
ipp2p:value("dc", translate("Direct Connect"))
ipp2p:value("bit", translate("BitTorrent, extended BT"))
ipp2p:value("apple", translate("AppleJuice"))
ipp2p:value("winmx", translate("WinMX"))
ipp2p:value("soul", translate("SoulSeek"))
ipp2p:value("ares", translate("AresLite"))

return m
