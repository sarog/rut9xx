
module("luci.controller.dmvpn", package.seeall)

function index()
	entry( {"admin", "services", "vpn", "dmvpn"}, arcombine(cbi("dmvpn_add"), cbi("dmvpn_edit")),
		_("DMVPN"), 8).leaf=true
	entry({"admin", "services", "vpn", "dmvpn_delete"}, call("dmvpn_delete"), nil).leaf = true
end


function dmvpn_delete()
	local uci = require "luci.model.uci".cursor()
	local path  = luci.dispatcher.context.requestpath
	local dmvpn = path[#path]

	if dmvpn then
		local config_mode = uci:get("dmvpn", dmvpn, "config_mode")

		uci:delete("strongswan", dmvpn.."_dmvpn")
		uci:delete("network", dmvpn)
		uci:delete("network", dmvpn.."_static")
		uci:delete("quagga", dmvpn.."_dmvpn")

		if config_mode == "spoke" then
			uci:delete("network", dmvpn.."_dmvpn_route")
		end

		uci:delete("dmvpn", dmvpn)
		uci:commit("strongswan")
		uci:commit("network")
		uci:commit("quagga")
		uci:commit("dmvpn")

		luci.http.redirect(luci.dispatcher.build_url("admin/services/vpn/dmvpn"))
		luci.sys.call("/sbin/luci-reload & >/dev/null 2>/dev/null")

		return
	end
end
