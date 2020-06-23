local m

m = Map("smpp_config", translate("SMPP Server Configuration"), 
	translate(""))
m.addremove = false

sc = m:section(NamedSection, "smpp","smpp", translate("Transmitter Configuration"))

enb_block = sc:option(Flag, "enabled", translate("Enable"), translate("Enables (checked) and disables (unchecked) smpp server"))
enb_block.rmempty = false

o = sc:option(Value, "username", translate("User name"), translate("User name for authentication on SMPP server. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
o.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~. -]+$',0)"
o.default = "admin"

o = sc:option(Value, "password", translate("Password"), translate("Password for authentication on SMPP server. Allowed characters (a-zA-Z0-9!@#$%&*+/=?^_`{|}~. -<>:;[])"))
o.password = true
o.datatype = "password(1)"
o.default = "admin01"

o = sc:option(Value, "port", translate("Server port"), translate("A port that will be used for smpp server communications. Allowed all not used ports (0-65535)"))
o.datatype = "port"
o.default = "7777"

return m

