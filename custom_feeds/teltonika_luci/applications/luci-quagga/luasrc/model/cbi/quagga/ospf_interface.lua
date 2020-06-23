local dsp = require "luci.dispatcher"

if not arg[1] or arg[1] == "" then
	luci.http.redirect(dsp.build_url("admin/network/routes/dynamic_routes/proto_ospf"))
  return
end

local m = Map("quagga", translatef("OSPF Interface Configuration"), "")
  m.redirect = dsp.build_url("admin/network/routes/dynamic_routes/proto_ospf")

local s = m:section( NamedSection, arg[1], "ospf_interface", translate("General Settings"), "")

s:option(Flag, "enabled", translate("Enable"))

o = s:option(Value, "cost", translate("Cost"), translate("The cost value is set to router-LSA’s metric field and used for SPF calculation."))
  o.datatype = "range(1,65535)"

o = s:option(Value, "hello_interval", translate("Hello Interval"), translate("Setting this value, Hello packet will be sent every timer value seconds on the specified interface"))
  o.datatype = "range(1, 65535)"
  o.default = 10

o = s:option(Value, "dead_interval", translate("Router Dead Interval"), translate("This value must be the same for all routers attached to a common network."))
  o.datatype = "range(1, 65535)"
  o.default = 40

o = s:option(Value, "retransmit_interval", translate("Retransmit"), translate("This value is used when retransmitting Database Description and Link State Request packets."))
  o.datatype = "range(0,65535)"
  o.default = "5"

o = s:option(Value, "priority", translate("Priority"), translate("The router with the highest priority will be more eligible to become Designated Router. Setting the value to 0, makes the router ineligible to become Designated Router."))
  o.datatype = "range(0,255)"
  o.default = "1"

o = s:option(ListValue, "typ", translate("Type"), translate(""))
  o:value("", translate(""))
  o:value("broadcast", translate("Broadcast"))
  o:value("non-broadcast", translate("Nonbroadcast"))
  o:value("point-to-point", translate("Poin-to-point"))
  o:value("point-to-multipoint", translate("Point-to-multipoint"))

o = s:option(ListValue, "authentication", translate("Authentication"), translate(""))
  o:value("none", translate("None"))
  o:value("pass", translate("Password"))
  o:value("md5_hmac", translate("MD5 HMAC"))

o = s:option(Value, "id", translate("ID"), translate(""))
  o:depends({authentication="md5_hmac"})
  o.datatype = "range(1,100)"

o = s:option(Value, "password", translate("Password"), translate(""))
  o:depends({authentication="pass"})
  o:depends({authentication="md5_hmac"})
  o.password = true



return m
