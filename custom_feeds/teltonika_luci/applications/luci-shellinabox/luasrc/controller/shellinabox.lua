
module("luci.controller.shellinabox", package.seeall)

function index()
	entry({"admin", "services", "cli"}, template("shellinabox/cli"), _("CLI"), 90)
end

