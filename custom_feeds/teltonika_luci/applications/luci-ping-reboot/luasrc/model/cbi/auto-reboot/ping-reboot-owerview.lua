-- PING Reboot
local m, s, e, v, t, k, l, sim2
dsp = require "luci.dispatcher"

m = Map("ping_reboot", translate("Ping Reboot"))

s = m:section(TypedSection,"ping_reboot", translate("Ping Reboot Settings"))
	s.addremove = true
	s.template = "cbi/tblsection"
	s.anonymous = true
	s.extedit = dsp.build_url("admin", "services", "auto-reboot", "ping-reboot", "%s")

-- enable ping reboot option
e = s:option(Flag, "enable", translate("Enable"), translate("Enable ping reboot feature"))
e.rmempty = false

-- enable router reboot
v = s:option(DummyValue, "action", translate("Action"), translate("Action after the defined number of unsuccessfull retries (no echo reply for sent ICMP (Internet Control Message Protocol) packet received)"))
	function v.cfgvalue(self, section)
		local value = self.map:get(section, self.option)
		local actions = {"Reboot", "Modem restart", "Restart mobile connection", "(Re)register", "None", "Send SMS"}

		if value then
			value = tonumber(value)
			for n, i in ipairs(actions) do
				if value == n then
					value = i
				end
			end
		else
			value = "None"
		end

		return value
	end

-- ping inverval column and number validation
t = s:option(DummyValue, "time", translate("Interval (min)"), translate("Time interval in minutes between two ping packets"))
	function t.cfgvalue(self, section)
			return self.map:get(section, self.option) or "-"
	end
--Laikas iki rebooto po nesekmingo pingo

l = s:option(DummyValue, "time_out", translate("Ping timeout (sec)"), translate("Time interval (in seconds) to wait for ICMP (Internet Control Message Protocol) echo reply packet. Range [1 - 9999]"))
	function l.cfgvalue(self, section)
		return self.map:get(section, self.option) or "-"
	end
----Ping packet size------

z = s:option(DummyValue, "packet_size", translate("Packet size"), translate("Ping packet size in bytes. Range [0 - 1000]"))
	function z.cfgvalue(self, section)
		return self.map:get(section, self.option) or "-"
	end

-- number of retries and number validation
k = s:option(DummyValue, "retry", translate("Retry count"), translate("Number of failed to receive ICMP (Internet Control Message Protocol) echo reply packets. Range [1 - 9999]"))
	function k.cfgvalue(self, section)
		return self.map:get(section, self.option) or "-"
	end

-- host ping from wired
l = s:option(DummyValue, "host", translate("Hosts to ping"), translate("IP addresses or domain names which will be used to send ping packets to. E.g. 192.168.1.1 (or www.host.com if DNS server is configured correctly)"))

	function l.cfgvalue(self, section)
		local value
		local option

		for i=0, 2 do
			option = i > 0 and string.format("host%d", i) or "host"
			host = self.map:get(section, option)

			if host then
				if value then
					value = translate(string.format("%s, %s",value, host))
				else
					value = host
				end
			end
		end

		return value and value or "-"
	end

return m
