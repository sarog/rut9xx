--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: asterisk-mod-chan.lua 3618 2008-10-23 02:25:26Z jow $
]]--

cbimap = Map("asterisk", "asterisk", "")


module = cbimap:section(TypedSection, "module", translate("Modules"), "")
module.anonymous = true

chan_agent = module:option(ListValue, "chan_agent", translate("Agent Proxy Channel"), "")
chan_agent:value("yes", translate("Load"))
chan_agent:value("no", translate("Do Not Load"))
chan_agent:value("auto", translate("Load as Required"))
chan_agent.rmempty = true

chan_alsa = module:option(ListValue, "chan_alsa", translate("Channel driver for GTalk"), "")
chan_alsa:value("yes", translate("Load"))
chan_alsa:value("no", translate("Do Not Load"))
chan_alsa:value("auto", translate("Load as Required"))
chan_alsa.rmempty = true

chan_gtalk = module:option(ListValue, "chan_gtalk", translate("Channel driver for GTalk"), "")
chan_gtalk:value("yes", translate("Load"))
chan_gtalk:value("no", translate("Do Not Load"))
chan_gtalk:value("auto", translate("Load as Required"))
chan_gtalk.rmempty = true

chan_iax2 = module:option(Flag, "chan_iax2", translate("Option chan_iax2"), "")
chan_iax2.rmempty = true

chan_local = module:option(ListValue, "chan_local", translate("Local Proxy Channel"), "")
chan_local:value("yes", translate("Load"))
chan_local:value("no", translate("Do Not Load"))
chan_local:value("auto", translate("Load as Required"))
chan_local.rmempty = true

chan_sip = module:option(ListValue, "chan_sip", translate("Session Initiation Protocol (SIP)"), "")
chan_sip:value("yes", translate("Load"))
chan_sip:value("no", translate("Do Not Load"))
chan_sip:value("auto", translate("Load as Required"))
chan_sip.rmempty = true


return cbimap
