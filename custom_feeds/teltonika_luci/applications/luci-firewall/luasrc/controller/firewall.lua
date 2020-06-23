module("luci.controller.firewall", package.seeall)

function index()
	entry({"admin", "network", "firewall"},
		alias("admin", "network", "firewall", "zones"),
		_("Firewall"), 50).i18n = "firewall"

	entry({"admin", "network", "firewall", "zones"},
		arcombine(cbi("firewall/zones"), cbi("firewall/zone-details")),
		_("General Settings"), 10).leaf = true

	entry({"admin", "network", "firewall", "forwards"},
		arcombine(cbi("firewall/forwards"), cbi("firewall/forward-details")),
		_("Port Forwarding"), 20).leaf = true

	entry({"admin", "network", "firewall", "rules"},
		arcombine(cbi("firewall/rules"), cbi("firewall/rule-details")),
		_("Traffic Rules"), 30).leaf = true

	entry({"admin", "network", "firewall", "custom"},
		cbi("firewall/custom"),
		_("Custom Rules"), 40).leaf = true

	entry({"admin", "network", "firewall", "ddos"},
		cbi("firewall/ddos_prevention"),
		_("DDOS Prevention"), 50).leaf = true

	entry({"admin", "network", "firewall", "pscan"},
		cbi("firewall/pscan_prevention"),
		_("Port Scan Prevention"), 60).leaf = true

	entry({"admin", "network", "firewall", "helpers"},
		cbi("firewall/nat_helpers"),
		_("Helpers"), 70).leaf = true

end
