m=Map("quagga", translate("General Protocols Configuration"))

s = m:section(TypedSection, "staic_route", translate("Static Routes"), translate(""))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.sorthint  = translate("All rules are executed in current list order")
s.novaluetext = translate("There are no static routes created yet")

o =s:option(Value, "destination", "Destination")
	o.datatype = "ip4addr"

o = s:option(Value, "mask", "Netmask")
	o.datatype = "ip4addr"

o = s:option(Value, "gateway", "Gateway")
	o.datatype = "ip4addr"

return m
