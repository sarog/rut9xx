--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: asterisk-dialplans.lua 3620 2008-10-23 15:42:12Z jow $
]]--

cbimap = Map("asterisk", "asterisk", "")

dialplan = cbimap:section(TypedSection, "dialplan", translate("Section dialplan"), "")
dialplan.addremove = true
dialplan.dynamic = true

include = dialplan:option(MultiValue, "include", translate("Include zones and plans"), "")
cbimap.uci:foreach( "asterisk", "dialplan", function(s) include:value(s['.name']) end )
cbimap.uci:foreach( "asterisk", "dialzone", function(s) include:value(s['.name']) end )

dialplanexten = cbimap:section(TypedSection, "dialplanexten", translate("Dialplan Extension"), "")
dialplanexten.anonymous = true
dialplanexten.addremove = true
dialplanexten.dynamic = true


dialplangeneral = cbimap:section(TypedSection, "dialplangeneral", translate("Dialplan General Options"), "")
dialplangeneral.anonymous = true
dialplangeneral.addremove = true

allowtransfer = dialplangeneral:option(Flag, "allowtransfer", translate("Allow transfer"), "")
allowtransfer.rmempty = true

canreinvite = dialplangeneral:option(ListValue, "canreinvite", translate("Reinvite/redirect media connections"), "")
canreinvite:value("yes", translate("Yes"))
canreinvite:value("nonat", translate("Yes when not behind NAT"))
canreinvite:value("update", translate("Use UPDATE rather than INVITE for path redirection"))
canreinvite:value("no", translate("No"))
canreinvite.rmempty = true

clearglobalvars = dialplangeneral:option(Flag, "clearglobalvars", translate("Clear global vars"), "")
clearglobalvars.rmempty = true


dialplangoto = cbimap:section(TypedSection, "dialplangoto", translate("Dialplan Goto"), "")
dialplangoto.anonymous = true
dialplangoto.addremove = true
dialplangoto.dynamic = true


dialplanmeetme = cbimap:section(TypedSection, "dialplanmeetme", translate("Dialplan Conference"), "")
dialplanmeetme.anonymous = true
dialplanmeetme.addremove = true
dialplanmeetme.dynamic = true


dialplansaytime = cbimap:section(TypedSection, "dialplansaytime", translate("Dialplan Time"), "")
dialplansaytime.anonymous = true
dialplansaytime.addremove = true
dialplansaytime.dynamic = true


dialplanvoice = cbimap:section(TypedSection, "dialplanvoice", translate("Dialplan Voicemail"), "")
dialplanvoice.anonymous = true
dialplanvoice.addremove = true
dialplanvoice.dynamic = true


dialzone = cbimap:section(TypedSection, "dialzone", translate("Dial Zones for Dialplan"), "")
dialzone.addremove = true
dialzone.template = "cbi/tblsection"

addprefix = dialzone:option(Value, "addprefix", translate("Prefix to add matching dialplans"), "")
addprefix.rmempty = true

--international = dialzone:option(DynamicList, "international", "Match International prefix", "")
international = dialzone:option(Value, "international", translate("Match International prefix"), "")
international.rmempty = true

localprefix = dialzone:option(Value, "localprefix", translate("Prefix (0) to add/remove to/from intl. numbers"), "")
localprefix.rmempty = true

localzone = dialzone:option(Value, "localzone", translate("Dialzone for intl. numbers matched as local"), "")
localzone.titleref = luci.dispatcher.build_url( "admin", "services", "asterisk", "dialplans" )
cbimap.uci:foreach( "asterisk", "dialplan", function(s) localzone:value(s['.name']) end )
cbimap.uci:foreach( "asterisk", "dialzone", function(s) localzone:value(s['.name']) end )

match = dialzone:option(Value, "match", translate("Match plan"), "")
match.rmempty = true

uses = dialzone:option(ListValue, "uses", translate("Connection to use"), "")
uses.titleref = luci.dispatcher.build_url( "admin", "services", "asterisk", "sip-conns" )
cbimap.uci:foreach( "asterisk", "sip", function(s) uses:value('SIP/'..s['.name']) end )
cbimap.uci:foreach( "asterisk", "iax", function(s) uses:value('IAX/'..s['.name']) end )


return cbimap
