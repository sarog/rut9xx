local ds = require "luci.dispatcher"

m = Map("output_control", translate("Periodic Output Control"),translate(""))

s = m:section(TypedSection, "rule", translate("Control Rules"))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.extedit   = ds.build_url("admin/services/input-output/output/periodic/%s")
s.novaluetext = translate("There are no output rules created yet")

function s.parse(self, ...)
	TypedSection.parse(self, ...)
	if created then
		self.map:set(created, "action", "on")
		self.map:set(created, "mode", "fixed")
		self.map:set(created, "enabled", "0")
		m.uci:save("sms_utils")
		luci.http.redirect(ds.build_url("admin/services/input-output/output/periodic", created))
	end
end

src = s:option(DummyValue, "action", translate("Action"), translate("Specifies what action will happen"))
	src.rawhtml = true
	src.width   = "12%"

	function src.cfgvalue(self, section)
		local value = m.uci:get(self.config, section, self.option)
		if value then
			return value:gsub("^%l", string.upper)
		else
			return "-"
		end
	end

src = s:option(DummyValue, "gpio", translate("Output"), translate("Specifies for which output type rule will be applied"))
	src.rawhtml = true
	src.width   = "12%"

	function src.cfgvalue(self, section)
		local value = m.uci:get(self.config, section, self.option)
		if value then
			if value == "DOUT1" then value = "Digital OC"
			elseif value == "DOUT2" then value = "Digital relay"
			elseif value == "DOUT3" then value = "Digital 4PIN"
			end
			return value
		else
			return "-"
		end
	end

src = s:option(DummyValue, "mode", translate("Mode"), translate("Repetition mode. It can be fixed (happens at specified time) or interval (happens constantly after specified time from each other)"))
	src.rawhtml = true
	src.width   = "12%"

	function src.cfgvalue(self, section)
		local value = m.uci:get(self.config, section, self.option)
		if value then
			return value:gsub("^%l", string.upper)
		else
			return "-"
		end
	end
src = s:option(DummyValue, "interval_time", translate("Interval (minutes)"), translate("Interval to repeat action in minutes"))
src.rawhtml = true
src.width   = "12%"

function src.cfgvalue(self, section)
	local value = m.uci:get(self.config, section, self.option)
	if value then
		return value:gsub("^%l", string.upper)
	else
		return "-"
	end
end

src = s:option(DummyValue, "fixed_hour", translate("Hour"), translate("Hour to execute action"))
	src.rawhtml = true
	src.width   = "12%"
function src.cfgvalue(self, section)
	
	
	local value = m.uci:get(self.config, section, self.option)
	if value then
		return value:gsub("^%l", string.upper)
	else
		return "-"
	end
end


src = s:option(DummyValue, "fixed_minute", translate("Minute"), translate("Minute to execute action"))
src.rawhtml = true
src.width   = "12%"

function src.cfgvalue(self, section)
	local value = m.uci:get(self.config, section, self.option)
	if value then
		return value:gsub("^%l", string.upper)
	else
		return "-"
	end
end
	
src = s:option(DummyValue, "timeout_time", translate("Action timeout (sec)"), translate("Specifies after how much time this action should end"))
	src.rawhtml = true
	src.width   = "12%"

	function src.cfgvalue(self, section)
		local value = m.uci:get(self.config, section, self.option) or nil
		local enabled = m.uci:get(self.config, section, "timeout") or "0"
		if value and enabled == "1" then
			return value:gsub("^%l", string.upper)
		elseif value then
			return "Not enabled"
		else
			return "-"
		end
	end

src = s:option(DummyValue, "day", translate("Days"), translate("Specifies in which days action should happen"))
	src.rawhtml = true
	src.width   = "30%"

	function src.cfgvalue(self, section)
		local table = m.uci:get(self.config, section, self.option)
		local value
		if table then
			for key, val in pairs(table) do
				if not value then
					value = (val:gsub("^%l", string.upper))
				else
					value = value .. ", " .. (val:gsub("^%l", string.upper))
				end
			end
		else
			value = "-"
		end
			return value
		
	end

o = s:option(Flag, "enabled", translate("Enable"), translate("Enable output control rule"))


return m 
