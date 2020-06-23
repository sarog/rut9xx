local uci = require "luci.model.uci".cursor()
local ds = require "luci.dispatcher"
local sys = require "luci.sys"
local utl = require "luci.util"

local function parseFunctionFactory(o, e, p, pv, f, fv, t, tt, ttv)
	return function(self, section, novld)
		local xe, xp, xf, xt, xtt
		if e ~= nil then xe = e:formvalue(section) == "1" else xe = false end
		if p ~= nil then xp = p:formvalue(section) == pv else xp = true end
		if f ~= nil then xf = f:formvalue(section) == fv else xf = true end
		if t ~= nil then xt = t:formvalue(section) == "1" else xt = true end
		if xt == true and tt ~= nil then xtt = tt:formvalue(section) == ttv else xtt = true end
		if xe == true and xp == true and xf == true and xt == true and xtt == true then
			local v = self:formvalue(section)
			if v == nil or type(v) ~= "string" or v == "" then
				self:add_error(section, "invalid", self.title .. " cannot be empty")
			end
		end
		o.parse(self, section, novld)
	end
end

local slave_cfg
if arg[1] then
	slave_cfg = arg[1]
else
	luci.http.redirect(ds.build_url("admin/services/modbus/modbus_data_sender"))
end

m = Map("modbus_data_sender", translate("Advanced sender settings"), translate("Here you can configure advanced settings for the data sender"))
m.redirect = ds.build_url("admin/services/modbus/modbus_data_sender")

local sender_id = m.uci:get("modbus_data_sender", slave_cfg, "sender_id")

if not sender_id or sender_id == "" then
	timestring = utl.trim(sys.exec("date +%s")) or "N/A"
	m.uci:set("modbus_data_sender", slave_cfg, "sender_id", slave_cfg .. "_" .. timestring)
end

s = m:section(NamedSection, slave_cfg, "data_sender", "Data sender configuration")

enabled = s:option(Flag, "enabled", translate("Enabled"), translate("Check to enable this data sender"))

name = s:option(Value, "name", translate("Name"), translate("Name of the data sender. Used for easier data senders management purposes only (optional)"))
name.datatype = "lengthvalidation(0, 64)"
name.parse = parseFunctionFactory(Value, enabled, nil, nil, nil, nil, nil, nil, nil)

protocol = s:option(ListValue, "protocol", translate("Protocol"), translate("Protocol used for sending the data to server"))
protocol:value("http", translate("HTTP(S)"))
protocol:value("mqtt", translate("MQTT"))
if luci.sys.exec("opkg list-installed tlt_custom_pkg_azure_iothub"):sub(1, #("tlt_custom_pkg_azure_iothub")) == "tlt_custom_pkg_azure_iothub" then
	protocol:value("azure_mqtt", translate("Azure MQTT"))
end

json_string = s:option(Value, "json_string", translate("JSON format"), translate("allows to fully customize JSON segment"))
json_string.template = "modbus_master/custom_json_textbox"
json_string.default = '{"ID":"%i", "TS":"%t","ST":"%s","VR":"%a"}'
json_string.rows = "8"
json_string.indicator = arg[1]
json_string.parse = parseFunctionFactory(Value, enabled, nil, nil, nil, nil, nil, nil, nil)

json_segment_count = s:option(ListValue, "json_segment_count", translate("Segment count"), translate("Max segment count in one JSON string sent to server"))
for i = 1, 10 do
	json_segment_count:value(("%d"):format(i), ("%d"):format(i))
end

host = s:option(Value, "host", translate("URL / Host / Connection string"), translate("URL for HTTP(S); Host for MQTT; Connection string for Azure MQTT"))
host.datatype = "lengthvalidation(0, 512)"
host.parse = parseFunctionFactory(Value, enabled, nil, nil, nil, nil, nil, nil, nil)

port = s:option(Value, "port", translate("Port"), translate("Port number"))
port.datatype = "port"
port.default = "1883"
port:depends("protocol", "mqtt")
port.parse = parseFunctionFactory(Value, enabled, protocol, "mqtt", nil, nil, nil, nil, nil)

keepalive = s:option(Value, "keepalive", translate("Keepalive"), translate("MQTT keepalive period in seconds [1-640]"))
keepalive.datatype = "range(1,640)"
keepalive.default = "60"
keepalive:depends("protocol", "mqtt")
keepalive.parse = parseFunctionFactory(Value, enabled, protocol, "mqtt", nil, nil, nil, nil, nil)

topic = s:option(Value, "topic", translate("Topic"), translate("MQTT topic to be used for publishing the data"))
topic.datatype = "lengthvalidation(0, 512)"
topic:depends("protocol", "mqtt")
topic.parse = parseFunctionFactory(Value, enabled, protocol, "mqtt", nil, nil, nil, nil, nil)

period = s:option(Value, "period", translate("Period"), translate("Interval for sending the collected data to server (in seconds, 1-86400)"))
period.datatype = "range(1,86400)"
period.default = "60"
period.parse = parseFunctionFactory(Value, enabled, nil, nil, nil, nil, nil, nil, nil)

device_filtering = s:option(ListValue, "device_filtering", translate("Data filtering"), translate("Choose which data this sender will send to server"))
device_filtering:value("all", translate("All data"))
device_filtering:value("slave_id", translate("By slave ID"))
device_filtering:value("slave_ip", translate("By slave IP"))

filter_slave_id = s:option(Value, "filter_slave_id", translate("Slave ID"), translate("Data will be sent to server only from slave device with this modbus ID (1-255)"))
filter_slave_id.datatype = "range(1, 255)"
filter_slave_id:depends("device_filtering", "slave_id")
filter_slave_id.parse = parseFunctionFactory(Value, enabled, nil, nil, device_filtering, "slave_id", nil, nil, nil)

filter_slave_ip = s:option(Value, "filter_slave_ip", translate("Slave IP"), translate("Data will be sent to server from slave device with this IP address only (Modbus TCP slaves only)"))
filter_slave_ip.datatype = "host"
filter_slave_ip:depends("device_filtering", "slave_ip")
filter_slave_ip.parse = parseFunctionFactory(Value, enabled, nil, nil, device_filtering, "slave_ip", nil, nil, nil)

retry_sending = s:option(Flag, "retry_sending", translate("Retry on fail"), translate("In case of a failed attempt, retry to send the same data to server later (Retry until successful)"))

custom_header = s:option(DynamicList, "custom_header", translate("Custom Header"), translate("Allows to add custom headers to the HTTP requests"))
custom_header:depends("protocol", "http")

tls_enabled = s:option(Flag, "tls_enabled", translate("Use TLS"), translate("Use TLS to encrypt the data sent"))
tls_enabled:depends("protocol", "mqtt")

tls_type = s:option(ListValue, "tls_type", translate("TLS type"), translate("Choose TLS type"))
tls_type:value("cert", translate("Certificate based"))
tls_type:value("psk", translate("Pre-Shared-Key based"))
tls_type:depends("tls_enabled", "1")

cafile = s:option(FileUpload, "cafile", translate("CA File"), translate("Add trusted CA certificate file"))
cafile:depends("tls_type","cert")
cafile.size = "51200"
cafile.sizetext = translate("Selected file is too large. Maximum allowed size is 50 KiB")
cafile.sizetextempty = translate("Selected file is empty")
cafile.parse = parseFunctionFactory(FileUpload, enabled, protocol, "mqtt", nil, nil, tls_enabled, tls_type, "cert")

certfile = s:option(FileUpload, "certfile", translate("Client certificate"), translate("Add client certificate file. If client certificate is not needed, leave both client certificate and client key fields empty"))
certfile:depends("tls_type","cert")
certfile.size = "51200"
certfile.sizetext = translate("Selected file is too large. Maximum allowed size is 50 KiB")
certfile.sizetextempty = translate("Selected file is empty")
certfile.parse = parseFunctionFactory(FileUpload, enabled, protocol, "mqtt", nil, nil, tls_enabled, tls_type, "cert")

keyfile = s:option(FileUpload, "keyfile", translate("Private key"), translate("File containing private key for this client. This file needs to be not encrypted"))
keyfile:depends("tls_type","cert")
keyfile.size = "51200"
keyfile.sizetext = translate("Selected file is too large. Maximum allowed size is 50 KiB")
keyfile.sizetextempty = translate("Selected file is empty")
keyfile.parse = parseFunctionFactory(FileUpload, enabled, protocol, "mqtt", nil, nil, tls_enabled, tls_type, "cert")

psk = s:option(Value, "psk", translate("Pre-Shared-Key"), translate("The pre-shared-key in hex format with no leading “0x”"))
psk.datatype = "lengthvalidation(0, 128)"
psk:depends("tls_type","psk")
psk.parse = parseFunctionFactory(Value, enabled, protocol, "mqtt", nil, nil, tls_enabled, tls_type, "psk")

identity = s:option(Value, "identity", translate("Identity"), translate("The identity of this client.  May be used as the username depending on the server settings"))
identity.datatype = "lengthvalidation(0, 128)"
identity:depends("tls_type","psk")
identity.parse = parseFunctionFactory(Value, enabled, protocol, "mqtt", nil, nil, tls_enabled, tls_type, "psk")

use_credentials = s:option(Flag, "use_credentials", translate("Use credentials"), translate("Use credentials to connect to MQTT broker"))
use_credentials:depends("protocol", "mqtt")

username = s:option(Value, "username", translate("Username"), translate("Username to use when connecting to MQTT broker"))
username:depends("use_credentials", "1")
username.datatype = "lengthvalidation(0, 128)"
username.parse = parseFunctionFactory(Value, enabled, protocol, "mqtt", nil, nil, use_credentials, nil, nil)

password = s:option(Value, "password", translate("Password"), translate("Password to use when connecting to MQTT broker"))
password:depends("use_credentials", "1")
password.password = true
password.noautocomplete = true
password.datatype = "lengthvalidation(0, 128)"
password.parse = parseFunctionFactory(Value, enabled, protocol, "mqtt", nil, nil, use_credentials, nil, nil)

return m
