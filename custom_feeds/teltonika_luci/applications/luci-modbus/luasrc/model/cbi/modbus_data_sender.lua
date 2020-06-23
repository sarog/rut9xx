local uci = require "luci.model.uci".cursor()
local ds = require "luci.dispatcher"
local sys = require "luci.sys"

m = Map("modbus_data_sender", translate("Modbus data sender"), translate("Modbus data to server function allows to send data collected from modbus slaves to remote server"))

s = m:section(TypedSection, "data_sender", translate("Modbus data senders"))
s.template  = "cbi/tblsection"

s.addremove = true
s.anonymous = true
s.sortable  = false
s.extedit   = ds.build_url("admin/services/modbus/modbus_data_sender/%s")
s.novaluetext = translate("No data senders created yet")

name = s:option(DummyValue, "name", translate("Name"), translate("Name of the data sender. Used for easier data senders management purposes only (optional)"))
name.width   = "10%"
function name.cfgvalue(self, s)
	local set_name = self.map:get(s, "name")
	if set_name then
		if #set_name < 24 then
			return set_name
		else
			return string.sub(set_name,1,24) .. "..."
		end
	else
		return "N/A"
	end
end

protocol = s:option(DummyValue, "protocol", translate("Protocol"), translate("Protocol used for sending the data to server"))
protocol.width   = "10%"
function protocol.cfgvalue(self, s)
	local set_protocol = self.map:get(s, "protocol")
	if set_protocol == "http" then
		return "HTTP(S)"
	elseif set_protocol == "mqtt" then
		return "MQTT"
	elseif set_protocol == "azure_mqtt" then
		return "Azure MQTT"
	else
		return "N/A"
	end
end

host = s:option(DummyValue, "host", translate("URL / Host / Connection string"), translate("URL for HTTP(S); Host for MQTT; Connection string for Azure MQTT"))
host.rawhtml = true
host.width   = "30%"

device = s:option(DummyValue, "device", translate("Device"), translate("Which slave devices data will be sent to the server by this sender (IP or Modbus ID)"))
device.rawhtml = true
device.width   = "15%"

function device.cfgvalue(self, s)
	local set_filtering = self.map:get(s, "device_filtering")
	local set_value

	if set_filtering == "slave_id" then
		set_value = self.map:get(s, "filter_slave_id")
	elseif set_filtering == "slave_ip" then
		set_value = self.map:get(s, "filter_slave_ip")
	else
		set_value = "All"
	end

	if set_value then
		if #set_value < 32 then
			return set_value
		else
			return string.sub(set_value,1,32) .. "..."
		end
	else
		return "N/A"
	end
end

period = s:option(DummyValue, "period", translate("Period"), translate("Interval for sending collected data to server (in seconds, 1-86400)"))
period.rawhtml = true
period.width   = "5%"
function period.cfgvalue(self, s)
	return self.map:get(s, "period") or "N/A"
end

enabled = s:option(Flag, "enabled", translate("Enabled"), translate("Check to enable the the sender"))
enabled.rawhtml = true
enabled.width   = "5%"

enabled.parse = function(self, section, novld)
	local v = self:formvalue(section)
	if v ~= "1" then
		Value.parse(self, section, novld)
		return
	end

	local xname = self.map.uci:get(self.config, section, "name")
	if type(xname) ~= "string" or xname == "" then
		self:add_error(section, "invalid", "Name cannot be empty")
	end

	local xjson_string = self.map.uci:get(self.config, section, "json_string")
	if type(xjson_string) ~= "string" or xjson_string == "" then
		self:add_error(section, "invalid", "JSON format string cannot be empty")
	end

	local xhost = self.map.uci:get(self.config, section, "host")
	if type(xhost) ~= "string" or xhost == "" then
		self:add_error(section, "invalid", "URL / Host / Connection string cannot be empty")
	end

	local xperiod = self.map.uci:get(self.config, section, "period")
	if type(xperiod) ~= "string" or xperiod == "" then
		self:add_error(section, "invalid", "Name cannot be empty")
	end

	local xdevice_filtering = self.map.uci:get(self.config, section, "device_filtering")

	if xdevice_filtering == "slave_id" then
		local xfilter_slave_id = self.map.uci:get(self.config, section, "filter_slave_id")
		if type(xfilter_slave_id) ~= "string" or xfilter_slave_id == "" then
			self:add_error(section, "invalid", "Filter Slave ID cannot be empty")
		end
	end

	if xdevice_filtering == "slave_ip" then
		local xfilter_slave_ip = self.map.uci:get(self.config, section, "filter_slave_ip")
		if type(xfilter_slave_ip) ~= "string" or xfilter_slave_ip == "" then
			self:add_error(section, "invalip", "Filter Slave IP cannot be empty")
		end
	end

	local xprotocol = self.map.uci:get(self.config, section, "protocol")
	if xprotocol == "mqtt" then
		local xport = self.map.uci:get(self.config, section, "port")
		if type(xport) ~= "string" or xport == "" then
			self:add_error(section, "invalid", "Port cannot be empty")
		end

		local xkeepalive = self.map.uci:get(self.config, section, "keepalive")
		if type(xkeepalive) ~= "string" or xkeepalive == "" then
			self:add_error(section, "invalid", "Keepalive cannot be empty")
		end

		local xtopic = self.map.uci:get(self.config, section, "topic")
		if type(xtopic) ~= "string" or xtopic == "" then
			self:add_error(section, "invalid", "Topic cannot be empty")
		end

		local xtls_enabled = self.map.uci:get(self.config, section, "tls_enabled")

		if xtls_enabled == "1" then

			local xtls_type = self.map.uci:get(self.config, section, "tls_type")

			if xtls_type == "cert" then
				local xcafile = self.map.uci:get(self.config, section, "cafile")
				if type(xcafile) ~= "string" or xcafile == "" then
					self:add_error(section, "invalid", "CA cannot be empty")
				end

				local xcertfile = self.map.uci:get(self.config, section, "certfile")
				if type(xcertfile) ~= "string" or xcertfile == "" then
					self:add_error(section, "invalid", "Certification cannot be empty")
				end

				local xkeyfile = self.map.uci:get(self.config, section, "keyfile")
				if type(xkeyfile) ~= "string" or xkeyfile == "" then
					self:add_error(section, "invalid", "Key cannot be empty")
				end
			end

			if xtls_type == "psk" then
				local xpsk = self.map.uci:get(self.config, section, "psk")
				if type(xpsk) ~= "string" or xpsk == "" then
					self:add_error(section, "invalid", "PSK cannot be empty")
				end

				local xidentity = self.map.uci:get(self.config, section, "identity")
				if type(xidentity) ~= "string" or xidentity == "" then
					self:add_error(section, "invalid", "Identity cannot be empty")
				end
			end

		end

	end

	Value.parse(self, section, novld)
end

return m
