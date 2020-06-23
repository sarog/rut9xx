--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008-2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: system.lua 8122 2011-12-20 17:35:50Z jow $
]]--

module("luci.controller.admin.licenses", package.seeall)

function index()
	    entry({"admin", "system", "licenses"}, alias("admin", "system", "licenses","general_info"), _("Licenses"), 7)
        entry({"admin", "system", "licenses","general_info"}, template("admin_licenses/general_info"),
                _("General Info"), 1).leaf = true
        entry({"admin", "system", "licenses","gplv2"}, template("admin_licenses/GPLv2"), _("GPLv2"), 2).leaf = true
        entry({"admin", "system", "licenses","gplv3"}, template("admin_licenses/GPLv3"), _("GPLv3"), 3).leaf = true
        entry({"admin", "system", "licenses","lgplv2"}, template("admin_licenses/LGPLv2-1"),
                _("LGPLv2.1"), 4).leaf = true
        entry({"admin", "system", "licenses","mit"}, template("admin_licenses/MIT"), _("MIT"), 5).leaf = true
        entry({"admin", "system", "licenses","bsd-4"}, template("admin_licenses/BSD"), _("BSD-4-Clause"), 6).leaf = true
        entry({"admin", "system", "licenses","bsd_like"}, template("admin_licenses/BSD_like"), _("BSD"), 7).leaf = true
        entry({"admin", "system", "licenses","isc"}, template("admin_licenses/ISC"), _("ISC"), 8).leaf = true
        entry({"admin", "system", "licenses","aslv2"}, template("admin_licenses/ASLv2"), _("ASLv2"), 9).leaf = true
        entry({"admin", "system", "licenses","openssl"}, template("admin_licenses/OpenSSL"),
                _("OpenSSL"), 10).leaf = true
        entry({"admin", "system", "licenses","zlib"}, template("admin_licenses/ZLIB"), _("ZLIB"), 11).leaf = true
end
