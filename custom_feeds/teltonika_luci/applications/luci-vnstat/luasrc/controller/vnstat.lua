module("luci.controller.vnstat", package.seeall)

function index()
	entry({"admin", "status", "vnstat"}, alias("admin", "status", "vnstat", "graphs"), _(translate("VnStat Traffic Monitor")), 90).i18n = "vnstat"
	entry({"admin", "status", "vnstat", "graphs"}, template("vnstat"), _(translate("Graphs")), 1)
	entry({"admin", "status", "vnstat", "config"}, cbi("vnstat"), _(translate("Configuration")), 2)

	entry({"mini", "network", "vnstat"}, alias("mini", "network", "vnstat", "graphs"), _(translate("VnStat Traffic Monitor")"VnStat Traffic Monitor"), 90).i18n = "vnstat"
	entry({"mini", "network", "vnstat", "graphs"}, template("vnstat"), _(translate("Graphs")), 1)
	entry({"mini", "network", "vnstat", "config"}, cbi("vnstat"), _(translate("Configuration")), 2)
end
