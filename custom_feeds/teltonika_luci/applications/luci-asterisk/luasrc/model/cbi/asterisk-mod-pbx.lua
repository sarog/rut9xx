--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: asterisk-mod-pbx.lua 3618 2008-10-23 02:25:26Z jow $
]]--

cbimap = Map("asterisk", "asterisk", "")

module = cbimap:section(TypedSection, "module", translate("Modules"), "")
module.anonymous = true

pbx_ael = module:option(ListValue, "pbx_ael", translate("Asterisk Extension Language Compiler"), "")
pbx_ael:value("yes", translate("Load"))
pbx_ael:value("no", translate("Do Not Load"))
pbx_ael:value("auto", translate("Load as Required"))
pbx_ael.rmempty = true

pbx_config = module:option(ListValue, "pbx_config", translate("Text Extension Configuration"), "")
pbx_config:value("yes", translate("Load"))
pbx_config:value("no", translate("Do Not Load"))
pbx_config:value("auto", translate("Load as Required"))
pbx_config.rmempty = true

pbx_functions = module:option(ListValue, "pbx_functions", translate("load => .so ; Builtin dialplan functions"), "")
pbx_functions:value("yes", translate("Load"))
pbx_functions:value("no", translate("Do Not Load"))
pbx_functions:value("auto", translate("Load as Required"))
pbx_functions.rmempty = true

pbx_loopback = module:option(ListValue, "pbx_loopback", translate("Loopback Switch"), "")
pbx_loopback:value("yes", translate("Load"))
pbx_loopback:value("no", translate("Do Not Load"))
pbx_loopback:value("auto", translate("Load as Required"))
pbx_loopback.rmempty = true

pbx_realtime = module:option(ListValue, "pbx_realtime", translate("Realtime Switch"), "")
pbx_realtime:value("yes", translate("Load"))
pbx_realtime:value("no", translate("Do Not Load"))
pbx_realtime:value("auto", translate("Load as Required"))
pbx_realtime.rmempty = true

pbx_spool = module:option(ListValue, "pbx_spool", translate("Outgoing Spool Support"), "")
pbx_spool:value("yes", translate("Load"))
pbx_spool:value("no", translate("Do Not Load"))
pbx_spool:value("auto", translate("Load as Required"))
pbx_spool.rmempty = true

pbx_wilcalu = module:option(ListValue, "pbx_wilcalu", translate("Wil Cal U (Auto Dialer)"), "")
pbx_wilcalu:value("yes", translate("Load"))
pbx_wilcalu:value("no", translate("Do Not Load"))
pbx_wilcalu:value("auto", translate("Load as Required"))
pbx_wilcalu.rmempty = true


return cbimap
