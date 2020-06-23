--[[
LuCI - Lua Configuration Interface

Copyright 2009 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: ff_p2pblock.lua 7362 2011-08-12 13:16:27Z jow $
]]--

module("luci.controller.ff_p2pblock", package.seeall)

function index()
	entry({"admin", "network", "firewall", "p2pblock"}, cbi("luci_fw/p2pblock"),
		_(translate("P2P-Block")), 40).i18n = "p2pblock"
end
