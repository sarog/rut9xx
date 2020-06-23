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
local section_name
local dev_list = {}
local list_length = 0
local lines = {}
dev_list = luci.util.exec("ls /dev/usb/") or "N/A"

for s in dev_list:gmatch("[^\r\n]+") do
    table.insert(lines, s)
    list_length = list_length +1
end

if arg[1] then
	section_name = arg[1]
else
	luci.http.redirect(luci.dispatcher.build_url("admin", "services", "usb-tools", "p910nd"))
end

m = Map("p910nd", translate("Printer server"),translatef(""))

s = m:section(NamedSection, section_name, "p910nd", translate("Settings"))

s:option(Flag, "enabled", translate("Enable"))

dev = s:option(ListValue, "device", translate("Device"))
if dev_list ~= "" then
    for i=list_length,1,-1
    do
        test="/dev/usb/"..lines[i]
    dev:value(test, translate("/dev/usb/"..lines[i]))
    end
else
    dev:value("N/A", "N/A")
end

p = s:option(ListValue, "port", translate("Port"), translate("TCP listener port."))
p.rmempty = true
for i=0,9 do
	p:value(i, 9100+i)
end

bidi = s:option(Flag, "bidirectional", translate("Bidirectional mode"))
bidi.default = "1"

return m
