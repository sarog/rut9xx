module("luci.controller.telenor-mqtt", package.seeall)

function index()
	local fs = require "nixio.fs"
	
	entry({"admin", "services", "iot", "telenor-mqtt"},
		cbi("telenor-mqtt"), _("Telenor"), 5).leaf = true

  if not fs.access("/usr/lib/opkg/info/tlt_custom_pkg_azure_iothub.control") and 
  not fs.access("/usr/lib/opkg/info/tlt_custom_pkg_cmstreamapp.control") and 
  not fs.access("/usr/lib/opkg/info/tlt_custom_pkg_twstreamapp.control") and
  not fs.access("/usr/lib/opkg/info/tlt_custom_pkg_cotStreamApp.control") then
		entry({"admin", "services", "iot"},
			alias("admin", "services", "iot", "telenor-mqtt"), _("IoT Platforms"), 50)
	end
end