--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: asterisk-iax-connections.lua 3620 2008-10-23 15:42:12Z jow $
]]--

cbimap = Map("asterisk", "asterisk", "")

iax = cbimap:section(TypedSection, "iax", translate("IAX Connection"), "")
iax.addremove = true

alwaysinternational = iax:option(Flag, "alwaysinternational", translate("Always Dial International"), "")
alwaysinternational.optional = true

context = iax:option(ListValue, "context", translate("Context to use"), "")
context.titleref = luci.dispatcher.build_url( "admin", "services", "asterisk", "dialplans" )
cbimap.uci:foreach( "asterisk", "dialplan", function(s) context:value(s['.name']) end )
cbimap.uci:foreach( "asterisk", "dialzone", function(s) context:value(s['.name']) end )

countrycode = iax:option(Value, "countrycode", translate("Country Code for connection"), "")
countrycode.optional = true

extension = iax:option(Value, "extension", translate("Add as Extension"), "")
extension.optional = true

host = iax:option(Value, "host", translate("Host name (or blank)"), "")
host.optional = true

internationalprefix = iax:option(Value, "internationalprefix", translate("International Dial Prefix"), "")
internationalprefix.optional = true

prefix = iax:option(Value, "prefix", translate("Dial Prefix (for external line)"), "")
prefix.optional = true

secret = iax:option(Value, "secret", translate("Secret"), "")
secret.optional = true

timeout = iax:option(Value, "timeout", translate("Dial Timeout (sec)"), "")
timeout.optional = true

type = iax:option(ListValue, "type", translate("Option type"), "")
type:value("friend", translate("Friend (outbound/inbound)"))
type:value("user", translate("User (inbound - authenticate by \"from\")"))
type:value("peer", translate("Peer (outbound - match by host)"))
type.optional = true

username = iax:option(Value, "username", translate("User name"), "")
username.optional = true


return cbimap
