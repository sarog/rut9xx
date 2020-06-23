local m, s, o
local modeList

m = Map("network", translate("Wan selection"),
	translate("Select wan thing."))

if m:formvalue("cbid.network.wan._next") then
	local ifc
	x = uci.cursor()
	ifc = x:get("network", "wan", "ifname")
	if ifc == "eth0.2" then
		luci.http.redirect(luci.dispatcher.build_url("admin/wizard/step-wire"))
		return
	elseif ifc == "usb0" then
		luci.http.redirect(luci.dispatcher.build_url("admin/wizard/step-3g"))
		return
	elseif ifc == "eth1" then
-- 		luci.http.redirect(luci.dispatcher.build_url("admin/wizard/step-lan"))
		return
	elseif ifc == "wlan0" then
		return
	end
end

s = m:section(NamedSection, "wan", "interface", translate("Device Configuration"))
s.addremove = false

modeList = s:option(ListValue, "ifname", translate("Wan Mode"), translate("Select what mode will be used for WAN"))
modeList:value("eth0.2",  translate("Wired"))
modeList:value("eth1", translate("WiMAX"))
modeList:value("usb0", translate("3G"))
modeList:value("wlan0", translate("WiFi"))

o = s:option(DummyValue, "_prevNext", translate(" "))
o.template = "cfgwzd-module/next_apply"

testButton = s:option(Button, "_next")

testButton.title      = translate(" ")
testButton.inputtitle = translate("Next")
testButton.inputstyle = "apply"

if m:formvalue("cbi.wizard.skip") then
	luci.http.redirect(luci.dispatcher.build_url("/admin/status/sysinfo"))
end

return m
