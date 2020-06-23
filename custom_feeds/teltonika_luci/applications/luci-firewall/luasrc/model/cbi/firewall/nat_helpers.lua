local m, s, name, flag

m = Map("kmod_man",
	translate("NAT Helpers"),
	translate("Manage protocol specific NAT helpers."))

s = m:section(TypedSection, "module", translate("NAT Helpers"))
s.anonymous = true
s.addremove = false
s.template = "cbi/tblsection"

name = s:option(DummyValue, "name", translate("Protocol"))
name.rmempty = false

flag = s:option(Flag, "enabled", translate("Enable"))
flag.rmempty = false

return m
