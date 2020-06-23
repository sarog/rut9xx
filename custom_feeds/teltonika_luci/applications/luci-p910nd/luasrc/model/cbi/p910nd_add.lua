--[[

LuCI p910nd
(c) 2008 Yanira <forum-2008@email.de>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

$Id: p910nd.lua 7739 2011-10-16 14:08:39Z soma $

]]--

local uci = luci.model.uci.cursor_state()

m = Map("p910nd", translate("Printer server"),translatef(""))

s = m:section(TypedSection, "p910nd", translate("Printer server instances"))
s.addremove = true
s.template = "p910nd/tblsection"
s.novaluetext = translate("There are no network printers configurations yet")
s.extedit = luci.dispatcher.build_url("admin", "services", "usb-tools", "p910nd", "%s")
s.defaults = {enabled = "0"}
s.sectionhead = "Name"

s:option(Flag, "enabled", translate("Enable"))

return m
