m = Map("modbusgateway", translate("MQTT Gateway"))

s = m:section(NamedSection, "gateway", "gateway", translate(""), "")

enabled = s:option(Flag, "enabled", "Enable", translate("Enable Modbus TCP"))
enabled.default = "0"
enabled.rmempty = false
enabled.template = "mqtt-gateway/js"

local function parseFunctionGenerator(errmsg)
	return function (self, section, novld, ...)
		e = enabled:formvalue(section)
		if e == "1" then
			local x = self:formvalue(section)
			if x == nil or x == "" then
				self:add_error(section, "invalid", errmsg)
			end
		end
		Value.parse(self, section, novld, ...)
	end
end

host = s:option(Value, "host", "Host", translate("MQTT broker hostname or IP"))
host.datatype = "host"
host.default = "127.0.0.1"
host.parse = parseFunctionGenerator("Host cannot be empty")

port = s:option(Value, "port", "Port", translate("MQTT broker port number"))
port.datatype = "port"
port.default = "1883"
port.parse = parseFunctionGenerator("Port cannot be empty")

request = s:option(Value, "request", "Request topic", translate("Request topic (alphanumeric characters only)"))
request.default = "request"
request.datatype = "lengthvalidation(1, 32, '^[a-zA-Z0-9]+$')"
request.parse = parseFunctionGenerator("Request topic cannot be empty")

response = s:option(Value, "response", "Response topic", translate("Response topic (alphanumeric characters only)"))
response.default = "response"
response.datatype = "lengthvalidation(1, 32, '^[a-zA-Z0-9]+$')"
response.parse = parseFunctionGenerator("Response topic cannot be empty")

user = s:option(Value, "user", "Username", translate("MQTT client username (leave empty if anonymous)"))

pass = s:option(Value, "pass", "Password", translate("MQTT client password (leave empty if anonymous)"))
pass.password = true
function pass:parse(section, novld, ...)
	local u = user:formvalue(section)
	local p = pass:formvalue(section)
	local uEmpty = (u == nil or u == "")
	local pEmpty = (p == nil or p == "")
	if (uEmpty and not pEmpty) or (not uEmpty and pEmpty) then
		self:add_error(section, "invalid", "Both username and password must be either empty or filled")
	end
	Value.parse(self, section, novld, ...)
end

return m
