require("luci.fs")
require("luci.config")
require "teltonika_lua_functions"

local utl = require "luci.util"
local nw = require "luci.model.network"
local sys = require "luci.sys"
local types = require "luci.cbi.datatypes"

m = Map("azure_iothub", translate("Azure IoTHub Settings"), translate(""))
m.template = 'azure_iothub/azure_iothub'
s = m:section(NamedSection, "azure_iothub", "azure_iothub", translate("Azure IoThub"))
open_e = s:option(Flag, "enable", translate("Enable Azure IoThub monitoring"), translate(""))
open_e.rmempty = false

o = s:option(Value, "connection_string", translate("Connection string"))
o.datatype = "string_not_empty"

o = s:option(ListValue, "msg_type", translate("Messages Type"))
o:value("gsmctl", translate("GSM values"))
o:value("mqtt", translate("MQTT messages"))

p = s:option(Value, "mqtt_ip", translate("MQTT Host"), translate("Host address of MQTT broker"))
p.datatype = "string_not_empty"
p:depends("msg_type", "mqtt")

function p.validate(self, value)
	local type = luci.http.formvalue("cbid.azure_iothub.azure_iothub.msg_type")
	if type == "mqtt" then
		if types.host(value) then
			return value
		else
			m.message = translatef("err: Bad Host field input format")
			return nil
		end
	end
end

p = s:option(Value, "mqtt_port", translate("MQTT Port"), translate("Port of MQTT broker"))
p.datatype = "string_not_empty"
p:depends("msg_type", "mqtt")

function p.validate(self, value)
	local type = luci.http.formvalue("cbid.azure_iothub.azure_iothub.msg_type")
	if type == "mqtt" then
		if types.port(value) then
			return value
		else
			m.message = translatef("err: Bad Port field input format")
			return nil
		end
	end
end

p = s:option(Value, "mqtt_topic", translate("Topic"), translate("Topic to subscribe"))
p.datatype = "string_not_empty"
p:depends("msg_type", "mqtt")

p = s:option(Value, "mqtt_username", translate("Username"), translate(""))
p:depends("msg_type", "mqtt")

p = s:option(Value, "mqtt_password", translate("Password"), translate(""))
p.datatype = "password_validate"
p.noautocomplete = true
p.password = true
p:depends("msg_type", "mqtt")

p = s:option(Value, "message_interval", translate("Message sending interval (sec.)"), translate("Number of seconds to send device data to Azure IoThub (min. 10s)"))
p.default = "300"
p.datatype = "range(10,99999)"
p:depends("msg_type", "gsmctl")

ipaddr = s:option(Flag, "ipaddr", translate("IP address"), translate(""))
ipaddr.rmempty = false
ipaddr:depends("msg_type", "gsmctl")

bsent = s:option(Flag, "bsent", translate("Number of bytes sent"), translate(""))
bsent.rmempty = false
bsent:depends("msg_type", "gsmctl")

brecv = s:option(Flag, "brecv", translate("Number of bytes received"), translate(""))
brecv.rmempty = false
brecv:depends("msg_type", "gsmctl")

connstate = s:option(Flag, "connstate", translate("Mobile connection state"), translate(""))
connstate.rmempty = false
connstate:depends("msg_type", "gsmctl")

netstate = s:option(Flag, "netstate", translate("Network link state"), translate(""))
netstate.rmempty = false
netstate:depends("msg_type", "gsmctl")

imei = s:option(Flag, "imei", translate("IMEI"), translate(""))
imei.rmempty = false
imei:depends("msg_type", "gsmctl")

iccid = s:option(Flag, "iccid", translate("ICCID"), translate(""))
iccid.rmempty = false
iccid:depends("msg_type", "gsmctl")

model = s:option(Flag, "model", translate("Model"), translate(""))
model.rmempty = false
model:depends("msg_type", "gsmctl")

manuf = s:option(Flag, "manuf", translate("Manufacturer"), translate(""))
manuf.rmempty = false
manuf:depends("msg_type", "gsmctl")

serial = s:option(Flag, "serial", translate("Serial"), translate(""))
serial.rmempty = false
serial:depends("msg_type", "gsmctl")

revision = s:option(Flag, "revision", translate("Revision"), translate(""))
revision.rmempty = false
revision:depends("msg_type", "gsmctl")

imsi = s:option(Flag, "imsi", translate("IMSI"), translate(""))
imsi.rmempty = false
imsi:depends("msg_type", "gsmctl")

simstate = s:option(Flag, "simstate", translate("SIM state"), translate(""))
simstate.rmempty = false
simstate:depends("msg_type", "gsmctl")

pinstate = s:option(Flag, "pinstate", translate("PIN state"), translate(""))
pinstate.rmempty = false
pinstate:depends("msg_type", "gsmctl")

signal = s:option(Flag, "signal", translate("GSM signal"), translate(""))
signal.rmempty = false
signal:depends("msg_type", "gsmctl")

rscp = s:option(Flag, "rscp", translate("WCDMA RSCP"), translate(""))
rscp.rmempty = false
rscp:depends("msg_type", "gsmctl")

ecio = s:option(Flag, "ecio", translate("WCDMA EC/IO"), translate(""))
ecio.rmempty = false
ecio:depends("msg_type", "gsmctl")

rsrp = s:option(Flag, "rsrp", translate("LTE RSRP"), translate(""))
rsrp.rmempty = false
rsrp:depends("msg_type", "gsmctl")

sinr = s:option(Flag, "sinr", translate("LTE SINR"), translate(""))
sinr.rmempty = false
sinr:depends("msg_type", "gsmctl")

rsrq = s:option(Flag, "rsrq", translate("LTE RSRQ"), translate(""))
rsrq.rmempty = false
rsrq:depends("msg_type", "gsmctl")

cellid = s:option(Flag, "cellid", translate("CELL ID"), translate(""))
cellid.rmempty = false
cellid:depends("msg_type", "gsmctl")

operator = s:option(Flag, "operator", translate("Operator"), translate(""))
operator.rmempty = false
operator:depends("msg_type", "gsmctl")

opernum = s:option(Flag, "opernum", translate("Operator number"), translate(""))
opernum.rmempty = false
opernum:depends("msg_type", "gsmctl")

conntype = s:option(Flag, "conntype", translate("Connection type"), translate(""))
conntype.rmempty = false
conntype:depends("msg_type", "gsmctl")

temp = s:option(Flag, "temp", translate("Temperature"), translate(""))
temp.rmempty = false
temp:depends("msg_type", "gsmctl")

pincount = s:option(Flag, "pincount", translate("PIN count"), translate(""))
pincount.rmempty = false
pincount:depends("msg_type", "gsmctl")

return m
