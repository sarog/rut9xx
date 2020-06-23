--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: pavyzdys.lua 6984 2017-08-22 15:14:42Z soma $
]]--
local l=require"luci.sys"
map = Map("zerotier", translate("ZeroTier-One"))
map.template = "zeromap"

x = uci.cursor()

--Sekcijos gali buti dvieju tipu "NamedSection" ir "TypedSection"
--"NamedSection" yra naudojama sekcija su unikaliu pavadinimu
s = map:section(NamedSection, "zerotier", "ZeroTier", translate("ZeroTier"))
s.addremove = false

flag = s:option(Flag, "enabled", translate("Enabled"), translate("Enabled"))
flag.default= "1"

secret = s:option(DummyValue, "address", translate("Address"), translate("Randomly generated unique address"))
secret.default = "Will be generated after successful connection"

network = s:option(DynamicList, "join", translate("Networks"), translate("Zerotier networks to join"))
network.rmempty = false

function network.validate(self, value, section)
	for q,i in ipairs(value) do
		for w,y in ipairs(value) do
			if i == y and q ~= w then
				return nil, 'two or more network values are the same'
			end
		end
	end
	return value
end

function map.on_after_commit()
	local selected_network = x:get("zerotier","zerotier","selectedNetwork")
	local join = x:get("zerotier","zerotier","join")
	local selected
	if selected_network then
		if join then
			for _,i in ipairs(x:get("zerotier","zerotier","join")) do
				if selected_netork == i then
					selected = 1
				end
			end
		end
		if selected == 1 or not join then
			x:delete("zerotier", "zerotier", "selectedNetwork")
			x:delete("zerotier", "zerotier", "vpnenabled")
			x:commit("zerotier")
		end
	end
end 

return map
