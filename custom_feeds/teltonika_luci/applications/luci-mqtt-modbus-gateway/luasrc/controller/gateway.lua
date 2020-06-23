module("luci.controller.gateway", package.seeall)

function index()
	entry({"admin", "services", "modbus", "gateway"}, cbi("gateway"), _("MQTT gateway"), 40).leaf = true
end
