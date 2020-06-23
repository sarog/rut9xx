
eventlog = require'tlt_eventslog_lua'
local ut = require "luci.util"
local sys = require "luci.sys"
local m
local interval
local uci = require "luci.model.uci".cursor()

local interfacesa = {}
interfacesa["3g-ppp"]="Mobile"
interfacesa["eth2"]="Mobile"
interfacesa["eth1"]="Wired"
interfacesa["wlan0"]="WiFi"
interfacesa["none"]="Mobile bridged"
interfacesa["wwan0"]="Mobile"
interfacesa["usb0"]="WiMAX"
interfacesa["wm0"]="WiMAX"

function make_message(element, option, interval, retry)

	value = m:formvalue(string.format("cbid.sim_switch.rules.%s", option)) or m.uci:get("sim_switch", "rules", option)

	if value and value == "1" and interval then
		local time = interval * retry
		local inf_span

		if time >= 60 then
			local minutes = time / 60
			local sec = time % 60
			if sec and sec > 0 then
				inf_span = string.format("Sim switch will be performed after %.f min. %d sec.", minutes, sec)
			else
				inf_span = string.format("Sim switch will be performed after %d min", minutes)
			end
		else
			inf_span = string.format("Sim switch will be performed after %d sec.", time)
		end

		if option == "switchdata_sim1" or option == "switchdata_sim2" then
			element.user_info = inf_span .. "<br>Mobile data limit has to be enabled for current SIM."
		else
			element.user_info = inf_span
		end
	end
end

m = Map("sim_switch", translatef("SIM Switching", modulsevice))
m.addremove = false
m.disclaimer_msg = true

interval = m.uci:get("sim_switch", "sim_switch", "interval")
form_interval = m:formvalue("cbid.sim_switch.sim_switch.interval")

if interval and form_interval then
		if form_interval ~= interval then
			interval = form_interval
		end
end


s = m:section(NamedSection, "rules", "rules", translate("Primary Card"));
s.addremove = false

default= s:option(ListValue, "default", translate("Primary SIM card"), translate("SIM card that will be used in the system as a primary SIM card"))
default:value("sim1", translate("SIM 1"))
default:value("sim2", translate("SIM 2"))
default.default = "sim1"

function default.write(self, section, value)
	if value then
		--luci.sys.call("uci set -q simcard.simcard.default="..value)
		m.uci:set("simcard" ,"simcard", "default", value)
		m.uci:save("simcard")
		m.uci:commit("simcard")
		--m.uci:apply("simcard") --Apply'inam, kad uci track perleistu simcard configa
		if value == "sim1" then
			t = {requests = "insert", table = "EVENTS", type="CONFIG", text="SIM 1 selected as primary SIM card."}
			eventlog:insert(t)
		elseif value == "sim2" then
			t = {requests = "insert", table = "EVENTS", type="CONFIG", text="SIM 2 selected as primary SIM card."}
			eventlog:insert(t)
		end
	end
end

function default.cfgvalue(self, section)
	value = ut.trim(luci.sys.exec("uci get -q simcard.simcard.default"))
	return value
end

s2 = m:section(NamedSection, "sim_switch", "sim_switch", translate("SIM Switching"));

o = s2:option(Flag, "enabled", translate("Enable automatic switching"), translate("Automatically switch between primary and secondary SIM cards based on the various rules and criterions defined below"))

o = s2:option(Value, "interval", translate("Check interval"), translate("Check interval in seconds"))
	o.datatype = "uinteger"
	o.default = "30"
function o.validate(self, value, section)
	v = tonumber(value)
	if v < 1 or v > 3600 then
		m.message = translate("err: Check interval must be from 1 to 3600.")
		return nil
	else
		return tostring(v)

	end
end

s3 = m:section(NamedSection, "rules", "rules");
	s3.template = "cbi/sim_manage_tabs_switch"
	s3:tab("sim1", "SIM1 To SIM2")
	s3:tab("sim2", "SIM2 To SIM1")
	s3.addremove = false

------------------------------------
-- SIM1 tab
------------------------------------
o = s3:taboption("sim1", Flag, "switchsignal_sim1", translate("On weak signal"), translate("Perform a SIM card switch when a signal\\'s strength drops below a certain threshold"))
	make_message(o,"switchsignal_sim1", interval, 3)

o = s3:taboption("sim1", Value, "signal_sim1", translate("Signal strength (dBm)"), translate("Lowest signal\\'s strength value in dBm below which a SIM card switch should occur"))
	o.datatype = "integer"
	o:depends("switchsignal_sim1",1)

	function o.validate(self, value, section)
		v1 = tonumber(value)
		if v1 < -999 or v1 > -1 then
			m.message = translate("err: Signal strength must be from -1 to -999.")
			return nil
		else
			return tostring(v1)
		end
	end

o = s3:taboption("sim1", Flag, "switchdata_sim1", translate("On data limit"), translate("Perform a SIM card switch when mobile data limit for your currrent SIM card is exceeded"))
	o.rmempty = false
	make_message(o,"switchdata_sim1", interval, 1)

o = s3:taboption("sim1", Flag, "switchsms_sim1", translate("On SMS limit"), translate("Perform a SIM card switch when sent sms limit for your currrent SIM card is exceeded"))
	o.rmempty = false
	make_message(o,"switchsms_sim1", interval, 1)

o = s3:taboption("sim1", Flag, "switchroaming_sim1", translate("On roaming"), translate("Perform a SIM card switch when roaming is detected"))
	make_message(o,"switchroaming_sim1", interval, 2)

o = s3:taboption("sim1", Flag, "switchnonetwork_sim1", translate("No network"), translate("Perform a SIM card switch when network isn't detected"))
	make_message(o,"switchnonetwork_sim1", interval, 4)

o = s3:taboption("sim1", Flag, "ondenied1", translate("On network denied"), translate("Perform a SIM card switch when network is denied"))
	make_message(o,"ondenied1", interval, 2)

e = s3:taboption("sim1", Flag, "switchfails_sim1", translate("On data connection fail"), translate("Perform a SIM card switch when data connection fails"))
	e.rmempty = false
	method_value = m:formvalue(string.format("cbid.sim_switch.rules.check_method_sim1")) or m.uci:get("sim_switch", "rules", "check_method_sim1")
	if method_value and method_value == "icmp" then
		local retries_value = m:formvalue(string.format("cbid.sim_switch.rules.health_fail_retries_sim1")) or m.uci:get("sim_switch", "rules", "health_fail_retries_sim1") or 1
		make_message(e,"switchfails_sim1", interval, retries_value)
	else
		make_message(e,"switchfails_sim1", interval, 2)
	end

method = s3:taboption("sim1", ListValue, "check_method_sim1", translate("Method"), translate(""))
	method:value("lcp", translate("LCP echo"))
	method:value("icmp", translate("ICMP echo"))
	method:depends("switchfails_sim1", "1")

iface = s3:taboption("sim1", ListValue, "icmp_iface_sim1", translate("Health monitor ICMP interface"), translate("Choose interface as source to reach the host"))
iface:depends("check_method_sim1", "icmp")
--iface = s:option(ListValue, "interface", translate("Interface"), translate("The zone where target network resides"))

uci:foreach("network", "interface", function(a)
	if a[".name"] ~= "loopback" and a[".name"] ~= "ppp" and a[".name"] ~= "ppp_usb" and a.proto ~= "l2tp" and a.proto ~= "pptp" then
		name=interfacesa[a.ifname]
		if name then
			type=a[".name"]:upper()
			all=type.." ("..name..")"
-- 		elseif a.proto == "l2tp" then
-- 			type=a[".name"]
-- -- 			all="l2tp_"..type
-- 			all="L2TP (".. type ..")"
		else
			type=a[".name"]:upper()
			all=type
		end
		iface:value(a.ifname, all)
	end
end)

-- xl2tp=m.uci:get("xl2tpd", "xl2tpd", "_name")
-- if xl2tp and xl2tp ~= "" then
-- -- 	iface:value("ppp0", "xl2tp_"..xl2tp)
-- 	iface:value("ppp0", "xL2TP (".. xl2tp .. ")")
-- end
--
-- uci:foreach("gre_tunnel", "gre_tunnel", function(a)
-- -- 	iface:value(a.ifname)
-- 	iface:value(a.ifname, "GRE (".. a.ifname .. ")")
-- end)
--
-- uci:foreach("pptpd", "service", function(a)
-- 	local name = m.uci:get("pptpd", "pptpd", "_name") or "pptp"
-- -- 	iface:value("pptp-server", "pptp_"..name)
-- 	iface:value("pptp-server", "PPTP (".. name.. ")")
-- end)

uci:foreach("openvpn", "openvpn", function(b)
	if b.dev ~= "tun_rms" then
		iface:value(b.dev, "OpenVPN (".. b.dev .. ")")
	end
end)

icmp_hosts = s3:taboption("sim1", Value, "icmp_hosts_sim1", translate("Health monitor ICMP host"), translate("A remote host to ping (send an ICMP (Internet Control Message Protocol) packet to) and determine when connection goes down"))
	icmp_hosts.default = "8.8.8.8"
	icmp_hosts.optional = false
	icmp_hosts.datatype = "host"
	icmp_hosts:depends("check_method_sim1", "icmp")


timeout = s3:taboption("sim1", ListValue, "timeout_sim1", translate("Health monitor ICMP timeout"), translate("A timeout value for ICMP (Internet Control Message Protocol) packet"))
	timeout:value("1", translate("1 sec."))
	timeout:value("2", translate("2 sec."))
	timeout:value("3", translate("3 sec."))
	timeout:value("4", translate("4 sec."))
	timeout:value("5", translate("5 sec."))
	timeout:value("10", translate("10 sec."))
	timeout.default = "3"
	timeout.optional = false
	timeout:depends("check_method_sim1", "icmp")

fail = s3:taboption("sim1", ListValue, "health_fail_retries_sim1", translate("Attempts before SIM failover"), translate("Failed ping attempts\\' count before switching to SIM2"))
	fail:value("1", "1")
	fail:value("3", "3")
	fail:value("5", "5")
	fail:value("10", "10")
	fail:value("15", "15")
	fail:value("20", "20")
	fail.default = "3"
	fail.optional = false
	fail:depends("check_method_sim1", "icmp")

e = s3:taboption("sim1", Flag, "switchtimeout_sim1", translate("Switch back to primary SIM card after timeout"), translate("Switch back to primary SIM card after timeout has been reached"))
	e.rmempty = false
	e:depends({default = "sim2"})

o = s3:taboption("sim1", Value, "initial_sim1", translate("Initial timeout (min)"), translate("An initial timeout value in minutes after which a SIM card\\'s switch-back should occur"))
	o.datatype = "integer"
	o:depends({default = "sim2", switchtimeout_sim1 = "1"})
	o.default="1"

o = s3:taboption("sim1", Value, "subsequent_sim1", translate("Subsequent timeout (min)"), translate("A subsequent timeout value in minutes after which a SIM card\\'s switch-back should occur.\\nThis value is increased every time an unsuccessfull switch-back is made: new_timeout = old_timeout + subsequent_timeout"))
	o.datatype = "integer"
	o:depends({default = "sim2", switchtimeout_sim1 = "1"})
	o.default="0"

------------------------------------
-- SIM2 tab
------------------------------------
o = s3:taboption("sim2", Flag, "switchsignal_sim2", translate("On weak signal"), translate("Perform a SIM card switch when a signal\\'s strength drops below a certain threshold"))
	make_message(o,"switchsignal_sim2", interval, 3)

o = s3:taboption("sim2", Value, "signal_sim2", translate("Signal strength (dBm)"), translate("Lowest signal\\'s strength value in dBm below which a SIM card switch should occur"))
	o.datatype = "integer"
	o:depends("switchsignal_sim2",1)
	function o.validate(self, value, section)
            v1 = tonumber(value)
            if v1 < -999 then
                m.message = translate("err: Signal strength must be between -1 to -999.")
                return nil
            elseif v1 > -1 then
                m.message = translate("err: Signal strength must be between -1 to -999.")
                return nil
            else
                return tostring(v1)
            end
	end

o = s3:taboption("sim2", Flag, "switchdata_sim2", translate("On data limit"), translate("Perform a SIM card switch when mobile data limit for your currrent SIM card is exceeded"))
	o.rmempty = false
	make_message(o,"switchdata_sim2", interval, 1)

o = s3:taboption("sim2", Flag, "switchsms_sim2", translate("On SMS limit"), translate("Perform a SIM card switch when sent sms limit for your currrent SIM card is exceeded"))
	o.rmempty = false
	make_message(o,"switchsms_sim2", interval, 1)

o = s3:taboption("sim2", Flag, "switchroaming_sim2", translate("On roaming"), translate("Perform a SIM card switch when roaming is detected"))
	make_message(o,"switchroaming_sim2", interval, 2)

o = s3:taboption("sim2", Flag, "switchnonetwork_sim2", translate("No network"), translate("Perform a SIM card switch when network isn't detected"))
	make_message(o,"switchnonetwork_sim2", interval, 4)

o = s3:taboption("sim2", Flag, "ondenied2", translate("On network denied"), translate("Perform a SIM card switch when network is denied"))
	make_message(o,"ondenied2", interval, 2)

e = s3:taboption("sim2", Flag, "switchfails_sim2", translate("On data connection fail"), translate("Perform a SIM card switch when data connection fails"))
	e.rmempty = false
	method_value = m:formvalue(string.format("cbid.sim_switch.rules.check_method_sim2")) or m.uci:get("sim_switch", "rules", "check_method_sim2")
	if method_value and method_value == "icmp" then
		local retries_value = m:formvalue(string.format("cbid.sim_switch.rules.health_fail_retries_sim2")) or m.uci:get("sim_switch", "rules", "health_fail_retries_sim2") or 1
		make_message(e,"switchfails_sim2", interval, retries_value)
	else
		make_message(e,"switchfails_sim2", interval, 2)
	end

method = s3:taboption("sim2", ListValue, "check_method_sim2", translate("Method"), translate(""))
	method:value("lcp", translate("LCP echo"))
	method:value("icmp", translate("ICMP echo"))
	method:depends("switchfails_sim2", "1")

iface = s3:taboption("sim2", ListValue, "icmp_iface_sim2", translate("Health monitor ICMP interface"), translate("Choose interface as source to reach the host"))
iface:depends("check_method_sim2", "icmp")

uci:foreach("network", "interface", function(a)
	if a[".name"] ~= "loopback" and a[".name"] ~= "ppp" and a[".name"] ~= "ppp_usb" and a.proto ~= "l2tp" and a.proto ~= "pptp" then
		name=interfacesa[a.ifname]
		if name then
			type=a[".name"]:upper()
			all=type.." ("..name..")"
		else
			type=a[".name"]:upper()
			all=type
		end
		iface:value(a.ifname, all)
	end
end)

uci:foreach("openvpn", "openvpn", function(b)
	if b.dev ~= "tun_rms" then
		iface:value(b.dev, "OpenVPN (".. b.dev .. ")")
	end
end)

icmp_hosts = s3:taboption("sim2", Value, "icmp_hosts_sim2", translate("Health monitor ICMP host"), translate("A remote host to ping (send an ICMP (Internet Control Message Protocol) packet to) and determine when connection goes down"))
	icmp_hosts.default = "8.8.8.8"
	icmp_hosts.optional = false
	icmp_hosts.datatype = "host"
	icmp_hosts:depends("check_method_sim2", "icmp")

timeout = s3:taboption("sim2", ListValue, "timeout_sim2", translate("Health monitor ICMP timeout"), translate("A timeout value for ICMP (Internet Control Message Protocol) packet"))
	timeout:value("1", translate("1 sec."))
	timeout:value("2", translate("2 sec."))
	timeout:value("3", translate("3 sec."))
	timeout:value("4", translate("4 sec."))
	timeout:value("5", translate("5 sec."))
	timeout:value("10", translate("10 sec."))
	timeout.default = "3"
	timeout.optional = false
	timeout:depends("check_method_sim2", "icmp")

fail = s3:taboption("sim2", ListValue, "health_fail_retries_sim2", translate("Attempts before SIM failover"), translate("Failed ping attempts\\' count before switching to SIM1"))
	fail:value("1", "1")
	fail:value("3", "3")
	fail:value("5", "5")
	fail:value("10", "10")
	fail:value("15", "15")
	fail:value("20", "20")
	fail.default = "3"
	fail.optional = false
	fail:depends("check_method_sim2", "icmp")

e = s3:taboption("sim2", Flag, "switchtimeout_sim2", translate("Switch back to primary SIM card after timeout"), translate("Switch back to primary SIM card after timeout has been reached"))
	e.rmempty = false
	e:depends({default = "sim1"})

o = s3:taboption("sim2", Value, "initial_sim2", translate("Initial timeout (min)"), translate("An initial timeout value in minutes after which a SIM card\\'s switch-back should occur"))
	o.datatype = "integer"
	o:depends({default = "sim1", switchtimeout_sim2 = "1"})
	o.default="1"

o = s3:taboption("sim2", Value, "subsequent_sim2", translate("Subsequent timeout (min)"), translate("A subsequent timeout value in minutes after which a SIM card\\'s switch-back should occur.\\nThis value is increased every time an unsuccessfull switch-back is made: new_timeout = old_timeout + subsequent_timeout"))
	o.datatype = "integer"
	o:depends({default = "sim1", switchtimeout_sim2 = "1"})
	o.default="0"

function m.on_commit(self)
	local enabled = m:formvalue("cbid."..self.config..".sim_switch.enabled") or "0"
	local sim1_enb = m:formvalue("cbid."..self.config..".rules.switchdata_sim1") or "0"
	local sim2_enb = m:formvalue("cbid."..self.config..".rules.switchdata_sim2") or "0"
	local sms1_enb = m:formvalue("cbid."..self.config..".rules.switchsms_sim1") or "0"
	local sms2_enb = m:formvalue("cbid."..self.config..".rules.switchsms_sim2") or "0"
	local sms_limit_sim1 = m.uci:get(self.config, "rules", "sms_limit_enable_sim1") or "0"
	local sms_limit_sim2 = m.uci:get(self.config, "rules", "sms_limit_enable_sim2") or "0"


	if enabled == "1" and (sim1_enb == "1" or sim2_enb == "1") then
		luci.sys.call("uci set -q mdcollectd.config.sim_switch=1")
	else
		luci.sys.call("uci set -q mdcollectd.config.sim_switch=0")
	end

	if sms1_enb == "1" or sms2_enb == "1" or sms_limit_sim1 == "1"
	or sms_limit_sim2 == "1" then
		luci.sys.call("uci set -q smscollect.config.enabled=1")
		luci.sys.call("sed -i '/smscollect crontab/d' /etc/crontabs/root 2>/dev/null")
		luci.sys.call("echo '1 0 * * * /usr/bin/smscollect crontab' >> /etc/crontabs/root")
		luci.sys.call("uci set -q overview.show.sms_limit=1")
	else
		luci.sys.call("sed -i '/smscollect crontab/d' /etc/crontabs/root 2>/dev/null")
		luci.sys.call("uci set -q smscollect.config.enabled=0")
		luci.sys.call("uci set -q overview.show.sms_limit=0")
	end
	luci.sys.call("uci commit")
end

return m
