--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: asterisk-sip-connections.lua 3620 2008-10-23 15:42:12Z jow $
]]--

cbimap = Map("asterisk", "asterisk", "")

sip = cbimap:section(TypedSection, "sip", translate("SIP Connection"), "")
sip.addremove = true

alwaysinternational = sip:option(Flag, "alwaysinternational", translate("Always Dial International"), "")
alwaysinternational.optional = true

canreinvite = sip:option(ListValue, "canreinvite", translate("Reinvite/redirect media connections"), "")
canreinvite:value("yes", translate("Yes"))
canreinvite:value("nonat", translate("Yes when not behind NAT"))
canreinvite:value("update", translate("Use UPDATE rather than INVITE for path redirection"))
canreinvite:value("no", translate("No"))
canreinvite.optional = true

context = sip:option(ListValue, "context", translate("Context to use"), "")
context.titleref = luci.dispatcher.build_url( "admin", "services", "asterisk", "dialplans" )
cbimap.uci:foreach( "asterisk", "dialplan", function(s) context:value(s['.name']) end )
cbimap.uci:foreach( "asterisk", "dialzone", function(s) context:value(s['.name']) end )

countrycode = sip:option(Value, "countrycode", translate("Country Code for connection"), "")
countrycode.optional = true

dtmfmode = sip:option(ListValue, "dtmfmode", translate("DTMF mode"), "")
dtmfmode:value("info", translate("Use RFC2833 or INFO for the BudgeTone"))
dtmfmode:value("rfc2833", translate("Use RFC2833 for the BudgeTone"))
dtmfmode:value("inband", translate("Use Inband (only with ulaw/alaw)"))
dtmfmode.optional = true

extension = sip:option(Value, "extension", translate("Add as Extension"), "")
extension.optional = true

fromdomain = sip:option(Value, "fromdomain", translate("Primary domain identity for From: headers"), "")
fromdomain.optional = true

fromuser = sip:option(Value, "fromuser", translate("From user (required by many SIP providers)"), "")
fromuser.optional = true

host = sip:option(Value, "host", translate("Host name (or blank)"), "")
host.optional = true

incoming = sip:option(DynamicList, "incoming", translate("Ring on incoming dialplan contexts"), "")
incoming.optional = true

insecure = sip:option(ListValue, "insecure", translate("Allow Insecure for"), "")
insecure:value("port", translate("Allow mismatched port number"))
insecure:value("invite", translate("Do not require auth of incoming INVITE"))
insecure:value("port,invite", translate("Allow mismatched port and Do not require auth of incoming INVITE"))
insecure.optional = true

internationalprefix = sip:option(Value, "internationalprefix", translate("International Dial Prefix"), "")
internationalprefix.optional = true

mailbox = sip:option(Value, "mailbox", translate("Mailbox for MWI"), "")
mailbox.optional = true

nat = sip:option(Flag, "nat", translate("NAT between phone and Asterisk"), "")
nat.optional = true

pedantic = sip:option(Flag, "pedantic", translate("Check tags in headers"), "")
pedantic.optional = true

port = sip:option(Value, "port", translate("SIP Port"), "")
port.optional = true

prefix = sip:option(Value, "prefix", translate("Dial Prefix (for external line)"), "")
prefix.optional = true

qualify = sip:option(Value, "qualify", translate("Reply Timeout (ms) for down connection"), "")
qualify.optional = true

register = sip:option(Flag, "register", translate("Register connection"), "")
register.optional = true

secret = sip:option(Value, "secret", translate("Secret"), "")
secret.optional = true

selfmailbox = sip:option(Flag, "selfmailbox", translate("Dial own extension for mailbox"), "")
selfmailbox.optional = true

timeout = sip:option(Value, "timeout", translate("Dial Timeout (sec)"), "")
timeout.optional = true

type = sip:option(ListValue, "type", translate("Client Type"), "")
type:value("friend", translate("Friend (outbound/inbound)"))
type:value("user", translate("User (inbound - authenticate by \"from\")"))
type:value("peer", translate("Peer (outbound - match by host)"))
type.optional = true

username = sip:option(Value, "username", translate("Username"), "")
username.optional = true


return cbimap
