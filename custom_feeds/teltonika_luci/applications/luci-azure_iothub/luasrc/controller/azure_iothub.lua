module("luci.controller.azure_iothub", package.seeall)

function index()

	entry({"admin", "services", "iot", "azure_iothub"},
		cbi("azure_iothub"), _("Azure IoThub"), 1).leaf = true

	entry({"admin", "services", "iot"},
		alias("admin", "services", "iot", "azure_iothub"), _("IoT Platforms"), 50)
end