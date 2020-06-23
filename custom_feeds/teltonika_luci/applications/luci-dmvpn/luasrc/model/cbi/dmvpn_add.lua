
local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()
local nw  = require "luci.model.network"
local ntm = require "luci.model.network".init()
local utl = require "luci.util"
local bus = require "ubus"
local ubus = bus.connect()
require "luci.fs"

local m = Map("dmvpn", translate("DMVPN"))

local dmvpn_instance = m:section( TypedSection, "dmvpn", translate("DMVPN Configuration"), translate("") )
	dmvpn_instance.addremove = true
	dmvpn_instance.template = "dmvpn/tblsection"
	dmvpn_instance.novaluetext = translate("There are no DMVPN configurations yet")
	dmvpn_instance.extedit = luci.dispatcher.build_url("admin", "services", "vpn", "dmvpn", "%s")
	dmvpn_instance.defaults = {enabled = "0"}
	dmvpn_instance.sectionhead = "Tunnel name"

	function dmvpn_instance.create(self, section)
		local network_static_table = {}
		local network_table = {}
		local quagga_table = {}
		local ipsec_conn_table = {}

		local instance_count = 0
		m.uci:foreach(self.config, "dmvpn", function(sections)
			instance_count = instance_count + 1
		end)

		if instance_count >= 5 then
			m.message = "err: Can't create more instances. Only 5 DMVPN instances are allowed"
		elseif string.len(section) > 10 then
			m.message = "err: Can't create instace. Instance name length can't be longer than 10 characters."
		else
			local created = TypedSection.create(self, section)

			if created then
				network_static_table["proto"] = "static"
				network_static_table["ifname"] = "@"..section
				m.uci:section("network", "interface", section.."_static", network_static_table)

				network_table["proto"] = "gre"
				network_table["zone"] = "gre"
				network_table["disabled"] = "1"
				network_table["auto"] = "0"
				m.uci:section("network", "interface", section, network_table)
				m.uci:save("network")

				quagga_table["enabled"] = "0"
				quagga_table["interface"] = "gre4-"..section
				quagga_table["ipsec_support"] = "1"
				quagga_table["ipsec_instance"] = section.."_dmvpn"
				m.uci:section("quagga", "nhrp_instance", section.."_dmvpn", quagga_table)
				m.uci:save("quagga")

				ipsec_conn_table["mode"] = "start"
				ipsec_conn_table["ipsec_type"] = "transport"
				ipsec_conn_table["aggressive"] = "no"
				ipsec_conn_table["keyexchange"] = "ikev1"
				ipsec_conn_table["leftprotoport"] = "gre"
				ipsec_conn_table["rightprotoport"] = "gre"
				ipsec_conn_table["enabled"] = "0"
				ipsec_conn_table["auth"] = "psk"
				m.uci:section("strongswan", "conn", section.."_dmvpn", ipsec_conn_table)
				m.uci:save("strongswan")

				m.message = translate("err: New DMVPN instance created successfully. Configure it now")
				return true
			end

			m.message = translate("err: Failed to create new DMVPN instance")
		end

		return false
	end

local status = dmvpn_instance:option(Flag, "enabled", translate("Enabled"), translate("Make a rule active/inactive"))

	function set_ipsec_fw_rules(value)
		m.uci:set("firewall", "IPsecESP", "enabled", value)
		m.uci:set("firewall", "IPsecNAT", "enabled", value)
		m.uci:set("firewall", "IPsecIKE", "enabled", value)
		m.uci:commit("firewall")
	end

	function handle_ipsec_fw_rules_on_disable()
		local ipsec_enabled = false
		local IPsecESP = m.uci:get("firewall", "IPsecESP", "enabled") or "1"
		local IPsecNAT = m.uci:get("firewall", "IPsecNAT", "enabled") or "1"
		local IPsecIKE = m.uci:get("firewall", "IPsecIKE", "enabled") or "1"

		m.uci:foreach(status.config, "conn", function(sec)
			local enabled = status:formvalue(sec[".name"])
			if enabled and enabled == "1" then
				ipsec_enabled = true
			end
		end)

		if not ipsec_enabled then
			if IPsecESP ~= "0" or IPsecNAT ~= "0" or IPsecIKE ~= "0" then
				set_ipsec_fw_rules("0")
			end
		end
	end

	function status.on_enable(self, section)
		self.map.uci:set("dmvpn", section, "enabled", "1")
		self.map.uci:set("strongswan", section.."_dmvpn", "enabled", "1")
		self.map.uci:set("network", section, "disabled", "0")
		self.map.uci:set("network", section, "auto", "1")
		self.map.uci:set("network", section.."_static", "enabled", "1")
		self.map.uci:set("quagga", section.."_dmvpn", "enabled", "1")
		self.map.uci:set("quagga", "nhrp", "enabled", "1")

		set_ipsec_fw_rules("1")

		self.map.uci:commit("strongswan")
		self.map.uci:commit("network")
		self.map.uci:commit("quagga")
	end

	function status.on_disable(self, section)
		self.map.uci:set("dmvpn", section, "enabled", "0")
		self.map.uci:set("strongswan", section.."_dmvpn", "enabled", "0")
		self.map.uci:set("network", section, "disabled", "1")
		self.map.uci:set("network", section, "auto", "0")
		self.map.uci:set("network", section.."_static", "enabled", "0")
		self.map.uci:set("quagga", section.."_dmvpn", "enabled", "0")

		handle_ipsec_fw_rules_on_disable()

		self.map.uci:commit("strongswan")
		self.map.uci:commit("network")
		self.map.uci:commit("quagga")
	end

o = dmvpn_instance:option( DummyValue, "hub_address", translate("Hub address"), translate("DMVPN Hub's IP address or domain"))
	o.rawhtml = true

	function explode(delimiter, text)
		local text_arr, arr_length
		text_arr={}
		arr_length=0
		if(#text == 1) then
			return {text}
		end
		while true do
			l = string.find(text, delimiter, arr_length, true)
			if l ~= nil then
				table.insert(text_arr, string.sub(text, arr_length, l - 1))
				arr_length = l + 1
			else
				table.insert(text_arr, string.sub(text, arr_length))
				break
			end
		end
		return text_arr
	end

	function o.cfgvalue(self, section)
		local config_mode = self.map:get(section, "config_mode") or "-"
		if config_mode == "spoke" then
			return self.map:get(section, self.option) or "Not set"
		elseif config_mode == "hub" then
			local is_addr = self.map.uci:get("network", section, "ipaddr") or ""
			local tunlink = self.map.uci:get("network", section, "tunlink") or ""

			if string.match(is_addr, "(%d+)%.(%d+)%.(%d+)%.(%d+)") then
				return is_addr
			else
				local addrs = ubus:call("network.interface.%s" % tunlink,
					"status", { })

				if addrs then
					if addrs["ipv4-address"] and #addrs["ipv4-address"] > 0 and addrs["ipv4-address"][1].address ~= "" then
						return addrs["ipv4-address"] and #addrs["ipv4-address"] > 0 and addrs["ipv4-address"][1].address
					else
						return "Not set"
					end
				else
					return "Not set"
				end
			end
		else
			return config_mode
		end
	end

o = dmvpn_instance:option( DummyValue, "config_mode", translate("Configuration mode"), translate("Mode of configuration (Hub or Spoke)"))

	function o.cfgvalue(self, section)
		local config_mode = self.map:get(section, self.option) or "Unavailable"
		if config_mode == "spoke" then
			return "Spoke"
		elseif config_mode == "hub" then
			return "Hub"
		else
			return config_mode
		end
	end

return m
