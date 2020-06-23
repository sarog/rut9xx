--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: asterisk-mod-res-feature.lua 3620 2008-10-23 15:42:12Z jow $
]]--

cbimap = Map("asterisk", "asterisk", "")

featuremap = cbimap:section(TypedSection, "featuremap", translate("Feature Key maps"), "")
featuremap.anonymous = true
featuremap.addremove = true

atxfer = featuremap:option(Value, "atxfer", translate("Attended transfer key"), "")
atxfer.rmempty = true

blindxfer = featuremap:option(Value, "blindxfer", translate("Blind transfer key"), "")
blindxfer.rmempty = true

disconnect = featuremap:option(Value, "disconnect", translate("Key to Disconnect call"), "")
disconnect.rmempty = true

parkcall = featuremap:option(Value, "parkcall", translate("Key to Park call"), "")
parkcall.rmempty = true


featurepark = cbimap:section(TypedSection, "featurepark", translate("Parking Feature"), "")
featurepark.anonymous = true

parkenabled = featurepark:option(Flag, "parkenabled", translate("Enable Parking"), "")

adsipark = featurepark:option(Flag, "adsipark", translate("ADSI Park"), "")
adsipark.rmempty = true
adsipark:depends({ parkenabled = "1" })

atxfernoanswertimeout = featurepark:option(Value, "atxfernoanswertimeout", translate("Attended transfer timeout (sec)"), "")
atxfernoanswertimeout.rmempty = true
atxfernoanswertimeout:depends({ parkenabled = "1" })

automon = featurepark:option(Value, "automon", translate("One touch record key"), "")
automon.rmempty = true
automon:depends({ parkenabled = "1" })

context = featurepark:option(Value, "context", translate("Name of call context for parking"), "")
context.rmempty = true
context:depends({ parkenabled = "1" })

courtesytone = featurepark:option(Value, "courtesytone", translate("Sound file to play to parked caller"), "")
courtesytone.rmempty = true
courtesytone:depends({ parkenabled = "1" })

featuredigittimeout = featurepark:option(Value, "featuredigittimeout", translate("Max time (ms) between digits for feature activation"), "")
featuredigittimeout.rmempty = true
featuredigittimeout:depends({ parkenabled = "1" })

findslot = featurepark:option(ListValue, "findslot", translate("Method to Find Parking slot"), "")
findslot:value("first", translate("First available slot"))
findslot:value("next", translate("Next free parking space"))
findslot.rmempty = true
findslot:depends({ parkenabled = "1" })

parkedmusicclass = featurepark:option(ListValue, "parkedmusicclass", translate("Music on Hold class for the parked channel"), "")
parkedmusicclass.titleref = luci.dispatcher.build_url( "admin", "services", "asterisk" )
parkedmusicclass:depends({ parkenabled = "1" })
cbimap.uci:foreach( "asterisk", "moh", function(s) parkedmusicclass:value(s['.name']) end )

parkedplay = featurepark:option(ListValue, "parkedplay", translate("Play courtesy tone to") "")
parkedplay:value("caller", translate("Caller"))
parkedplay:value("parked", translate("Parked user"))
parkedplay:value("both", translate("Both"))
parkedplay.rmempty = true
parkedplay:depends({ parkenabled = "1" })

parkext = featurepark:option(Value, "parkext", translate("Extension to dial to park"), "")
parkext.rmempty = true
parkext:depends({ parkenabled = "1" })

parkingtime = featurepark:option(Value, "parkingtime", translate("Parking time (secs)"), "")
parkingtime.rmempty = true
parkingtime:depends({ parkenabled = "1" })

parkpos = featurepark:option(Value, "parkpos", translate("Range of extensions for call parking"), "")
parkpos.rmempty = true
parkpos:depends({ parkenabled = "1" })

pickupexten = featurepark:option(Value, "pickupexten", translate("Pickup extension"), "")
pickupexten.rmempty = true
pickupexten:depends({ parkenabled = "1" })

transferdigittimeout = featurepark:option(Value, "transferdigittimeout", translate("Seconds to wait bewteen digits when transferring"), "")
transferdigittimeout.rmempty = true
transferdigittimeout:depends({ parkenabled = "1" })

xferfailsound = featurepark:option(Value, "xferfailsound", translate("Sound when attended transfer is complete"), "")
xferfailsound.rmempty = true
xferfailsound:depends({ parkenabled = "1" })

xfersound = featurepark:option(Value, "xfersound", translate("Sound when attended transfer fails"), "")
xfersound.rmempty = true
xfersound:depends({ parkenabled = "1" })


return cbimap
