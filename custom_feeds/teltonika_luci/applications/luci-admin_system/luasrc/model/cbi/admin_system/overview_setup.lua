m2 = Map("overview", translate("Overview Page Configuration"), 
	translate(""))
m2.addremove = false
local utl = require "luci.util"
local nw = require "luci.model.network"
local sys = require "luci.sys"
local ntm = require "luci.model.network".init()
local bus = require "ubus"
local _ubus = bus.connect()
local _ubus = bus.connect()
local uci = require "luci.model.uci".cursor()
function getParam(string)
	local h = io.popen(string)
	local t = h:read()
	h:close()
	return t
end

function hex_to_string(str)
    return (str:gsub('..', function (cc)
        return string.char(tonumber(cc, 16))
    end))
end

-- For adding the same priority drop down
function Add_ListValue(conf_name, web_name)
enb_block = sc:option(ListValue, conf_name, translate(web_name), translate("The higher priority, the higher the table will appear on the overview page. 0 - lowest priority, 5 - highest priority"))
enb_block:value("0")
enb_block:value("1")
enb_block:value("2")
enb_block:value("3")
enb_block:value("4")
enb_block:value("5")
enb_block.default = "0"
	return 1
end

local function debug(string, ...)
	luci.sys.call(string.format("/usr/bin/logger -t Webui \"%s\"", string.format(string, ...)))
end

m2.template = 'admin_system/overview_setup'
sc = m2:section(NamedSection, "show","status", translate("Overview Tables"))
sc.checkboxes=true
if luci.tools.status.show_mobile() then
	enb_block = sc:option(Flag, "mobile", translate("Mobile"), translate(""))
		enb_block.rmempty = false
		Add_ListValue("prior_mobilep", "Mobile priority")
	enb_block = sc:option(Flag, "data_limit", translate("Mobile data limit"), translate(""))
		enb_block.rmempty = false
		Add_ListValue("prior_datalimit", "Mobile data limit priority")
	enb_block = sc:option(Flag, "sms_counter", translate("SMS counter"), translate(""))
		enb_block.rmempty = false
		Add_ListValue("prior_smscounter", "SMS counter priority")
	enb_block = sc:option(Flag, "sms_limit", translate("SMS limit"), translate(""))
		enb_block.rmempty = false
		Add_ListValue("prior_smslimit", "SMS limit priority")
end

enb_block = sc:option(Flag, "system", translate("System"), translate(""))
	enb_block.rmempty = false
	Add_ListValue("prior_systemp", "System priority")
enb_block = sc:option(Flag, "wireless", translate("Wireless"), translate(""))
	enb_block.rmempty = false
	Add_ListValue("prior_wirelessp", "Wireless priority")
enb_block = sc:option(Flag, "wan", translate("WAN"), translate(""))
	enb_block.rmempty = false
	Add_ListValue("prior_wanp", "WAN priority")
enb_block = sc:option(Flag, "local_network", translate("Local network"), translate(""))
	enb_block.rmempty = false
	Add_ListValue("prior_localnetwork", "Local network priority")
enb_block = sc:option(Flag, "access_control", translate("Access control"), translate(""))
	enb_block.rmempty = false
	Add_ListValue("prior_accesscontrol", "Access control priority")
enb_block = sc:option(Flag, "system_events", translate("Recent system events"), translate(""))
	enb_block.rmempty = false
	Add_ListValue("prior_systemevents", "Recent system events priority")
enb_block = sc:option(Flag, "network_events", translate("Recent network events"), translate(""))
	enb_block.rmempty = false
	Add_ListValue("prior_networkevents", "Recent network events priority")


uci:foreach("network", "interface",
	function (section)
		local ifname = uci:get(
			"network", section[".name"], "ifname"
		)
		local metric = uci:get(
			"network", section[".name"], "metric"
		)
		local info1
		local string1
		if "usb0" == ifname then
			string1 = "network.interface." .. tostring(section[".name"])
			info1 = _ubus:call(string1, "status", { })
			
			if info1 and info1['ipv4-address'] then
				local a
				for _, a in ipairs(info1['ipv4-address']) do
					if a.address  then
						enb_block = sc:option(Flag, "wimax", translate("Wimax"), translate(""))
						enb_block.rmempty = false
						Add_ListValue("prior_wimaxp", "Wimax priority")
						debug(tostring(a.address))
					end
				end
			end
		end
	end
)

uci:foreach("openvpn" , "openvpn", function(sec)
	if sec[".name"] ~= "teltonika_auth_service" then
		local name = sec[".name"]

		if sec.name_is_hexed and sec.name_is_hexed == "1" then
			real_name = hex_to_string(sec[".name"])
		else
			real_name = name
		end

		enb_block = sc:option(Flag, "open_vpn_" .. name, real_name .. " VPN", translate(""))
		enb_block.rmempty = false
		Add_ListValue("prior_open_vpn" .. name, real_name .. " VPN priority")
	end
end)

local info = _ubus:call("network.wireless", "status", { })
local interfaces = info.radio0.interfaces
for i, net in ipairs(interfaces) do
	hotspot_id = uci:get("wireless", net.section, "hotspotid") or ""
	if hotspot_id ~= "" then
		enb_block = sc:option(Flag, hotspot_id, translate(net.config.ssid.." Hotspot"), translate(""))
		enb_block.rmempty = false
		Add_ListValue("prior_" .. hotspot_id, net.config.ssid.." Hotspot priority")
	end

end
-- enb_block = sc:option(Flag, "vrrp", translate("VRRP"), translate(""))
-- 	enb_block.rmempty = false
-- 	Add_ListValue("prior_vrrpp", "VRRP priority")

uci:foreach("vrrpd" , "vrrpd", function(sec)
		local name = sec[".name"]
		enb_block = sc:option(Flag, "vrrp_"..name, translatef("%s VRRP", name), translate(""))
		enb_block.rmempty = false
		Add_ListValue("prior_vrrpp_" .. name, name .. " VRRP priority")
end)

enb_block = sc:option(Flag, "monitoring", translate("Monitoring"), translate(""))                                                                                         
	enb_block.rmempty = false
	Add_ListValue("prior_monitoringp", "Monitoring priority")

local usb_module_name = uci:get("network", "ppp_usb", "ifname")
if usb_module_name == "wwan-usb0" then
	enb_block = sc:option(Flag, "usb_modem", translate("USB Modem"), translate(""))                                                                                         
	enb_block.rmempty = false
	Add_ListValue("prior_usbmodem", "USB Modem priority")
end


return m2
