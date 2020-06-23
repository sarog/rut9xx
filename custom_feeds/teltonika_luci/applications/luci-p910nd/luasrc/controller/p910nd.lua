--[[

LuCI UVC Streamer
(c) 2008 Yanira <forum-2008@email.de>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

$Id: p910nd.lua 7362 2011-08-12 13:16:27Z jow $

]]--

module("luci.controller.p910nd", package.seeall)

function index()
        local sys = require "luci.sys"
        local utl = require "luci.util"
        local usb = utl.trim(luci.sys.exec("uci get hwinfo.hwinfo.usb"))
        if usb == "1" then
            if not nixio.fs.access("/etc/config/p910nd") then
                    return
            end

            local page

            page = entry({"admin", "services", "usb-tools", "p910nd"}, arcombine(cbi("p910nd_add"), cbi("p910nd_edit")), _(translate("Printer server")), 2)
            page.leaf=true
            page.i18n = "p910nd"
            page.dependent = true
        end
end
