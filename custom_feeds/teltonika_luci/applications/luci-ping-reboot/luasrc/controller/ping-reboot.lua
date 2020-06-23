module("luci.controller.ping-reboot", package.seeall)

function index()
    entry({"admin", "services", "auto-reboot"}, alias("admin", "services", "auto-reboot", "ping-reboot"), _("Auto Reboot"), 95)
    entry({"admin", "services", "auto-reboot", "ping-reboot"}, arcombine(cbi("auto-reboot/ping-reboot-owerview"), cbi("auto-reboot/ping-reboot")), _("Ping Reboot"), 1).leaf=true
    entry({"admin", "services", "auto-reboot", "periodic-reboot"}, cbi("auto-reboot/periodic-reboot"), _("Periodic Reboot"), 2).leaf=true
    entry({"admin", "services", "auto-reboot", "wget-reboot"}, cbi("auto-reboot/wget-reboot"), _("Wget Reboot"), 3).leaf=true
end