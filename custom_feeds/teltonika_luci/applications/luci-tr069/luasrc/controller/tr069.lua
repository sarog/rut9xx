module("luci.controller.tr069", package.seeall)

function index()
    entry({"admin", "services", "tr069"}, cbi("tr069/tr069"), _("TR-069"), 9).leaf=true
end
