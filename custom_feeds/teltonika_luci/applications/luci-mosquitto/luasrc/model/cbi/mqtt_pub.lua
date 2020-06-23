m = Map("mqtt_pub", translate("MQTT Publisher"), translate(""))

local s2 = m:section(NamedSection, "mqtt_pub", "mqtt_pub",  translate(""), "")

enabled_pub = s2:option(Flag, "enabled", "Enable", "Select to enable MQTT publisher")

remote_addr = s2:option(Value, "remote_addr", "Hostname", "Specify address of the broker")
remote_addr:depends("enabled", "1")
remote_addr.datatype = "host"
remote_addr.parse = function(self, section, novld, ...)
	local enabled = luci.http.formvalue("cbid.mqtt_pub.mqtt_pub.enabled")
	local value = self:formvalue(section)
	if enabled and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: hostname is empty")
	end
	Value.parse(self, section, novld, ...)
end

remote_port = s2:option(Value, "remote_port", "Port", "Specify port of the broker")
remote_port:depends("enabled", "1")
remote_port.default = "1883"
remote_port.datatype = "port"
remote_port.parse = function(self, section, novld, ...)
	local enabled = luci.http.formvalue("cbid.mqtt_pub.mqtt_pub.enabled")
	local value = self:formvalue(section)
	if enabled and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: port is empty")
	end
	Value.parse(self, section, novld, ...)
end

remote_username = s2:option(Value, "username", "Username", "Specify username of remote host")
remote_username:depends("enabled", "1")
remote_username.parse = function(self, section, novld, ...)
	local enabled = luci.http.formvalue("cbid.mqtt_pub.mqtt_pub.enabled")
	local pass = luci.http.formvalue("cbid.mqtt_pub.mqtt_pub.password")
	local value = self:formvalue(section)
	if enabled and pass ~= nil and pass ~= "" and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: username is empty but password is not")
	end
	Value.parse(self, section, novld, ...)
end

remote_password = s2:option(Value, "password", "Password", "Specify password of remote host. Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:; []. At least 5 characters.")
remote_password:depends("enabled", "1")
remote_password.password = true
remote_password.datatype = "password(1)"
remote_password.parse = function(self, section, novld, ...)
	local enabled = luci.http.formvalue("cbid.mqtt_pub.mqtt_pub.enabled")
	local user = luci.http.formvalue("cbid.mqtt_pub.mqtt_pub.username")
	local value = self:formvalue(section)
	if enabled and user ~= nil and user ~= "" and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: password is empty but username is not")
	end
	Value.parse(self, section, novld, ...)
end

tls_enabled = s2:option(Flag, "tls", "TLS", "Select to enable TLS encryption")
tls_enabled:depends("enabled", "1")

tls_insecure = s2:option(Flag, "tls_insecure", "Allow insecure connection", "Allow not verifying server authenticity")
tls_insecure:depends({enabled = "1", tls = "1"})

tls_cafile = s2:option(FileUpload, "cafile", "CA file", "")
tls_cafile:depends({enabled = "1", tls = "1"})

tls_certfile = s2:option(FileUpload, "certfile", "Certificate file", "")
tls_certfile:depends({enabled = "1", tls = "1"})

tls_keyfile = s2:option(FileUpload, "keyfile", "Key file", "")
tls_keyfile:depends({enabled = "1", tls = "1"})

return m
