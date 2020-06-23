local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()
local util = require ("luci.util")

local nhrp = Map("quagga", translate("NHRP"))

local nhrp_global_section = nhrp:section(NamedSection, "nhrp", "nhrp_general", translate("Globals"), translate(""))
    nhrp_global_section.addremove = false

local global_enabled = nhrp_global_section:option(Flag, "enabled", translate("Enable service"), translate("Enable service"))
local global_enabled = nhrp_global_section:option(Flag, "debug", translate("Enable debugging"), translate("Enable logging of NHRP"))

local nhrp_section = nhrp:section(TypedSection, "nhrp_instance", translate("Interfaces"), translate(""))
    nhrp_section.addremove = true
    nhrp_section.anonymous = false
    nhrp_section.template = "cbi/tblsection"
    nhrp_section.novaluetext = translate("There are no NHRP configurations yet")
    nhrp_section.extedit = luci.dispatcher.build_url("admin", "network", "routes", "dynamic_routes", "proto_nhrp", "%s")
    nhrp_section.defaults = {enabled = "0"}
    nhrp_section.sectionhead = "Name"

local enabled = nhrp_section:option(Flag, "enabled", "Enabled")

local interface = nhrp_section:option(DummyValue, "interface", "Interface")
    interface.placeholder = "Not defined"

local proto_address = nhrp_section:option(DummyValue, "proto_address", "NHS")
    proto_address.placeholder = "Not defined"

local nbma_address = nhrp_section:option(DummyValue, "nbma_address", "NBMA")
    nbma_address.placeholder = "Not defined"

return nhrp