local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()
local util = require ("luci.util")

local m = Map("stunnel", translate("Stunnel"))

local globals = m:section( TypedSection, "globals", translate("Stunnel Globals"), translate("") )
globals.addremove = false

o = globals:option( Flag, "enabled", translate("Enabled"), translate("Enable service"))

o = globals:option( Value, "debug", translate("Debug Level"), translate("Level of output to log"))

o = globals:option( Flag, "use_alt", translate("Use alternative config"), translate("Enable alternative configuration option (Config upload).<br> * Be aware that when using alternative configuration, all configurations in \"Stunnel Configuration\" section will be skipped"))

alt_config = globals:option( FileUpload, "alt_config_file", translate("Upload alternative config"), translate("ISAKMP (Internet Security Association and Key Management Protocol) phase 1 exchange mode"))
alt_config:depends("use_alt", "1")
alt_config.size = "51200"
alt_config.sizetext = translate("Selected file is too large. Maximum allowed size is 50 KiB")
alt_config.sizetextempty = translate("Selected file is empty")

local services = m:section( TypedSection, "service", translate("Stunnel Configuration"), translate("") )
    services.addremove = true
    services.template = "stunnel/tblsection"
    services.novaluetext = translate("There are no STunnel configurations yet")
    services.extedit = luci.dispatcher.build_url("admin", "services", "vpn", "stunnel", "%s")
    services.defaults = {enabled = "0"}
    services.sectionhead = "Tunnel name"

    function services.create(self, section)
        local instance_count = 0
        m.uci:foreach(self.config, "service", function(sections)
            instance_count = instance_count + 1
        end)

        if instance_count >= 5 then
            m.message = "err: Can't create more instances. Only 5 STunnel instances are allowed"
            return
        elseif string.len(section) > 10 then
			m.message = "err: Name \'" .. section .. "\' is too long. Maximum 10 characters."
        else
            return TypedSection.create(self, section)
        end
    end


local status = services:option( Flag, "enabled", translate("Enabled"), translate("Make a stunnel active/inactive"))

ip_port = services:option( DummyValue, "accept_host", translate("Listening on"), translate("IP and port which server will be listening to"))

function ip_port.cfgvalue(self, section, value)
    local value = self.map:get(section, self.option)
    local port_value = self.map:get(section, "accept_port")

    if port_value and value then
        return value..":"..port_value
    else
        return "Not set"
    end
end

client = services:option( DummyValue, "client", translate("Operation mode"), translate("Stunnel operation mode. <br> * Server - Only listening on specified IP and Port. <br> * Client - Both listening and connecting to specified IPs"))

function client.cfgvalue(self, section, value)
    local value = self.map:get(section, self.option)

    if value and value == "1" then
        return "Client"
    elseif value and value == "0" then
        return "Server"
    else
        return "Not set"
    end
end

function m.on_before_save()
    local alt_file = m:get("globals", "alt_config_file")
    m:set("globals", "alt_config_file", "/etc/stunnel/stunnel.conf")
    if alt_file then
        m:set("globals", "alt_config_file", "/etc/stunnel/stunnel.conf")
        luci.sys.exec("cp -rf "..alt_file.." /etc/stunnel/stunnel.conf")
    else
        m:set("globals", "alt_config_file", "")
    end
end

function m.on_commit()
    luci.sys.exec("/etc/init.d/stunnel restart >/dev/null 2>&1 &")
end

return m
