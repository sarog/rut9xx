
module("luci.controller.profiles", package.seeall)

function index()
	entry( {"admin", "system", "profiles"}, alias("admin", "system", "profiles", "general"),
		_("Profiles"), 2)
	entry( {"admin", "system", "profiles", "general"}, cbi("profiles/general"), _("General"), 1)
	entry( {"admin", "system", "profiles", "scheduler"}, cbi("profiles/scheduler"), _("Scheduler"), 2)
    entry( {"admin", "system", "profiles", "apply"}, call("apply_profile")).leaf = true
	entry( {"admin", "system", "profiles", "update"}, call("update_profile"))
	entry( {"admin", "system", "profiles", "set"}, call("extract_profile"))
	entry( {"admin", "system", "profiles", "apply_uci"}, call("apply_profile_uci"))
end

function apply_profile()
	local uci = require "luci.model.uci".cursor()
	local sys = require "luci.sys"
	local utl = require "luci.util"
	local fs = require "nixio.fs"
    local _, port, protocol
    local lan_ip = uci:get("network", "lan", "ipaddr") or "192.168.1.1"
	local path  = luci.dispatcher.context.requestpath
	local profile = path[#path]

    if fs.access("/rom/etc/config/uhttpd") then
        local listen_http = utl.trim(sys.exec("uci -c /rom/etc/config/ get uhttpd.main.listen_http"))
        _, port = listen_http:match("^([^%[%]:]+):([^:]+)$")

        if not port then
            local listen_https = utl.trim(
                sys.exec("uci -c /rom/etc/config/ get uhttpd.main.listen_https"))
			_, port = listen_https:match("^([^%[%]:]+):([^:]+)$")
        end
	end

    luci.template.render("applying", {
    	title = luci.i18n.translate("Restarting..."),
   		msg  = luci.i18n.translate("The system is upgrading now."),
    	msg1  = luci.i18n.translate("<b>DO NOT POWER OFF THE DEVICE!</b>"),
    	msg2  = luci.i18n.translate("It might be necessary to change your computer\'s network"
			.. "settings to reach the device again, depending on your configuration."),
		protocol = (port and port == "443") and "https" or "http",
		ip = lan_ip,
		port = port or "80",
		profile = profile
    })
end

function extract_profile(name)
	local uci = require "luci.model.uci".cursor()
	local profile = name or luci.http.formvalue("profile")
	local src_lan = luci.http.formvalue("src_lan") or "1"
	local status = false
	local ip_addr

	if uci:get("profiles", profile) == "profile" then
		local tar_file = uci:get("profiles", profile, "archive")
		if nixio.fs.access("/etc/profiles/" .. tar_file) then
			local in_out = uci:get("hwinfo", "hwinfo", "in_out") or "0"
			local is_fl = uci:get("hwinfo", "hwinfo", "4pin_io") or "0"
			uci:set("profiles", "general", "profile", profile)
			uci:commit("profiles")
			luci.sys.exec("tar -C / -xzf /etc/profiles/%s" % tar_file)
			--Applying uci defaults
			luci.sys.exec("/usr/sbin/profile.sh -u")

			--DOUT1 ir DOUT2 nulinimas keiciant profili
			if in_out == "1" then
				os.execute("gpio.sh clear DOUT1; gpio.sh clear DOUT2")
			end
			if is_fl == "1" then
				os.execute("gpio.sh clear DOUT3")
			end
		
			if src_lan  == "1" then
				ip_addr = uci:get("network", "lan", "ipaddr") or "192.168.1.1"
			else
				ip_addr = luci.util.trim(
					luci.sys.exec(". /lib/teltonika-functions.sh; tlt_get_wan_ipaddr"))
			end

			status = true
		end
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json({ip_addr = ip_addr, status = status})
end

function update_profile()
	local profiles = require "luci.tools.profiles"
	local code, msg = profiles.update()

	luci.http.status(code, msg)
	if code ~= 200 then
		luci.http.prepare_content("text/plain")
		luci.http.write(msg)
	end
end

function apply_profile_uci()
	local uci = require "luci.model.uci".cursor()

    uci:apply()
end
