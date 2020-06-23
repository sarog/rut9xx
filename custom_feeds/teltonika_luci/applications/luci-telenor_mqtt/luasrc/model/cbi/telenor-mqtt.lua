local sys = require "luci.sys"

m = Map("telenor_mqtt", translate("Telenor Cloud MQTT connection"))
s = m:section(NamedSection, "telenor_mqtt", "telenor_mqtt", translate("Telenor Cloud MQTT connection settings"))

enabled = s:option(Flag, "enabled", "Enable", translate("Enable Telenor connection"))
enabled.default = "0"
enabled.rmempty = false

host = s:option(Value, "telenor_ip", "Host", translate("Host address"))
host.datatype = "host"

port = s:option(Value, "telenor_port", "Port", translate("Port"))
port.datatype = "port"

thing = s:option(Value, "telenor_id", "Thing Name", translate("Thing Name of Telenor Cloud"))
--thing.datatype = "host"

int_check = s:option(Value, "sleep", "Interval", translate("Time interval in seconds to send info to cloud"))
int_check.datatype = "range(5,600000)"

enabled_get = s:option(Flag, "enable_setting", "Receive messages", translate("Enable receiving messages from cloud and turning relay on/off"))

set_val = s:option(Value, "setting_value", "Setting Value", translate("Name of the value set in Telenor App Board"))
set_val.datatype = "string_not_empty"
set_val:depends("enable_setting", "1")

FileUpload.size = "262144"
FileUpload.sizetext = translate("Selected file is too large, max 256 KiB")
FileUpload.sizetextempty = translate("Selected file is empty")

o = s:option( FileUpload, "ca_file", translate("Certificate authority"), translate("The digital certificate verifies the ownership of a public key by the named subject of the certificate"))

o = s:option( FileUpload, "cert_file", translate("Client certificate"), translate("Identify a client or a user, authenticating the client to the server and establishing precisely who they are"))

o = s:option( FileUpload, "key_file", translate("Client key"), translate("It has been generated for the same purpose as client certificate"))


return m


