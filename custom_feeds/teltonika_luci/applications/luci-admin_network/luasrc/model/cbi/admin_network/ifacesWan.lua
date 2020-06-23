local function debug(string, ...)
	luci.sys.call(string.format("/usr/bin/logger -t Webui \"%s\"", string.format(string, ...)))
end

local ut = require "luci.util"
local nw = require "luci.model.network"
local fw = require "luci.model.firewall"
local ntm = require "luci.model.network".init()
require "teltonika_lua_functions"
local max_priority = 100
local interfaces = {
	{moulage=true, ifname="3g-ppp", genName="Mobile", type="3G"},
	{moulage=true, ifname="eth2", genName="Mobile", type="3G"},
	{moulage=true, ifname="usb0", genName="WiMAX", type="WiMAX"},
	{moulage=true, ifname="eth1", genName="Wired", type="vlan"},
	{moulage=true, ifname="wlan0", genName="WiFi", type="wifi"},
	{moulage=true, ifname="none", genName="Mobile bridged", type="3G"},
	{moulage=true, ifname="wwan0", genName="Mobile", type="3G"},
	{moulage=true, ifname="wm0", genName="WiMAX", type="WiMAX"},
	{moulage=true, ifname="wwan-usb0", genName="USB Modem", type="USB"}
}

m = Map("network", "WAN", translate("Your WAN configuration determines how the router will be connecting to the internet."))
-- 	m:chain("wireless")
-- 	m:chain("firewall")
-- 	m:chain("multiwan")
nw.init(m.uci)
fw.init(m.uci)

local bridge_mode = m.uci:get("network", "ppp", "method")
local vid = m.uci:get("system", "module", "vid")
local pid = m.uci:get("system", "module", "pid")
local bridge_on = false
local module = ""

if vid == "2C7C" and pid == "0125" then
	module = "EC25"
elseif vid == "05C6" and pid == "9215" then
	module = "EC20"
end

if bridge_mode and bridge_mode == "bridge" or bridge_mode == "pbridge" then
	bridge_on = true
end

m.bridge_on = bridge_on -- kintamasis tempalate'ams


wan_sec = m:section(TypedSection, "interface", translate("Operation Mode"))
	wan_sec.template = "admin_network/tblsection"
	wan_sec.sortable = true
	wan_sec.anonymous = true
	wan_sec.extedit = luci.dispatcher.build_url("admin/network/wan/edit/%s")

	function wan_sec.cfgsections(self)
		local sections = {}
		local name
		self.map.uci:foreach(self.map.config, self.sectiontype,
			function (section)
				name = self:checkscope(section[".name"])
				if name and name:match("wan")  then
					table.insert(sections, name)
				end
			end)
		return sections
	end

main_wan = wan_sec:option(Flag, "main", translate("Main WAN"))
	main_wan.template = "admin_network/fvalue_main"
	main_wan.default = "wan"

	function main_wan.cfgvalue(self, section)
		if section == "wan" then
			return "1"
		end
	end

	function main_wan.write(self, section, value)
		if bridge_mode and bridge_mode == "bridge" then
			local mobile_section = get_wan_section("type", "mobile")
			if mobile_section ~= value then
				self.map:del(section, "enabled")
				self.map:del(section, "disabled")
			end
		else
			self.map:del(section, "enabled")
			self.map:del(section, "disabled")
		end
		self.map:del(section, "metric")
	end

	--Custom parse function calling write function
	function main_wan.parse(self, section)
		local fvalue = luci.http.formvalue("main_wan")
			if fvalue ~= self.default and fvalue == section then
				self:write(section, fvalue)
			end
	end

back_wan = wan_sec:option(Flag, "enabled", translate("Mode"))
	back_wan.rmempty = false
	back_wan.forcewrite = true

	function back_wan.write(self, section, value)
		local main_wan = luci.http.formvalue("main_wan") == section and true or false

		if (value and value == "1") and not main_wan then
			self.map:del(section, self.option)
			self.map:set(section, "metric", "10")
			self.map:del(section, "disabled")
		elseif not main_wan then
			self.map:set(section, "disabled", "1")
			self.map:set(section, self.option, "0")
			self.map:del(section, "metric")
		end
	end

	function back_wan.cfgvalue(self, section)
		local enabled = self.map.uci:get("load_balancing", "general", "enabled") or "0"
		self.hardDisabled = false

		if bridge_on then
			local mobile_section = get_wan_section("type", "mobile")
			if mobile_section == "wan" then
				self.hardDisabled = true
			elseif mobile_section == section then
				self.hardDisabled = true
			end
		end

		if section ~= "wan" then
			return self.map:get(section, self.option) or "1"
		else
			self.hardDisabled = true
			if enabled == "1" then
				return 	self.map:get(section, self.option) or "1"
			end
		end
	end

ifname = wan_sec:option(DummyValue, "ifname", translate("Interface Name"))

	function ifname.cfgvalue(self, section)
		local value
		if self.tag_error[section] then
			value = self:formvalue(section)
		else
			value = self.map:get(section, self.option)
		end

		if not value then
			return nil
		else
			local name = "-"

			for n, i in ipairs(interfaces) do
				if i.ifname == value then
					int_name=i.genName
					return int_name.." ("..section:upper()..")"
				end
			end
			return name
		end
	end

proto = wan_sec:option(DummyValue, "proto", translate("Protocol"))

	function proto.cfgvalue(self, section)
		local value
		if self.tag_error[section] then
			value = self:formvalue(section)
		else
			value = self.map:get(section, self.option)
		end

		if not value then
			return "-"
		else
			if value == "dhcp" then
				return string.upper(value)
			elseif value == "none" then
				local ifname = self.map:get(section,"ifname")
				local proto = "none"
				m.uci:foreach("network", "interface", 
					function(s)
					if s.ifname and s.ifname == ifname then
						proto =  s.proto
					end
				end)
				if proto == "qmi2" then
					return "QMI"
				elseif proto == "3g" then
					return "PPP"
				end
			else
				return value:gsub("^%l", string.upper)
			end
		end
	end

ip_v4 = wan_sec:option(DummyValue, "ipaddr", translate("IP address"))

	function ip_v4.cfgvalue(self, section)

		local value = nil
		local intf
		local data = { ipaddrs = { } }
		local ppp_method = self.map.uci:get(self.config, "ppp", "method") or "nat"
		local ppp_ifname = self.map.uci:get(self.config, "ppp", "ifname") or nil

		local network = ntm:get_network(section)

		if network:ifname() then
			intf = ntm:get_interface(network:ifname())

			if intf and (ppp_method == "nat" or network:ifname() ~= ppp_ifname) then
				for _, a in ipairs(intf:ipaddrs()) do
					data.ipaddrs[#data.ipaddrs+1] = {
						addr      = a:host():string(),
						netmask   = a:mask():string(),
						prefix    = a:prefix()
					}
					value=data.ipaddrs[1].addr
				end
			else
				if ppp_method == "bridge" then
					value = "- (Bridge mode)"
				elseif ppp_method == "pbridge" then
					value = "- (Passthrough mode)"
				end
			end
		end
		if not value then
			return "-"
		else
			return value
		end
	end

function m.on_commit(self)
	local wan_main = luci.http.formvalue("main_wan")
	local wan_usb = luci.http.formvalue("cbid.network.wwan_usb0.enabled")

	local wan_en = luci.http.formvalue("cbid.network.wan.enabled")
	local wan2_en = luci.http.formvalue("cbid.network.wan2.enabled")
	local wan3_en = luci.http.formvalue("cbid.network.wan3.enabled")

-- workaround, kai is backup wan (mobile) perjungiama main wan (mobile), nepersileidzia konekcija ir nesusistato route kad atsirastu internetas
-- tam reikalinga papildoma opcija, kuri keiciasi ppp sekcijos ir perleidziama buna konekcija
	if wan_main and wan_main == "wan" then
		local ifname2 = luci.http.formvalue("cbid.network.wan2.ifname")
		local ifname3 = luci.http.formvalue("cbid.network.wan3.ifname")

		if (ifname2 and ifname2 == "Mobile (WAN2)" and wan2_en) or (ifname3 and ifname3 == "Mobile (WAN3)" and wan3_en) then
			m.uci:set("network", "ppp", "backup", "1")
		else
			m.uci:set("network", "ppp", "backup", "0")
		end

	elseif wan_main and wan_main == "wan2" then
		local ifname1 = luci.http.formvalue("cbid.network.wan.ifname")
		local ifname3 = luci.http.formvalue("cbid.network.wan3.ifname")

		if (ifname1 and ifname1 == "Mobile (WAN)" and wan_en) or (ifname3 and ifname3 == "Mobile (WAN3)" and wan3_en) then
			m.uci:set("network", "ppp", "backup", "1")
		else
			m.uci:set("network", "ppp", "backup", "0")
		end

	elseif wan_main and wan_main == "wan3" then
		local ifname1 = luci.http.formvalue("cbid.network.wan.ifname")
		local ifname2 = luci.http.formvalue("cbid.network.wan2.ifname")

		if (ifname1 and ifname1 == "Mobile (WAN)" and wan_en) or (ifname2 and ifname2 == "Mobile (WAN2)" and wan2_en) then
			m.uci:set("network", "ppp", "backup", "1")
		else
			m.uci:set("network", "ppp", "backup", "0")
		end
	end
	m.uci:commit("network")

	--luci.sys.exec("/usr/sbin/wan_rule_man.sh del_all")
	if wan_main then
		check_add_rule(wan_main)
	end
	
	if wan_usb then
		check_add_rule("wan_usb")
	end
	if wan_en then
		check_add_rule("wan")
	end
	if wan2_en then
		check_add_rule("wan2")
	end
	if wan3_en then
		check_add_rule("wan3")
	end
end
function check_add_rule(wan_iface)
	local wan_proto = ""
	local wan_stat_ip = ""
	local wan_ip = ""
	wan_proto = luci.util.trim(luci.sys.exec("uci -q get network.".. wan_iface ..".proto"))

	--[[if wan_proto == "static" then
		wan_stat_ip = luci.util.trim(luci.sys.exec("uci -q get network.".. wan_iface ..".ipaddr"))
		luci.sys.exec("/usr/sbin/wan_rule_man.sh add ".. wan_stat_ip .." ".. wan_iface .."")
	else
		wan_ip = luci.http.formvalue("cbid.network.".. wan_iface ..".ipaddr")
		if wan_ip and wan_ip ~= "-" then
			luci.sys.exec("/usr/sbin/wan_rule_man.sh add ".. wan_ip .." ".. wan_iface .."")
		end
	end]]--
end
function m.on_parse(self)
	if luci.http.formvalue("cbi.apply") then
		local main_section = luci.http.formvalue("main_wan")
		local mode = luci.http.formvalue("cbid.network.wan.mode")
		local use_ppp = false
		local sections = {}
		local tmp_table = {}
		local name, conf_enb, form_enb
		local wan_added = false
		local wan_changed = main_section ~= "wan" and true or false
		local enabled_sections = 0
		--I surenkame sekcijas kuriose atlikti pakeitimai
		m.uci:foreach(self.config, "interface",
			function (section)
				name = section[".name"]
				if name and name:match("wan")  then
					form_enb = luci.http.formvalue(string.format("cbid.%s.%s.enabled", self.config, name)) or "0"
					conf_enb = section.enabled or "1"

					if form_enb == "1" then
						enabled_sections = enabled_sections + 1
					end
					-- add to table if status changed: backup, main, or disbaled
					if main_section ~= name then
						if form_enb ~= conf_enb then
							--if status changed backup or disbaled
							tmp_table = {section = name, ifname = section.ifname, enabled = form_enb}
							table.insert(sections, tmp_table)
						elseif name == "wan" and wan_changed then
							tmp_table = {section = name, ifname = section.ifname, enabled = form_enb}
							table.insert(sections, tmp_table)
						end
					else
						--status changed to main
						if wan_changed then
							tmp_table = {section = name, ifname = section.ifname, enabled = "1"}
							table.insert(sections, tmp_table)
						end
					end

				end
			end)

		local enabled = m.uci:get("multiwan", "config", "enabled") or "0"
		local balancing_enabled = m.uci:get("load_balancing", "general", "enabled") or "0"

		if mode == "balanced" then
			if enabled == "1" then
				m.uci:set("multiwan", "config", "enabled", "0")
			end

			if balancing_enabled == "0" then
				m.uci:set("load_balancing", "general", "enabled", "1")
			end
		else
			if enabled_sections > 0 then
				if enabled == "0" then
					m.uci:set("multiwan", "config", "enabled", "1")
				end
			else
				if enabled == "1" then
					m.uci:set("multiwan", "config", "enabled", "0")
				end
			end

			if balancing_enabled == "1" then
				m.uci:set("load_balancing", "general", "enabled", "0")
			end
		end
		os.execute("rm -rf /tmp/.mwan")
		m.uci:save("multiwan")
		m.uci:commit("multiwan")
		m.uci:save("load_balancing")
		m.uci:commit("load_balancing")

		--Working part
		for n , i in ipairs(interfaces) do
			for sn , wan in ipairs(sections) do
				if i.ifname == wan.ifname then
					if i.type == "3G" then
						local ppp_ifname = m.uci:get(self.config, "ppp", "ifname")
						local ppp_enabled = m.uci:get(self.config, "ppp", "enabled")

						if wan.enabled == "1" then
							if ppp_enabled == "0" then
								m.uci:set(self.config, "ppp", "enabled", "1")
								m.uci:set(self.config, "ppp", "disabled", "0")
							end
						elseif wan.enabled == "0" then
							if ppp_enabled ~= "0" then
								m.uci:set(self.config, "ppp", "enabled", "0")
								m.uci:set(self.config, "ppp", "disabled", "1")
							end
						end

						if bridge_mode and wan_changed then
							if bridge_mode == "bridge" then
								if main_section == wan.section then
									luci.sys.exec("/usr/sbin/sim_switch set bridge")
								else
									luci.sys.exec("/usr/sbin/sim_switch disable bridge")
								end
							elseif bridge_mode == "pbridge" then
								if main_section == wan.section then
									luci.sys.exec("/usr/sbin/sim_switch set pbridge")
								else
									luci.sys.exec("/usr/sbin/sim_switch disable pbridge")
								end
 							end
						end
						m.uci:commit(self.config)
						
					elseif i.type == "USB" then
						local usb_ifname = m.uci:get(self.config, "ppp_usb", "ifname")
						local usb_enabled = m.uci:get(self.config, "ppp_usb", "enabled")

						if wan.enabled == "1" then
							if usb_enabled == "0" then
								m.uci:set(self.config, "ppp_usb", "enabled", "1")
								m.uci:set(self.config, "ppp_usb", "disabled", "0")
							end
						elseif wan.enabled == "0" then
							if usb_enabled ~= "0" then
								m.uci:set(self.config, "ppp_usb", "enabled", "0")
								m.uci:set(self.config, "ppp_usb", "disabled", "1")
							end
						end
						m.uci:commit(self.config)

					elseif i.type == "wifi" then
						local wifi_section, wifi_disabled
						local commit_wireless = false

						m.uci:foreach("wireless", "wifi-iface", function(ws)
							if ws.network and ws.network:match("wan") then
								wifi_section = ws[".name"]
								wifi_disabled = ws.disabled
							end
						end)

						if wifi_section then
							if main_section == wan.section then
								--Wifi parinktas kaip main wan
								m.uci:set("wireless", wifi_section, "network", "wan")
								commit_wireless = true
							elseif wan.section == "wan" then
								-- Wifi is main wan perjungtas i backup
								m.uci:set("wireless", wifi_section, "network", main_section)
								commit_wireless = true
							else
								-- Wifi ijungtas kaip backup
								m.uci:set("wireless", wifi_section, "network", wan.section)
								commit_wireless = true
							end

							if wan.enabled and wan.enabled == "1" then
								if wifi_disabled and wifi_disabled == "1" then
									m.uci:delete("wireless", wifi_section, "disabled")
									m.uci:set("wireless", wifi_section, "user_enable", "1")
									commit_wireless = true
								end
							elseif 	wan.enabled and wan.enabled == "0" then
								if not wifi_disabled then
									m.uci:set("wireless", wifi_section, "disabled", "1")
									m.uci:set("wireless", wifi_section, "user_enable", "0")
									commit_wireless = true
								end
							end

							if commit_wireless then
								m.uci:save("wireless")
								m.uci:commit("wireless")
							end
						end
					end
				end
			end
		end

		--Priklausomai nuo mode pasirinikimo ijungia arba isjungia load balancing
		on_off_balancing(mode)
	end
end

function on_off_balancing(mode)
	local enabled = m.uci:get("load_balancing", "general", "enabled") or "0"
	if mode == "balanced" then
		if enabled ~= "1" then
			m.uci:set("load_balancing", "general", "enabled", "1")
			m.uci:commit("load_balancing")
		end
	else
		if enabled ~= "0" then
			m.uci:delete("network", "ppp", "metric")
			m.uci:delete("network", "ppp_usb", "metric")
			m.uci:set("load_balancing", "general", "enabled", "0")
			m.uci:commit("load_balancing")
		end
	end
end

function m.on_after_all(self)
	local main_section = luci.http.formvalue("main_wan")
	local wan_changed = main_section ~= "wan" and true or false

	if wan_changed then
		nw:rename_network_custom("wan", "wan_tmp")
		nw:rename_network_custom(main_section, "wan")
		nw:rename_network_custom("wan_tmp", main_section)
		m.uci:reorder(self.config, "wan", 0)

		nw:rename_multiwan_section("wan", "wan_tmp")
		nw:rename_multiwan_section(main_section, "wan")
		nw:rename_multiwan_section("wan_tmp", main_section)

		handle_gre_tunlinks(main_section)
		set_priority(max_priority, false)
		rename_load_balancing(main_section)
		nw:commit("network")
		nw:commit("multiwan")
	end

	set_priority(max_priority, true)

	local wan_enabled
	require("uci")
	x = uci.cursor()
	zn = fw:get_zone("wan")

	m.uci:foreach("network", "interface", function(s)
		if s[".name"]:match("wan") then
			wan_enabled = x:get("network", s[".name"], "enabled")
			if not wan_enabled or (wan_enabled and wan_enabled == "1") then
				zn:add_network(s[".name"])
			else
				zn:del_network(s[".name"])
			end

			m.uci:set("network", "wan", "enabled", "1")

		end
	end)
	fw:commit("firewall")
	m.uci:save("network")
	m.uci:commit("network")

	local ifname = x:get("network", main_section, "ifname") or ""
	local curr_ifname = x:get("network", "wan", "ifname") or ""

	if ifname == "wlan0" or curr_ifname == "wlan0" then
		os.execute("(sleep 5; wifi up) &")
	end
	--if switching to main wired
	if curr_ifname == "eth1" then
		os.execute("(sleep 10; ifup wan) &")
	end
end

--pagal sekciju rikiavima wan confige, nustatome koks bus prioriti
function set_priority(max_priority, do_commit)
	local number = max_priority
	local metric = 10
	local ppp_ifname = m.uci:get("network", "ppp", "ifname") or ""
	local ppp_usb_ifname = m.uci:get("network", "ppp_usb", "ifname") or ""

	m.uci:foreach("network", "interface", function(s)
		if s[".name"]:match("wan") then
			m.uci:set("multiwan", s[".name"], "priority", number)
			number = number - 1
			if s[".name"] ~= "wan" and (not s.enabled or s.enabled == "1")  then
				m.uci:set("network", s[".name"], "metric", metric)
				if s["ifname"] == ppp_usb_ifname then
					m.uci:set("network", "ppp_usb", "metric", metric)
				end
				--Workaround del ec25 metric
				if (module == "EC25" or module == "EC20") and ppp_ifname == s.ifname then
					m.uci:set("network", "ppp", "metric", metric)
				end
				metric = metric + 10
			else
				if s["ifname"] == ppp_usb_ifname then
					m.uci:delete("network", "ppp_usb", "metric")
				elseif (module == "EC25" or module == "EC20") and ppp_ifname == s.ifname then
					m.uci:delete("network", "ppp", "metric")
				end
			end
		end
	end)

	if do_commit then
		m.uci:commit("multiwan")
		m.uci:commit("network")
	end
end

function rename_load_balancing(main_wan)
	m.uci:foreach("load_balancing", "member", function(sec)
		if sec.interface == "wan" then
			m.uci:set("load_balancing", sec[".name"], "interface", main_wan)
		elseif sec.interface == main_wan then
			m.uci:set("load_balancing", sec[".name"], "interface", "wan")
		end
	end)

	m.uci:commit("load_balancing")
end

function handle_gre_tunlinks(selected_main_wan)
	m.uci:foreach("network", "interface", function(gre_int)
		if gre_int.proto and gre_int.proto:match("gre") then
			if gre_int.tunlink == "wan" then
				m.uci:set("network", gre_int[".name"], "tunlink", selected_main_wan)
			elseif gre_int.tunlink == selected_main_wan then
				m.uci:set("network", gre_int[".name"], "tunlink", "wan")
			end
		end
	end)
end

return m
