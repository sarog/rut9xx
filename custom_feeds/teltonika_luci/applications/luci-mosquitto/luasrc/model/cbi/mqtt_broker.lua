require("luci.fs")
require("luci.config")

local fw = require "luci.model.firewall"
local deathTrap = { }

m = Map("mosquitto", translate("MQTT Broker"), translate(""))
m:chain("firewall")
fw.init(m.uci)

FileUpload.size = "1000000"
FileUpload.sizetext = translate("Selected file is too large, max 1 MB")
FileUpload.sizetextempty = translate("Selected file is empty")

local s2 = m:section(NamedSection, "mqtt", "mqtt", translate(""), "")

enabled = s2:option(Flag, "enabled", "Enable", "Select to enable MQTT")


local_port = s2:option( Value, "local_port", "Local Port",
"Specify local port which the MQTT will be listen to")
local_port.default = "1883"
local_port.datatype = "port"
local_port.parse = function(self, section, novld, ...)
	local enabled = luci.http.formvalue("cbid.mosquitto.mqtt.enabled")
	local value = self:formvalue(section)
	if enabled and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: local port is empty")
	end
	Value.parse(self, section, novld, ...)
end


ara = s2:option(Flag, "enable_ra", "Enable Remote Access",
"Select to enable remote access")
ara.rmempty = false

function ara.write(self, section)
	local fval = self:formvalue(section)
	local fport = local_port:formvalue(section)
	local needsPortUpdate = false
	local fwRuleInstName = "nil"

	if not deathTrap[1] then deathTrap[1] = true
	else return end

	if not fval then
		fval = "0"
	else
		fval = "1"
	end

	m.uci:foreach("firewall", "rule", function(z)
		if z.name == "Enable_MQTT_WAN" then
			fwRuleInstName = z[".name"]
			if z.dest_port ~= fport then
				needsPortUpdate = true
			end
			if z.enabled ~= fval then
				needsPortUpdate = true
			end
		end
	end)

	if needsPortUpdate == true then
		m.uci:set("firewall", fwRuleInstName, "dest_port", fport)
		m.uci:set("firewall", fwRuleInstName, "enabled", fval)
		m.uci:save("firewall")
	end

	if fwRuleInstName == "nil" then
		local wanZone = fw:get_zone("wan")
		if not wanZone then
			m.message = "Could not add firewall rule"
			return
		end
		local fw_rule = {
			name = "Enable_MQTT_WAN",
			target = "ACCEPT",
			proto = "tcp",
			dest_port = fport,
			enabled = fval
		}

		wanZone:add_rule(fw_rule)
		m.uci:save("firewall")
	end
end

function ara.cfgvalue(self, section)
	local fwRuleEn = false

	m.uci:foreach("firewall", "rule", function(z)
		if z.name == "Enable_MQTT_WAN" and z.enabled == "1" then
			fwRuleEn = true
		end
	end)
	if fwRuleEn then
		return self.enabled
	else
		return self.disabled
	end

end


local s2 = m:section( TypedSection, "mqtt", translate("Broker settings"), "")
s2:depends("settings", "broker")
s2:tab("security", translate("Security"))
s2:tab("client", translate("Bridge"))
s2:tab("misc",  translate("Miscellaneous"))
s2.template = "mqtt/tsection"
function s2.cfgsections(self)
	return {"mqtt"}
end


acl_file_path = s2:taboption("misc", FileUpload, "acl_file_path", "ACL File", "Select ACL file")
--acl_file_path.rmempty = false


password_file = s2:taboption("misc", FileUpload, "password_file", "Password File", "Uploads passwords/users file")
--password_file.rmepty = false


persistence = s2:taboption("misc", Flag, "persistence", "Persistence", " If true, connection, subscription and message data will be written to the disk")
--persistence.rmempty = false


allow_anonymous = s2:taboption("misc", Flag, "anonymous_access", "Allow Anonymous", "Allows anonymous access")
allow_anonymous.default = "1"
allow_anonymous.rmempty = false


client_enabled = s2:taboption("client", Flag, "client_enabled", "Enable", "Enable connection to remote bridge")
client_enabled.rmempty = false


connection_name = s2:taboption("client", Value, "connection_name", "Connection Name", "")
connection_name.datatype = "nospace"
connection_name:depends("client_enabled", "1")
--connection_name.rmempty=false
connection_name.parse = function(self, section, novld, ...)
	local bridgeEnabled = luci.http.formvalue("cbid.mosquitto.mqtt.client_enabled")
	local value = self:formvalue(section)
	if bridgeEnabled and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: connection name is empty")
	end
	Value.parse(self, section, novld, ...)
end


remote_address = s2:taboption("client", Value, "remote_addr", "Remote Address", "Select remote bridge address")
remote_address.datatype = "host"
--remote_address.rmempty = false
remote_address:depends("client_enabled", "1")
remote_address.parse = function(self, section, novld, ...)
	local bridgeEnabled = luci.http.formvalue("cbid.mosquitto.mqtt.client_enabled")
	local value = self:formvalue(section)
	if bridgeEnabled and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: remote address is empty")
	end
	Value.parse(self, section, novld, ...)
end


remote_port = s2:taboption("client", Value, "remote_port", "Remote Port", "Select remote port")
remote_port.datatype = "port"
--remote_port.rmempty = false
remote_port.default = "1883"
remote_port:depends("client_enabled", "1")
remote_port.parse = function(self, section, novld, ...)
	local bridgeEnabled = luci.http.formvalue("cbid.mosquitto.mqtt.client_enabled")
	local value = self:formvalue(section)
	if bridgeEnabled and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: remote port is empty")
	end
	Value.parse(self, section, novld, ...)
end


use_remote_tls = s2:taboption("client", Flag, "use_remote_tls", "Use Remote TLS/SSL", "Select to use TLS/SSL for remote connection")
-- use_remote_tls.rmempty = false
use_remote_tls:depends("client_enabled", "1")


bridge_ca_file = s2:taboption("client", FileUpload, "bridge_cafile", "Bridge CA File",
"Upload bridge CA file")
-- bridge_ca_file.rmempty = false
bridge_ca_file:depends("use_remote_tls", "1")


bridge_certfile = s2:taboption("client", FileUpload, "bridge_certfile", "Bridge CERT File",
"Upload bridge CERT file")
-- bridge_certfile.rmempty = false
bridge_certfile:depends("use_remote_tls", "1")


bridge_keyfile = s2:taboption("client", FileUpload, "bridge_keyfile", "Bridge Key File",
"Upload bridge Key file")
-- bridge_keyfile.rmempty = false
bridge_keyfile:depends("use_remote_tls", "1")


bridge_tls_version = s2:taboption("client", ListValue, "bridge_tls_version", "Bridge TLS version",
"Used bridge TLS version");
bridge_tls_version:depends("use_remote_tls", "1");
bridge_tls_version:value("tlsv1.1", "tlsv1.1");
bridge_tls_version:value("tlsv1.2", "tlsv1.2");


bridge_insecure = s2:taboption("client", Flag, "bridge_insecure", "Insecure",
"Do not verify if the hostname provided in the remote certificate matches the host/address being connected to")
bridge_insecure:depends("use_remote_tls", "1")


use_bridge_login = s2:taboption("client", Flag, "use_bridge_login", "Use Remote Bridge Login",
"Select to use login for bridge")
-- use_bridge_login.rmempty = false
use_bridge_login:depends("client_enabled", "1")


remote_clientid = s2:taboption("client", Value, "remote_clientid", "Remote ID", "Choose remote client ID")
remote_clientid:depends("use_bridge_login", "1")
-- remote_clientid.rmempty = false
remote_clientid.parse = function(self, section, novld, ...)
	local bridgeEnabled = luci.http.formvalue("cbid.mosquitto.mqtt.client_enabled")
	local bridgeLoginEnabled = luci.http.formvalue("cbid.mosquitto.mqtt.use_bridge_login")
	local value = self:formvalue(section)
	if bridgeEnabled and bridgeLoginEnabled and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: remote ID is empty")
	end
	Value.parse(self, section, novld, ...)
end


remote_username = s2:taboption("client", Value, "remote_username", "Remote Username",
"Choose remote user name")
-- remote_username.rmempty = false
remote_username:depends("use_bridge_login", "1")
remote_username.parse = function(self, section, novld, ...)
	local bridgeEnabled = luci.http.formvalue("cbid.mosquitto.mqtt.client_enabled")
	local bridgeLoginEnabled = luci.http.formvalue("cbid.mosquitto.mqtt.use_bridge_login")
	local value = self:formvalue(section)
	if bridgeEnabled and bridgeLoginEnabled and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: remote username is empty")
	end
	Value.parse(self, section, novld, ...)
end


remote_password = s2:taboption("client", Value, "remote_password", "Remote Password",
"Choose remote password. Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:; []")
-- remote_password.rmempty = false
remote_password:depends("use_bridge_login", "1")
remote_password.datatype = "password(1)"
remote_password.parse = function(self, section, novld, ...)
	local bridgeEnabled = luci.http.formvalue("cbid.mosquitto.mqtt.client_enabled")
	local bridgeLoginEnabled = luci.http.formvalue("cbid.mosquitto.mqtt.use_bridge_login")
	local value = self:formvalue(section)
	if bridgeEnabled and bridgeLoginEnabled and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: remote password is empty")
	end
	Value.parse(self, section, novld, ...)
end
remote_password.noautocomplete = true
remote_password.password = true


try_private = s2:taboption("client", Flag, "try_private", "Try Private", "Check if remote broker is another instance of a daemon")
-- try_private.rmempty = false
try_private:depends("client_enabled", "1")


cleansession = s2:taboption("client", Flag, "cleansession", "Clean Session", "Discard session state when connecting or disconnecting")
-- cleansession.rmempty = false
cleansession:depends("client_enabled", "1")


use_tls_ssl = s2:taboption("security", Flag, "use_tls_ssl", "Use TLS/SSL", "Mark to use TLS/SSL for connection")
use_tls_ssl.rmempty = false
function use_tls_ssl.write(self, section, value)
	self.map:set("mqtt", self.option, value)
end


ca_file = s2:taboption("security", FileUpload, "ca_file", "CA File", "Upload CA file");
ca_file:depends("use_tls_ssl", "1")
--ca_file.rmempty = true


cert_file = s2:taboption("security", FileUpload, "cert_file", "CERT File", "Upload CERT file");
cert_file:depends("use_tls_ssl", "1")
--cert_file.rmempty = true


key_file = s2:taboption("security", FileUpload, "key_file", "Key File", "Upload Key file");
key_file:depends("use_tls_ssl", "1")
--key_file.rmempty = true


tls_version = s2:taboption("security", ListValue, "tls_version", "TLS version", "Used TLS version");
tls_version:depends("use_tls_ssl", "1");
tls_version:value("tlsv1.1", "tlsv1.1");
tls_version:value("tlsv1.2", "tlsv1.2");
tls_version:value("all", "Support all");
tls_version.default = "all"


local st = m:section(TypedSection, "topic")
	st.addremove = true
	st.anonymous = true
	st.template = "mqtt/tblsection"
	st.novaluetext = "There are no topics created yet."


topic = st:option(Value, "topic", translate("Topic"), translate("Topic"))
	topic.rmempty = false


direction = st:option(ListValue, "direction", translate("Direction"), translate("The direction that the messages will be shared in"))
	direction:value("out", "OUT")
	direction:value("in", "IN")
	direction:value("both", "BOTH")
	direction.default = "out"


qos = st:option(Value, "qos", translate("QoS level "), translate("The publish/subscribe QoS level used for this topic"))
qos.rmempty=false
qos.default="0"
function qos:validate(value)
	if value ~= "0" and value ~= "1" and value ~= "2" then
		return nil, "QoS level must be either 0, 1 or 2"
	end
	return value
end


return m
