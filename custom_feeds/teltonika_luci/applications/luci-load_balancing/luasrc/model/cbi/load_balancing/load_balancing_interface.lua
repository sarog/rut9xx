-- ------ extra functions ------ --

function policy_check() -- check to see if any policy names exceed the maximum of 15 characters
	uci.cursor():foreach("balancing", "policy",
		function (section)
			if string.len(section[".name"]) > 15 then
				toolong = 1
				err_name_list = err_name_list .. section[".name"] .. " "
			end
		end
	)
end

function policy_warn() -- display status and warning messages at the top of the page
	if toolong == 1 then
		return "<font color=\"ff0000\"><strong>WARNING: Some policies have names exceeding the maximum of 15 characters!</strong></font>"
	else
		return ""
	end
end

function get_conn_name(self, section)
	local interfaces = {
		{moulage=true, ifname="3g-ppp", genName="Mobile", type="3G"},
		{moulage=true, ifname="eth2", genName="Mobile", type="3G"},
		{moulage=true, ifname="usb0", genName="WiMAX", type="WiMAX"},
		{moulage=true, ifname="eth1", genName="Wired", type="vlan"},
		{moulage=true, ifname="wlan0", genName="WiFi", type="wifi"},
		{moulage=true, ifname="none", genName="Mobile bridged", type="3G"},
		{moulage=true, ifname="wwan0", genName="Mobile", type="3G"},
		{moulage=true, ifname="wm0", genName="WiMAX", type="WiMAX"},
		{moulage=true, ifname="wwan-usb0", genName="USB Modem", type="USB"},
	}

	local wan_section = self.map.uci:get(self.config, section, "interface")
	local ifname = self.map.uci:get("network", wan_section, "ifname")

	if ifname then
		for a , b in ipairs(interfaces) do
			if b.ifname == ifname then
				return b.genName
			end
		end
	end
end

-- ------ policy configuration ------ --

ds = require "luci.dispatcher"
sys = require "luci.sys"
ut = require "luci.util"
local uci = require "luci.model.uci".cursor()
local message


toolong = 0
err_name_list = " "
policy_check()


m = Map("load_balancing", translate("Load Balancing Configuration"), translate(policy_warn()))


mwan_policy = m:section(TypedSection, "policy", translate("Policies"))
	mwan_policy.addremove = true
	mwan_policy.anonymous = false
	mwan_policy.dynamic = false
	mwan_policy.sectionhead = "Policy"
	mwan_policy.sortable = true
	mwan_policy.template = "cbi/tblsection"
	mwan_policy.extedit = ds.build_url("admin", "network", "balancing", "policy", "%s")

	function mwan_policy.parse(self, novld)
		local crval = "cbi.cts." .. self.config .. "." .. self.sectiontype
		local origin, name = next(self.map:formvaluetable(crval))
		-- If such a name already exists, throw an error to the user
		if name then
			if self:cfgvalue(name) then
				message = "err: "
				message = message .. "\'" .. (name or "") .. "\' already exists."
				name = nil
			elseif name:match("%W") then
				message = "err: "
				message = message .. "Name \'" .. (name or "") .. "\' contains special characters."
				name = nil
			end
		end
		TypedSection.parse(self, novld)
		self.invalid_cts = false -- Prevents 'Invalid value' message next to add button
		m.message = message
	end

	function mwan_policy.create(self, section)
		local created = TypedSection.create(self, section)
		if created then
			m.uci:save("load_balancing")
			luci.http.redirect(ds.build_url("admin", "network", "balancing", "policy", section))
		end
	end

	function mwan_policy.remove(self, section)
		local sections = m.uci:get_list(self.config, section, "use_member")

		for i, sec in ipairs(sections) do
			TypedSection.remove(self, sec)
		end
	end


use_member = mwan_policy:option(DummyValue, "use_member", translate("Members assigned"))
	use_member.rawhtml = true
	function use_member.cfgvalue(self, s)
		local tab, str = self.map:get(s, "use_member"), ""
		local name

		if tab then
			tab = type(tab) == "table" and tab or { tab }
			for k,v in pairs(tab) do
				name = get_conn_name(self, v) or "-"
				if name then
					str = string.format("%s%s<br />", str, name)
				end
			end
			return str
		else
			return "&#8212;"
		end

	end

use_member = mwan_policy:option(DummyValue, "use_member", translate("Ratio"))
	use_member.rawhtml = true
	function use_member.cfgvalue(self, s)
		local tab, str = self.map:get(s, "use_member"), ""
		local name

		if tab then
			tab = type(tab) == "table" and tab or { tab }
			for k,v in pairs(tab) do
				name = self.map.uci:get(self.config, v, "weight") or ""
				if name then
					str = string.format("%s%s<br />", str, name)
				end
			end
			return str
		else
			return "&#8212;"
		end

	end

mwan_rule = m:section(TypedSection, "rule", translate("Rules"))
	mwan_rule.addremove = true
	mwan_rule.anonymous = false
	mwan_rule.dynamic = false
	mwan_rule.sectionhead = "Rule"
	mwan_rule.sortable = true
	mwan_rule.template = "cbi/tblsection"
	mwan_rule.extedit = ds.build_url("admin", "network", "balancing",  "rule", "%s")

	function mwan_rule.parse(self, novld)
		local crval = "cbi.cts." .. self.config .. "." .. self.sectiontype
		local origin, name = next(self.map:formvaluetable(crval))
		-- If such a name already exists, throw an error to the user
		if name then
			if self:cfgvalue(name) then
				message = "err: "
				message = message .. "\'" .. (name or "") .. "\' already exists."
				name = nil
			elseif name:match("%W") then
				message = "err: "
				message = message .. "Name \'" .. (name or "") .. "\' contains special characters."
				name = nil
			end
		end
		TypedSection.parse(self, novld)
		self.invalid_cts = false -- Prevents 'Invalid value' message next to add button
		m.message = message
	end

	function mwan_rule.create(self, section)
		local created = TypedSection.create(self, section)
		if created then
			m.uci:save("load_balancing")
			luci.http.redirect(ds.build_url("admin", "network", "balancing", "rule", section))
		end
	end


src_ip = mwan_rule:option(DummyValue, "src_ip", translate("Source address"))
	src_ip.rawhtml = true
	function src_ip.cfgvalue(self, s)
		return self.map:get(s, "src_ip") or "&#8212;"
	end

src_port = mwan_rule:option(DummyValue, "src_port", translate("Source port"))
	src_port.rawhtml = true
	function src_port.cfgvalue(self, s)
		return self.map:get(s, "src_port") or "&#8212;"
	end

dest_ip = mwan_rule:option(DummyValue, "dest_ip", translate("Destination address"))
	dest_ip.rawhtml = true
	function dest_ip.cfgvalue(self, s)
		return self.map:get(s, "dest_ip") or "&#8212;"
	end

dest_port = mwan_rule:option(DummyValue, "dest_port", translate("Destination port"))
	dest_port.rawhtml = true
	function dest_port.cfgvalue(self, s)
		return self.map:get(s, "dest_port") or "&#8212;"
	end

proto = mwan_rule:option(DummyValue, "proto", translate("Protocol"))
	proto.rawhtml = true
	function proto.cfgvalue(self, s)
		return self.map:get(s, "proto") or "all"
	end

use_policy = mwan_rule:option(DummyValue, "use_policy", translate("Policy assigned"))
	use_policy.rawhtml = true
	function use_policy.cfgvalue(self, s)
		return self.map:get(s, "use_policy") or "&#8212;"
	end

return m
