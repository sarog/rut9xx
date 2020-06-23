
m = Map("vrrpd", translate("VRRP Configuration"))

s = m:section( NamedSection, "advanced","vrrp", translate("VRRP Settings"))

delay = s:option( Value, "delay", translate("Swap delay"), translate("Optional delay time (in minutes) where the swap to the other router is delayed"))
delay.datatype = "range(0,9999)"
delay.default = "0"

s2 = m:section( NamedSection, "ping","vrrp", translate("Check WAN Connection"))

o = s2:option( Flag, "enabled", translate("Enable"), translate("Enable WAN\\'s connection monitoring"))
o.rmempty = false
o.default = "0"

host = s2:option( Value, "host", translate("Ping IP address"), translate("A host to send ICMP (Internet Control Message Protocol) packets to"))
host.datatype = "ip4addr"

interval = s2:option( Value, "interval", translate("Ping interval"), translate("Time interval in minutes between two PINGs"))
interval.datatype = "range(0,1440)"
interval.default = "10"

t_out = s2:option( Value, "time_out", translate("Ping timeout (sec)"), translate("Response timeout value, interval [1 - 9999]"))
t_out.datatype = "range(0,9999)"
t_out.default = "1"

size = s2:option( Value, "packet_size", translate("Ping packet size"), translate("ICMP (Internet Control Message Protocol) packet\\'s size, interval [0 - 1000]"))
size.datatype = "range(0,1000)"

retry = s2:option( Value, "retry", translate("Ping retry count"), translate("Failed ping attempts\\' count before determining that connection is lost"))
retry.datatype = "range(0,100)"

s3 = m:section(NamedSection, "check","vrrp", translate("VRRP Events Configuration"))

o = s3:option( Flag, "check_signal", translate("Check signal strength"), translate("Switch to backup router if this router's signal is to weak"))
o.rmempty = false
o.default = "0"

retry = s3:option( Value, "signal", translate("Singal strength"), translate("Signal strength value in dBm"))
retry.datatype = "integer"

o = s3:option( Flag, "check_conn", translate("Check connection type"))
o.rmempty = false
o.default = "0"

o = s3:option( ListValue, "conn_type", translate("Connection type"))
o:value("2g", "2G")
o:value("3g", "3G")
o:value("lte", "LTE")

return m
