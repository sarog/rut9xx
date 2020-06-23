
module("luci.controller.multiwan", package.seeall)

function index()
	local fs = luci.fs or nixio.fs

	if not fs.access("/etc/config/multiwan") then
		return
	end
	
	local uci = require("luci.model.uci").cursor()

	local module = uci:get("system", "module", "type")
	if module and module == "none" or not module then
		if uci:get("multiwan", "config", "enabled") == "0" then
			module = false
		end
	end

	entry({"admin", "network", "multiwan"}, cbi("multiwan/multiwan"), nil)
	entry({"admin", "network", "multiwan", "status"}, call("multiwan_status"))
end

function multiwan_status()
	local nfs = require "nixio.fs"
	local cachefile = "/tmp/.mwan/cache"
	local rv = {	}

	cachefile = nfs.readfile(cachefile)
	if cachefile then
		local ntm = require "luci.model.network".init()
		_, _, wan_if_map = string.find(cachefile, "wan_if_map=\"([^\"]*)\"")
		_, _, wan_fail_map = string.find(cachefile, "wan_fail_map=\"([^\"]*)\"")
		_, _, wan_recovery_map = string.find(cachefile, "wan_recovery_map=\"([^\"]*)\"")
		
		rv.wans = { }
		wansid = {}

		for wanname, wanifname in string.gfind(wan_if_map, "([^%[]+)%[([^%]]+)%]") do
			local wanlink = ntm:get_interface(wanifname)
			      wanlink = wanlink and wanlink:get_network()
			      wanlink = wanlink and wanlink:adminlink() or "#"
			wansid[wanname] = #rv.wans + 1
			rv.wans[wansid[wanname]] = { name = wanname, link = wanlink, ifname = wanifname, status = "ok", count = 0 }
		end

		for wanname, failcount in string.gfind(wan_fail_map, "([^%[]+)%[([^%]]+)%]") do
			if failcount == "x" then
				rv.wans[wansid[wanname]].status = "ko"
			else
				rv.wans[wansid[wanname]].status = "failing"
				rv.wans[wansid[wanname]].count = failcount
			end
		end

		for wanname, recoverycount in string.gfind(wan_recovery_map, "([^%[]+)%[([^%]]+)%]") do
			rv.wans[wansid[wanname]].status = "recovering"
			rv.wans[wansid[wanname]].count = recoverycount
		end
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end
