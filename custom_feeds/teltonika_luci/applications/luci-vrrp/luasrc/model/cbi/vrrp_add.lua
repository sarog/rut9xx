--
-- Created by IntelliJ IDEA.
-- User: robertasj
-- Date: 10/7/19
-- Time: 8:52 AM
-- To change this template use File | Settings | File Templates.
--

local vrrpd_map = Map("vrrpd", translate("VRRP Configurations"), translate(""))

local vrrpd_section = vrrpd_map:section( TypedSection, "vrrpd", translate("Configurations"))
    vrrpd_section.addremove = true
    vrrpd_section.template = "vrrp/tblsection"
    vrrpd_section.novaluetext = translate("There are no VRRP configurations yet")
    vrrpd_section.extedit = luci.dispatcher.build_url("admin", "services", "vrrp", "%s")
    vrrpd_section.defaults = { enabled = "0" }
    vrrpd_section.sectionhead = "Name"

local enabled = vrrpd_section:option(Flag, "enabled", translate("Enabled"))

local v_id = vrrpd_section:option( DummyValue, "virtual_id", translate("Virtual ID"))

local priority = vrrpd_section:option( DummyValue, "priority", translate("Priority"))

local v_ip = vrrpd_section:option( DummyValue, "virtual_ip", translate("Virtual address"))

vrrpd_section:option( DummyValue, "_state", translate("State")).value = "-"

vrrpd_section:option( DummyValue, "_master_ip", translate("Master's IP")).value = "-"

return vrrpd_map