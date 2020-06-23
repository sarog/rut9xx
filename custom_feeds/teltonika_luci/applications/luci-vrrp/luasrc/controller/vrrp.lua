--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: openvpn.lua 7362 2011-08-12 13:16:27Z jow $
]]--

module("luci.controller.vrrp", package.seeall)
local uci = require "luci.model.uci".cursor()
local fs = require "nixio.fs"

function index()
	local vrrp
	vrrp = entry( {"admin", "services", "vrrp"}, arcombine(cbi("vrrp_add"), cbi("vrrp")), _("VRRP"), 1)
	vrrp.leaf=true

	entry({"admin", "services", "vrrp_status"}, call("vrrp_state"), nil)
end

function vrrp_state()
	local found = false
	local rv = { }

	uci:foreach("vrrpd", "vrrpd", function (instance)
		rv[instance[".name"]] = {}
		if instance.enabled == "1" then
			local instance_info = fs.readfile("/tmp/vrrpd_"..instance[".name"].."_log")
			if instance_info and type(instance_info) ~= "table" then
				local instance_splitted = luci.util.split(instance_info, "\n")
				found = true

				if instance_splitted[1] == "Master" then
					local bus = require "ubus"
					local ubus = bus.connect()
					local addrs = ubus:call("network.interface.%s" % instance.interface,
						"status", { })

					if addrs then
						if addrs["ipv4-address"] and #addrs["ipv4-address"] > 0 and addrs["ipv4-address"][1].address ~= "" then
							rv[instance[".name"]] = { state=instance_splitted[1], master_ip=addrs["ipv4-address"] and #addrs["ipv4-address"] > 0 and addrs["ipv4-address"][1].address }
						else
							rv[instance[".name"]] = { state=instance_splitted[1], master_ip="IP address not set for interface" }
						end
					else
						rv[instance[".name"]] = { state=instance_splitted[1], master_ip="IP address not set for interface" }
					end
				else
					rv[instance[".name"]] = { state=instance_splitted[1], master_ip=instance_splitted[2] }
				end
			end
		end
	end)

	if found then
		luci.http.prepare_content("application/json")
		luci.http.write_json(rv)
		return
	end

	luci.http.status(404, "No configurations are available")
end
