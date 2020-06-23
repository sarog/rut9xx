module("luci.controller.ntpc_service", package.seeall)
function index()
	local page

	page = entry({"admin", "services", "ntpc-service"}, cbi("ntpc/ntpc_service"), _("NTP"), 50)
	page.i18n = "ntpc"
	page.dependent = true
end
