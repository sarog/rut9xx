
module("luci.controller.gps", package.seeall)

local uci = require("luci.model.uci").cursor()

function index()
	local uci = require("luci.model.uci").cursor()
	local gps = uci:get("hwinfo", "hwinfo", "gps")

	if gps == "1" then
		entry({"admin", "services", "gps"}, call("go_to"), _("GPS"), 86)
		entry({"admin", "services", "gps", "map"}, template("gps/map"), _("Map"), 1)
		entry({"admin", "services", "gps", "general"}, cbi("gps/general"), _("General"), 2).leaf = true
		entry({"admin", "services", "gps", "nmea"}, cbi("gps/nmea"), _("NMEA"), 3).leaf = true
		entry({"admin", "services", "gps", "https"}, cbi("gps/https"), _("HTTPS"), 4).leaf = true
		entry({"admin", "services", "gps", "avl"}, arcombine(cbi("gps/avl"), cbi("gps/avl_details")),
			_("AVL"), 5).leaf = true
		entry({"admin", "services", "gps", "input"}, arcombine(cbi("gps/gps_input"), cbi("gps/gps_input-details")),
			_("AVL I/O"), 6).leaf = true
		entry({"admin", "services", "gps", "geofencing"},
			arcombine(cbi("gps/gps_geofencing"), cbi("gps/gps_geofencing-details")),
			_("GPS Geofencing"), 7).leaf = true
		entry({"admin", "services", "gps", "geofencing_delete"}, call("geofencing_delete"), nil).leaf = true
		entry({"admin", "services", "gps", "get_cord"}, call("get_cord"), nil)
		entry({"admin", "services", "gps", "get_geofence"}, call("get_geofence"), nil)
	end
end

function geofencing_delete()
        local path  = luci.dispatcher.context.requestpath
        local geofencing = path[#path]
        if geofencing then
                uci:delete("gps", geofencing)
                luci.http.redirect(luci.dispatcher.build_url("admin/services/gps/geofencing"))
                uci:commit("gps")
                luci.sys.call("/sbin/luci-reload & >/dev/null 2>/dev/null")
                return
        end
end

function go_to()
	local enabled = uci:get("gps", "gpsd", "enabled") or "0"

	if enabled == "1" then
		luci.http.redirect(luci.dispatcher.build_url("admin", "services", "gps", "map"))
	else
		luci.http.redirect(luci.dispatcher.build_url("admin", "services", "gps", "general"))
	end
end

function get_cord()
	local iterator = 1
	local array_geo = {}
	local res = luci.sys.exec("gpsctl -seix"):split("\n");

	uci:foreach("gps", "geofencing", function(section)
	     array_geo[iterator] = section
	     iterator = iterator + 1
        end)

	local table = {
		fix_status = res[1],
		fix_time =  res[2],
		latitude = res[3],
		longitude = res[4],
		geofence = array_geo
	}

	luci.http.prepare_content("application/json")
	luci.http.write_json(table)

	return
end

function get_geofence()
        local array_geo;
        local res = luci.sys.exec("gpsctl -seix"):split("\n");
        local section_name = luci.http.formvalue("section")

        uci:foreach("gps", "geofencing", function(s)
                if s[".name"] == section_name then
                        array_geo = s
                end
        end)

        local table = {
                fix_status = res[1],
                fix_time =  res[2],
                latitude = res[3],
                longitude = res[4],
                geofence = array_geo
        }

        luci.http.prepare_content("application/json")
        luci.http.write_json(table)

        return
end
