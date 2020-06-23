--[[
Copyright 2015 Teltonika
]]--

local ds = require "luci.dispatcher"
local fw = require "luci.model.firewall"

local m, s
local fw_section

function logger(string)
	os.execute("logger \"" ..string .. "\"")
end
m = Map("firewall", translate("DDOS Prevention"), translate(""))

------------------------------------------------------------------
--SYN Flood Protection--------------------------------------------
------------------------------------------------------------------

s = m:section(TypedSection, "defaults", translate("SYN Flood Protection"))

flood = s:option(Flag, "syn_flood", translate("Enable SYN flood protection"), translate("Makes router more resistant to SYN flood attacks"))

rate = s:option(Value, "synflood_rate", translate("SYN flood rate"), translate("Set rate limit (packets/second) for SYN packets above which the traffic is considered a flood."))
	rate.default = "25"

burst = s:option(Value, "synflood_burst", translate("SYN flood burst"), translate("Set burst limit for SYN packets above which the traffic is considered a flood if it exceeds the allowed rate."))
	burst.default = "50"

cookies = s:option(Flag, "tcp_syncookies", translate("TCP SYN cookies"), translate("Enable the use of SYN cookies."))

------------------------------------------------------------------
--Remote ICMP requests--------------------------------------------
------------------------------------------------------------------
local rule_section

m.uci:foreach("firewall", "rule", function(s)
	if s.name == "Allow-Ping" then
		rule_section = s[".name"]
	end
end)

if rule_section then
	local enb_limit = "0"

	s2 = m:section(NamedSection, rule_section, "rule", translate("Remote ICMP Requests"))

	icmp = s2:option(Flag, "enabled", translate("Enable ICMP requests"), translate("Blocks remote ICMP echo-request type"))
		icmp.rmempty = false

		function icmp.write(self, section, value)
			if value == "1" then
				m.uci:delete(self.config, section, self.option)
			else
				m.uci:set(self.config, section, self.option, "0")
			end
		end

		function icmp.cfgvalue(self, section)
			local value = m.uci:get(self.config, section, self.option)
			if value ~= "0" then
				value = "1"
			end
			return value
		end
	icmp_limit = s2:option(Flag, "icmp_limit", translate("Enable ICMP limit"), translate(""))
		icmp_limit.rmempty = false

		function icmp_limit.write(self, section, value)
			local drop_section

			enb_limit = value or "0"
			m.uci:foreach(self.config, "rule", function(sec)
				if sec.name == "Drop_icmp_packets" then
					drop_section = sec[".name"]
				end
			end)

			if value == "1" then
				if not drop_section then
					m.uci:section(self.config, "rule", nil, {
						src = 'wan',
						name = 'Drop_icmp_packets',
						proto = 'icmp',
						target = 'DROP',
					})
				end
			else
				if drop_section then
					m.uci:delete(self.config, drop_section)
				end
			end
		end

		function icmp_limit.cfgvalue(self, section)
			local limit = m.uci:get(self.config, section, "limit")
			local limit_burst = m.uci:get(self.config, section, "limit_burst")

			if limit or limit_burst then
				return "1"
			else
				return "0"
			end
		end

	icmp_period = s2:option(ListValue, "period", translate("Limit period"))
		icmp_period:value("second","Second")
		icmp_period:value("minute","Minute")
		icmp_period:value("hour","Hour")
		icmp_period:value("day","Day")

		function icmp_period.write(self, section, value)
			m.uci:set("ddos", "ddos", "icmp_period", value)
		end

		function icmp_period.cfgvalue(self, section)
			local value = m.uci:get("ddos", "ddos", "icmp_period")
			return value
		end

	icmp_rate = s2:option(Value, "limit", translate("Limit"), translate("Maximum average matching rate"))
		icmp_rate.default = "10"
		icmp_rate.datatype = "integer"
		icmp_rate.forcewrite = true

		function icmp_rate.write(self, section, value)
			local time = icmp_period:formvalue(section) or "second"
			m.uci:set("ddos", "ddos", "icmp_limit", value)
			m.uci:commit("ddos")
			if enb_limit == "1" then
				m.uci:set(self.config, section, self.option, value .. "/" .. time)
			else
				m.uci:delete(self.config, section, self.option)
			end
		end

		function icmp_rate.cfgvalue(self, section)
			local value = m.uci:get("ddos", "ddos", "icmp_limit")
			return value
		end

	icmp_burst = s2:option(Value, "limit_burst", translate("Limit burst"), translate("Maximum initial number of packets to match"))
		icmp_burst.default = "5"
		icmp_burst.datatype = "integer"
		icmp_burst.forcewrite = true


		function icmp_burst.write(self, section, value)
			m.uci:set("ddos", "ddos", "icmp_limit_burst", value)
			m.uci:commit("ddos")
			if enb_limit == "1" then
				m.uci:set(self.config, section, self.option, value)
			else
				m.uci:delete(self.config, section, self.option)
			end
		end

		function icmp_burst.cfgvalue(self, section)
			local value = m.uci:get("ddos", "ddos", "icmp_limit_burst")
			return value
		end
end

------------------------------------------------------------------
--SSH Attacks Prevention--------------------------------------------
------------------------------------------------------------------

m.uci:foreach("firewall", "rule", function(s)
	if s.name == "Enable_SSH_WAN" then
		rule_section = s[".name"]
	end
end)

if rule_section then
	local enb_limit = "0"

	s3 = m:section(NamedSection, rule_section, "rule", translate("SSH Attack Prevention"))

	ssh = s3:option(Flag, "ssh_limit", translate("Enable SSH limit"), translate("It limits SSH connections per period"))
		ssh.rmempty = false

		function ssh.write(self, section, value)
			enb_limit = value or "0"
		end

		function ssh.cfgvalue(self, section)
			local limit = m.uci:get(self.config, section, "limit")
			local limit_burst = m.uci:get(self.config, section, "limit_burst")

			if limit or limit_burst then
				return "1"
			else
				return "0"
			end
		end

	period = s3:option(ListValue, "period", translate("Limit period"))
		period:value("second","Second")
		period:value("minute","Minute")
		period:value("hour","Hour")
		period:value("day","Day")

		function period.write(self, section, value)
			m.uci:set("ddos", "ddos", "ssh_period", value)
		end

		function period.cfgvalue(self, section)
			local value = m.uci:get("ddos", "ddos", "ssh_period")
			return value
		end

	rate = s3:option(Value, "limit", translate("Limit"), translate("Maximum average matching rate"))
		rate.default = "10"
		rate.datatype = "integer"
		rate.forcewrite = true

		function rate.write(self, section, value)
			local time = period:formvalue(section) or "second"
			m.uci:set("ddos", "ddos", "ssh_limit", value)
			m.uci:commit("ddos")
			if enb_limit == "1" then
				m.uci:set(self.config, section, self.option, value .. "/" .. time)
			else
				m.uci:delete(self.config, section, self.option)
			end
		end

		function rate.cfgvalue(self, section)
			local value = m.uci:get("ddos", "ddos", "ssh_limit")
			return value
		end

	burst = s3:option(Value, "limit_burst", translate("Limit burst"), translate("Maximum initial number of packets to match"))
		burst.default = "5"
		burst.datatype = "integer"
		burst.forcewrite = true

		function burst.write(self, section, value)
			m.uci:set("ddos", "ddos", "ssh_limit_burst", value)
			m.uci:commit("ddos")
			if enb_limit == "1" then
				m.uci:set(self.config, section, self.option, value)
			else
				m.uci:delete(self.config, section, self.option)
			end
		end

		function burst.cfgvalue(self, section)
			local value = m.uci:get("ddos", "ddos", "ssh_limit_burst")
			return value
		end
end

------------------------------------------------------------------
--Http Attacks Prevention--------------------------------------------
------------------------------------------------------------------

m.uci:foreach("firewall", "rule", function(s)
	if s.name == "Enable_HTTP_WAN" then
		rule_section = s[".name"]
	end
end)

if rule_section then
	local enb_limit = "0"

	s4 = m:section(NamedSection, rule_section, "rule", translate("HTTP Attack Prevention"))

	http = s4:option(Flag, "http_limit", translate("Enable HTTP limit"), translate("It limits HTTP connections per period"))
		http.rmempty = false

		function http.write(self, section, value)
			enb_limit = value or "0"
		end

		function http.cfgvalue(self, section)
			local limit = m.uci:get(self.config, section, "limit")
			local limit_burst = m.uci:get(self.config, section, "limit_burst")

			if limit or limit_burst then
				return "1"
			else
				return "0"
			end
		end

	period = s4:option(ListValue, "period", translate("Limit period"))
		period:value("second","Second")
		period:value("minute","Minute")
		period:value("hour","Hour")
		period:value("day","Day")

		function period.write(self, section, value)
			m.uci:set("ddos", "ddos", "http_period", value)
		end

		function period.cfgvalue(self, section)
			local value = m.uci:get("ddos", "ddos", "http_period")
			return value
		end

	rate = s4:option(Value, "limit", translate("Limit"), translate("Maximum average matching rate"))
		rate.default = "10"
		rate.datatype = "integer"
		rate.forcewrite = true

		function rate.write(self, section, value)
			local time = period:formvalue(section) or "second"
			m.uci:set("ddos", "ddos", "http_limit", value)
			m.uci:commit("ddos")
			if enb_limit == "1" then
				m.uci:set(self.config, section, self.option, value .. "/" .. time)
			else
				m.uci:delete(self.config, section, self.option)
			end
		end

		function rate.cfgvalue(self, section)
			local value = m.uci:get("ddos", "ddos", "http_limit")
			return value
		end

	burst = s4:option(Value, "limit_burst", translate("Limit burst"), translate("Maximum initial number of packets to match"))
		burst.default = "10"
		burst.datatype = "integer"
		burst.forcewrite = true

		function burst.write(self, section, value)
			m.uci:set("ddos", "ddos", "http_limit_burst", value)
			m.uci:commit("ddos")
			if enb_limit == "1" then
				m.uci:set(self.config, section, self.option, value)
			else
				m.uci:delete(self.config, section, self.option)
			end
		end

		function burst.cfgvalue(self, section)
			local value = m.uci:get("ddos", "ddos", "http_limit_burst")
			return value
		end
end

------------------------------------------------------------------
--Http Attacks Prevention--------------------------------------------
------------------------------------------------------------------

m.uci:foreach("firewall", "rule", function(s)
	if s.name == "Enable_HTTPS_WAN" then
		rule_section = s[".name"]
	end
end)

if rule_section then
	local enb_limit = "0"

	s5 = m:section(NamedSection, rule_section, "rule", translate("HTTPS Attack Prevention"))

	http = s5:option(Flag, "https_limit", translate("Enable HTTPS limit"), translate("It limits HTTPS connections per period"))
		http.rmempty = false

		function http.write(self, section, value)
			enb_limit = value or "0"
		end

		function http.cfgvalue(self, section)
			local limit = m.uci:get(self.config, section, "limit")
			local limit_burst = m.uci:get(self.config, section, "limit_burst")

			if limit or limit_burst then
				return "1"
			else
				return "0"
			end
		end

	period = s5:option(ListValue, "period", translate("Limit period"))
		period:value("second","Second")
		period:value("minute","Minute")
		period:value("hour","Hour")
		period:value("day","Day")

		function period.write(self, section, value)
			m.uci:set("ddos", "ddos", "https_period", value)
		end

		function period.cfgvalue(self, section)
			local value = m.uci:get("ddos", "ddos", "https_period")
			return value
		end

	rate = s5:option(Value, "limit", translate("Limit"), translate("Maximum average matching rate"))
		rate.default = "10"
		rate.datatype = "integer"
		rate.forcewrite = true

		function rate.write(self, section, value)
			local time = period:formvalue(section) or "second"
			m.uci:set("ddos", "ddos", "https_limit", value)
			m.uci:commit("ddos")
			if enb_limit == "1" then
				m.uci:set(self.config, section, self.option, value .. "/" .. time)
			else
				m.uci:delete(self.config, section, self.option)
			end
		end

		function rate.cfgvalue(self, section)
			local value = m.uci:get("ddos", "ddos", "https_limit")
			return value
		end

	burst = s5:option(Value, "limit_burst", translate("Limit burst"), translate("Maximum initial number of packets to match"))
		burst.default = "10"
		burst.datatype = "integer"
		burst.forcewrite = true

		function burst.write(self, section, value)
			m.uci:set("ddos", "ddos", "https_limit_burst", value)
			m.uci:commit("ddos")
			if enb_limit == "1" then
				m.uci:set(self.config, section, self.option, value)
			else
				m.uci:delete(self.config, section, self.option)
			end
		end

		function burst.cfgvalue(self, section)
			local value = m.uci:get("ddos", "ddos", "https_limit_burst")
			return value
		end
end

return m
