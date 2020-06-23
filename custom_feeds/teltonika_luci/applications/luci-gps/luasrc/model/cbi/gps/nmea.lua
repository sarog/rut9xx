function validate_external_flash(value)
    if value and not value:match("^\/mnt\/") then
        return nil, "Location must be prefixed with \"/mnt/\" to avoid " ..
                    "wear out of device flash"
    else
        return value
    end
end

map = Map("gps")
-------------------------------------------------------------------------------
--                         Nmea forwarding section
-------------------------------------------------------------------------------
forwarding_sec = map:section(NamedSection, "nmea_forwarding", nil, translate("NMEA forwarding"))

enabled = forwarding_sec:option(Flag, "enabled",
                                translate("Enabled"),
                                translate("Enabled NMEA sentence forwarding to a remote server"))
enabled.rmempty = false

hostname = forwarding_sec:option(Value, "hostname",
                                translate("Hostname"),
                                translate("Hostname of the server for NMEA sentence forwarding"))
hostname.rmempty = false
hostname.datatype = "or(hostname, ipaddr)"

port = forwarding_sec:option(Value, "port",
                             translate("Port"),
                             translate("Port of the server for NMEA sentence forwarding"))
port.rmempty = false
port.datatype = "port"

proto = forwarding_sec:option(ListValue, "proto",
                              translate("Protocol"),
                              translate("Porotocol to be used for NMEA sentence forwarding"))
proto.rmempty = false
proto:value("tcp", "TCP")
proto:value("udp", "UDP")

connection_contain = forwarding_sec:option(Flag, "con_contain",
                                          translate("Contain Connection"),
                                          translate("Contain active session with a remote server"))

connection_contain.rmempty = false

-------------------------------------------------------------------------------
--                     Nmea forwarding cache section
-------------------------------------------------------------------------------
cache_sec = map:section(NamedSection, "nmea_forwarding_cache", nil, translate("NMEA forwarding cache"))
type = cache_sec:option(ListValue, "type",
                        translate("Type"),
                        translate("Cache type"))
type.rmempty = false
type:value("ram")
type:value("flash")

sentences_max = cache_sec:option(Value, "sentences_max",
                                 translate("Maximum sentences"),
                                 translate("Maximum amount of sentences in the cache"))
sentences_max.rmempty = false
sentences_max.datatype = "uinteger"

cache_location = cache_sec:option(Value, "location",
                                  translate("File"),
                                  translate("File to be used for NMEA sentence caching"))
cache_location.rmempty = false
cache_location:depends({type = "flash"})
function cache_location.validate(self, value, section) return validate_external_flash(value) end

-------------------------------------------------------------------------------
--                       Nmea collecting section
-------------------------------------------------------------------------------

collecting_sec = map:section(NamedSection, "nmea_collecting", nil, translate("NMEA collecting"))
enabled = collecting_sec:option(Flag, "enabled",
                                translate("Enabled"),
                                translate("Enable NMEA sentence collecting to a file"))
enabled.rmempty = false

collecting_location = collecting_sec:option(Value, "location",
                                            translate("Location"),
                                            translate("File to be used for NMEA sentence collecting"))
collecting_location:depends({enabled = "1"})
collecting_location.datatype = "string_not_empty"
function collecting_location.validate(self, value, section) return validate_external_flash(value) end

-------------------------------------------------------------------------------
--                      Nmea sentence settings section
-------------------------------------------------------------------------------

sentence_sec = map:section(TypedSection, "nmea_rule", translate("NMEA sentence settings"))
sentence_sec.template = "cbi/tblsection"
sentence_sec.addremove = false

forwarding_enabled = sentence_sec:option(Flag, "forwarding_enabled",
                                         translate("Forwarding enabled"),
                                         translate("Enable forwarding for a sentence"))
forwarding_enabled.rmempty = false

forwarding_interval = sentence_sec:option(Value, "forwarding_interval",
                                          translate("Forwarding interval"),
                                          translate("Set interval of seconds for sentence forwarding"))
forwarding_interval.rmempty = false
forwarding_interval.datatype = "uinteger"

collecting_enabled = sentence_sec:option(Flag, "collecting_enabled",
                                         translate("Collecting enabled"),
                                         translate("Enable collecting for a sentence"))
collecting_enabled.rmempty = false

collecting_interval = sentence_sec:option(Value, "collecting_interval",
                                          translate("Collecting interval"),
                                          translate("Set interval of seconds for sentence collecting"))
collecting_interval.rmempty = false
collecting_interval.datatype = "uinteger"

return map
