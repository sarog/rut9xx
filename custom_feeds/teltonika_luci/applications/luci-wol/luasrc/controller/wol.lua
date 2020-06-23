module("luci.controller.wol", package.seeall)

function index()
	entry({"admin", "services", "wol"}, cbi("wol"), _(translate("Wake on LAN")), 90).i18n = "wol"
end
