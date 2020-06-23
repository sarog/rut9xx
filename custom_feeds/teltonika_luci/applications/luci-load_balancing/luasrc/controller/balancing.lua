module("luci.controller.balancing", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/load_balancing") then
		return
	end

	entry({"admin", "network", "balancing"}, alias("admin", "network", "balancing", "configuration"),
		_("Load Balancing"), 600)
	entry({"admin", "network", "balancing", "configuration"}, cbi("load_balancing/load_balancing_interface"))
	entry({"admin", "network", "balancing",  "policy"}, cbi("load_balancing/load_balancing_policyconfig")).leaf = true
	entry({"admin", "network", "balancing",  "rule"}, cbi("load_balancing/load_balancing_ruleconfig")).leaf = true
	entry({"admin", "network", "balancing",  "status"}, call("load_balancing_status"))

end

function load_balancing_status()
	local uci = require "luci.model.uci".cursor()
	local names = {
		{ifname="3g-ppp", genName="Mobile", type="3G"},
		{ifname="eth2", genName="Mobile", type="3G"},
		{ifname="usb0", genName="WiMAX", type="WiMAX"},
		{ifname="eth1", genName="Wired", type="vlan"},
		{ifname="wlan0", genName="WiFi", type="wifi"},
		{ifname="none", genName="Mobile bridged", type="3G"},
		{ifname="wwan0", genName="Mobile", type="3G"},
		{ifname="wm0", genName="WiMAX", type="WiMAX"},
		{ifname="wwan-usb0", genName="USB Modem", type="USB"},
	}
	local interface, status, usage, name, ifname
	--FIXME: global?
	output = assert (io.popen("/usr/sbin/load_balancing interfaces"))
	interfaces = {}
	policies = {}

	for line in output:lines() do
		interface = line:match("wa%w+")
		if interface then
			ifname = uci:get("network", interface, "ifname")
			for n, i in ipairs(names) do
				if i.ifname == ifname then
					name = i.genName
				end
			end
			status = line:match("%w+$")
			if status then
				table.insert(interfaces, { interface = interface, status = status, name = name})
			end
		end
	end
	output:close()

	output = assert (io.popen("/usr/sbin/load_balancing policies"))

	for line in output:lines() do
		interface = line:match("wa%w+")
		if interface then
			usage = line:match("%d+%%")
			if status then
				table.insert(policies, { interface = interface, usage = usage})
			end
		end
	end

	output:close()



	rv = {
		interfaces = interfaces,
		policies = policies
	}
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end

