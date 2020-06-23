-- ------ extra functions ------ --

function policy_check() -- check to see if this policy's name exceed the maximum of 15 characters
	polchar = string.len(arg[1])
	if polchar > 15 then
		toolong = 1
	end
end

function policy_warn() -- display status and warning messages at the top of the page
	if toolong == 1 then
		return translatef("err: WARNING: this policy's name is %s characters exceeding the maximum of 15!", polchar)
	else
		return nil
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

	if wan_section == nil then
		return nil
	end

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
local util = require("luci.util")
local dsp = require "luci.dispatcher"
arg[1] = arg[1] or ""

toolong = 0
policy_check()

m = Map("load_balancing", translate("WAN Policy Configuration - " .. arg[1]))
	m.message = policy_warn()
	m.redirect = dsp.build_url("admin", "network", "balancing", "configuration")


mwan_policy = m:section(TypedSection, "member", "")
	mwan_policy.template="load_balancing/balancing_tblsection"
	mwan_policy.addremove = true
	mwan_policy.sortable = true

	function mwan_policy.parse(self, novld)
		local deleted_section_id = nil
		if self.addremove then
			-- Remove
			local crval = REMOVE_PREFIX .. self.config
			local name = self.map:formvaluetable(crval)
			for k,v in pairs(name) do
				if k:sub(-2) == ".x" then
					k = k:sub(1, #k - 2)
				end
				if self:cfgvalue(k) and self:checkscope(k) then
					deleted_section_id = k
					self:remove(k)
				end
			end
		end

		local co
		for i, k in ipairs(self:cfgsections()) do
			AbstractSection.parse_dynamic(self, k)
			if self.map:submitstate() then
				Node.parse(self, k, novld)
			end
			AbstractSection.parse_optionals(self, k)
		end

		if self.addremove then
			-- Create
			local created
			local crval = CREATE_PREFIX .. self.config .. "." .. self.sectiontype
			local origin, name = next(self.map:formvaluetable(crval))

			if self.anonymous then
				if name then
					created = self:create(nil, origin)
				end
			else
				if name then
					local section_name = arg[1] .. "_" .. name
					-- Ignore if it already exists
					if self:cfgvalue(section_name) then
						m.message = "err: This interface is already used"
							.. " in the policy"
						name = nil
					end

					name = self:checkscope(name)

					if not name then
						self.err_invalid = true
					end

					if name and #name > 0 then

						created = self:create(name, origin) and name
						if not created then
							self.invalid_cts = true
						end
					end
				end
			end

			if created then
				AbstractSection.parse_optionals(self, created)
			end
		end
		if self.sortable then
			local stval = RESORT_PREFIX .. self.config .. "." .. self.sectiontype
			local order = self.map:formvalue(stval)
			if order and #order > 0 then
				local sid
				local num = 0
				local sections = {}
				for sid in util.imatch(order) do
					self.map.uci:reorder(self.config, sid, num)
					num = num + 1
					if deleted_section_id ~= sid then
						table.insert(sections, sid)
					end
				end
				if num > 0 then
					self.changed = true
					self.map.uci:set(self.config, arg[1], "use_member", sections)
				end
			end
		end

		if created or self.changed then
			self:push_events()
		end
	end

	function mwan_policy.create(self, interface)
		local section = string.format("%s_%s", arg[1], interface)
		local use_members = self.map.uci:get_list(self.config, arg[1], "use_member")

		for key, value in pairs(use_members) do
			local member_iface = self.map.uci:get(self.config,
				value, "interface") or nil
			if member_iface == interface then
				m.message = "err: This interface is already used in the policy"
				return false
			end
		end

		local created = TypedSection.create(self, section)

		if created then
			self.map.uci:set(self.config, section, "interface", interface)
			self.map.uci:set(self.config, section, "metric", "1")
			-- It's important that this is queried again
			local sections = self.map.uci:get_list(self.config, arg[1], "use_member")
			table.insert(sections, section)
			self.map:set(arg[1], "use_member", sections)
			m.uci:save("load_balancing")
			return true
		end
	end

	function mwan_policy.remove(self, section)
		self.map.proceed = true
		TypedSection.remove(self, section)
		local sections = self.map.uci:get_list(self.config, arg[1], "use_member")
		local use_member_id = nil
		for n, i in ipairs(sections) do
			if sections[n] == section then
				use_member_id = n
			end
		end
		table.remove(sections, use_member_id)
		self.map:set(arg[1], "use_member", sections)
		m.uci:save("load_balancing")
	end

	--Return all matching UCI sections for this TypedSection
	function mwan_policy.cfgsections(self)
		local sections = self.map.uci:get_list(self.config, arg[1], "use_member")

		return sections
	end

interface = mwan_policy:option(DummyValue, "interface", translate("Interface"))

	function interface.cfgvalue(self, section)
		return get_conn_name(self, section) or "-"
	end

weight = mwan_policy:option(Value, "weight", translate("Ratio"))
	weight.default = "1"
	weight.datatype = "range(1,10)"

return m
