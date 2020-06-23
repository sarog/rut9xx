
module("luci.controller.input-output", package.seeall)

function index()
	local sys = require "luci.sys"
	local utl = require "luci.util"
	local in_out = utl.trim(luci.sys.exec("uci get hwinfo.hwinfo.in_out"))
	local is_fl = utl.trim(luci.sys.exec("uci get hwinfo.hwinfo.4pin_io"))
	if in_out == "1" or is_fl == "1" then
		entry({"admin", "services", "input-output"}, alias("admin", "services", "input-output", "status"), _("Input/Output"), 100).i18n = "input-output"
		entry({"admin", "services", "input-output", "status"}, template("input-output/status"), _("Status"), 10).leaf = true
		entry({"admin", "services", "input-output", "inputs"}, arcombine(cbi("input-output/inputs"), cbi("input-output/input-details")), _("Input"), 20).leaf = true
		entry({"admin", "services", "input-output", "output"}, alias("admin", "services", "input-output", "output", "output_configuration"), _("Output"), 30)
		entry({"admin", "services", "input-output", "output", "output_configuration"}, cbi("input-output/output_configuration"), _("Output Configuration"), 1).leaf = true
		entry({"admin", "services", "input-output", "output", "on_off"}, template("input-output/output"), _("ON/OFF"), 2).leaf = true
		entry({"admin", "services", "input-output", "output", "post_get"}, cbi("input-output/post_get"), _("Post/Get Configuration"), 3).leaf = true
		entry({"admin", "services", "input-output", "output", "periodic"}, arcombine(cbi("input-output/periodic_control"), cbi("input-output/periodic_control_details")), _("Periodic Control"), 4).leaf = true
		entry({"admin", "services", "input-output", "output", "scheduler"}, cbi("input-output/output_scheduler"), _("Scheduler"), 5).leaf = true
		entry({"admin", "services", "input-output", "customfields"}, cbi("input-output/customfields")).leaf = true
		if is_fl ~= "1" then
			entry({"admin", "services", "input-output", "analogfield"}, template("input-output/analogfield")).leaf = true
		end

	end
end
