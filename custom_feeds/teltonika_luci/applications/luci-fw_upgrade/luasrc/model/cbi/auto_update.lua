local m, s1, e1, t

--Map(config, title, description)
m = Map("rut_fota", translate("Fota service settings"), translate(""))

--display_apply = false;

--Named section(name, type, title, description) An object describing an UCI section selected by the name
s1 = m:section(NamedSection, "config", "rutfota", translate("Firmware Over The Air Configuration"))

--option class Fag (option, title, description)
e1 = s1:option(Flag, "enabled", translate("Enable FOTA"), translate("Check to enable FOTA service auto check"))
e1.rmempty = false

return m
