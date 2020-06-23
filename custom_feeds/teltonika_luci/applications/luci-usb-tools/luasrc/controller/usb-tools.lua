module("luci.controller.usb-tools", package.seeall)

function index()
        local utl = require "luci.util"
        local usb = utl.trim(luci.sys.exec("uci get hwinfo.hwinfo.usb"))
        if usb == "1" then
                entry({"admin", "services", "usb-tools"},
                        alias("admin", "services", "usb-tools", "network_shares"), _(translate("USB Tools")), 99)
        end
end