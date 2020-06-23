--[[
Copyright (C) 2014 - Eloi Carbó Solé (GSoC2014)
BGP/Bird integration with OpenWRT and QMP

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
]]--

require("luci.sys")
local http = require "luci.http"
local uci = require "luci.model.uci"
local uciout = uci.cursor()

m=Map("bird4", translate("Dynamic Routes"), "")

-- Named section: "bird"
--========== turi sios reiksmes likti confige
s_bird_uci = m:section(NamedSection, "bird", "bird", translate("General Settings"), "")
s_bird_uci.addremove = False

enable = s_bird_uci:option(Flag, "enabled", translate("Enable"), translate("If this option is true, the protocol enabled"))
enable.default=0
enable.rmempry = false

function enable.write(self, section, value)
  self.map:set(section, self.option, value)
  m.uci:set("firewall", "A_OSPFIGP", "enabled", value)
  m.uci:save("firewall")
  m.uci:commit("firewall")
end

--
-- uuc = s_bird_uci:option(Flag, "use_UCI_config", translate("Use UCI configuration"), translate("Use UCI configuration instead of the /etc/bird4.conf file"))
--
-- ucf = s_bird_uci:option(Value, "UCI_config_File", translate("UCI File"), translate("Specify the file to place the UCI-translated configuration"))
-- ucf.default = "/tmp/bird4.conf"
--==========

-- -- Named Section: "table"
--
-- s_bird_table = m:section(TypedSection, "table", translate("Tables configuration"), translate("Configuration of the tables used in the protocols"))
-- s_bird_table.addremove = true
-- s_bird_table.anonymous = true
--
-- name = s_bird_table:option(Value, "name", translate("Table name"), translate("Descriptor ID of the table"))

-- Named section: "global"

s_bird_global = m:section(NamedSection, "global", "global", translate(""), translate(""))
s_bird_global.addremove = False

id = s_bird_global:option(Value, "router_id", translate("Router ID"), translate("Identification number of the router. By default, is the router's IP."))

-- lf = s_bird_global:option(Value, "log_file", translate("Log File"), translate("File used to store log related data."))
--
-- l = s_bird_global:option(MultiValue, "log", translate("Log"), translate("Set which elements do you want to log."))
-- l:value("all", translate("All"))
-- l:value("error",translate("Error"))
-- l:value("debug",translate("Debug"))
--
-- d = s_bird_global:option(MultiValue, "debug", translate("Debug"), translate("Set which elements do you want to debug."))
-- d:value("all", translate("All"))
-- d:value("states",translate("States"))
-- d:value("routes",translate("Routes"))
-- d:value("filters",translate("Filters"))
-- d:value("interfaces",translate("Interfaces"))
-- d:value("events",translate("Events"))
-- d:value("packets",translate("Packets"))

function m.on_commit(self,map)
        luci.sys.call('/etc/init.d/bird4 stop; /etc/init.d/bird4 start')
end

return m
