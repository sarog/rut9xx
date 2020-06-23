local uci = require "luci.model.uci".cursor()

uci_galileo = luci.util.trim(luci.sys.exec("uci get gps.gpsd.galileo_sup"))
uci_glonass = luci.util.trim(luci.sys.exec("uci get gps.gpsd.glonass_sup"))
uci_beidou = luci.util.trim(luci.sys.exec("uci get gps.gpsd.beidou_sup"))

map = Map("gps",
        translate("GPS configuration"),
        translate("GPS service needs to be enabled to use GPS related functionallity. " ..
                     "You can micromanage this service using configuration options inside " ..
                     "the tabs above."))
sec = map:section(NamedSection, "gpsd", "General configuration")

enabled = sec:option(Flag, "enabled",
                     translate("Enabled"),
                     translate("Enable the GPS service to use GPS related functionality"))
enabled.rmempty = false

sec_sat = map:section(NamedSection, "gpsd", translate("Satellite configuration"), translate("Satellite configuration"),
		      translate("Changing these options requires modem reboot"))

galileo = sec_sat:option(Flag, "galileo_sup",
                     translate("Galileo NMEA support"),
                     translate("Enable the Galileo satellite support"))
galileo.rmempty = false
galileo.enabled = "1"

glonass = sec_sat:option(Flag, "glonass_sup",
                     translate("Glonass NMEA support"),
                     translate("Enable the Glonass satellite support"))
glonass.rmempty = false
glonass.enabled = "7"

beidou = sec_sat:option(Flag, "beidou_sup",
                     translate("BeiDou NMEA support"),
                     translate("Enable the BeiDou satellite support"))
beidou.rmempty = false
beidou.enabled = "3"

function map.on_commit(self)
	local web_galileo = map:formvalue("cbid.gps.gpsd.galileo_sup") or "0"
	local web_glonass = map:formvalue("cbid.gps.gpsd.glonass_sup") or "0"
	local web_beidou = map:formvalue("cbid.gps.gpsd.beidou_sup") or "0"
	local flag = 0

	if web_galileo ~= uci_galileo then
		luci.sys.exec("gsmctl -A AT+QGPSCFG=\\\"galileonmeatype\\\",%d" %{ web_galileo })
		flag = 1
	end

	if web_glonass ~= uci_glonass then
		luci.sys.exec("gsmctl -A AT+QGPSCFG=\\\"glonassnmeatype\\\",%d" %{ web_glonass })
		flag = 1
	end

	if web_beidou ~= uci_beidou then
		luci.sys.exec("gsmctl -A AT+QGPSCFG=\\\"beidounmeatype\\\",%d" %{ web_beidou })
		flag = 1
	end

	if flag == 1 then
		luci.sys.exec("/etc/init.d/modem restart")
	end
end

return map
