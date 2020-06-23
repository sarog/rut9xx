local function cecho(string)
	luci.sys.call("echo \"" .. string .. "\" >> /tmp/log.log")
end

local util = require "luci.util"
local uci = require "luci.model.uci".cursor()

m = Map("rs", translate("RS485 Configuration"))

sys = m:section(TypedSection, "rs485", translate("RS485 Serial Configuration"))
sys.addremove = false
sys.anonymous = true

if (m.uci:get("modbus_serial_master", "rs485", "enabled") ~= "1" or m.uci:get("rs", "rs485", "enabled") == "1") then
	o = sys:option(Flag, "enabled", translate("Enabled"), translate("Check to enable RS485 serial configuration"))
else
	o = sys:option(DummyValue, "", "")
	o.default = "<center>RS485 cannot be used if <b>Services -> Modbus -> Modbus Serial Master -> RS485 -> Enabled</b> is on.</center>"
	o.rawhtml = true
end

o = sys:option(ListValue, "baudrate", translate("Baud rate"), translate("Select supported baud rate"))
o:value("300", "300")
o:value("1200", "1200")
o:value("2400", "2400")
o:value("4800", "4800")
o:value("9600", "9600")
o:value("19200", "19200")
o:value("38400", "38400")
o:value("57600", "57600")
o:value("115200", "115200")
o.default = 115200

p = sys:option(ListValue, "parity", translate("Parity"), translate("Select what kind of parity bit to use for error detection"))
p:value("none", translate("None"))
p:value("odd", translate("Odd"))
p:value("even", translate("Even"))
p.default = none

fl = sys:option(ListValue, "flowcontrol", translate("Flow control"), translate("Select what kind of characters to use for flow control"))
fl:value("none", "None")
--fl:value("rts/cts", "RTS/CTS")
fl:value("xon/xoff", "Xon/Xoff")
fl.default = "none"

styp = sys:option(ListValue, "type", translate("Serial type"), translate("Select function of serial interface"))
styp:value("console", translate("Console"))
styp:value("overip", translate("Over IP"))
styp:value("modem", translate("Modem"))
styp:value("modbus", translate("Modbus gateway"))
styp:value("ntrip", translate("NTRIP client"))


nip = sys:option(Value, "ntrip_ip", translate("IP address"), translate("NTRIP server DNS or IP address"))
nip:depends("type","ntrip")
nip.datatype = "or(ip4addr, hostname)"
nip.default = "0.0.0.0"

nport = sys:option( Value, "ntrip_port", translate("Port"), translate("TCP/UDP port for NTRIP communication") )
nport:depends("type","ntrip")
nport.datatype = "port"

nmount = sys:option( Value, "ntrip_mount_point", translate("Mount point"), translate("NTRIP mount point") )
nmount:depends("type","ntrip")

nformat = sys:option(ListValue, "ntrip_data_format", translate("Data format"), translate("NTRIP data format"))
nformat:value("h", translate("NTRIP v2.0 TCP/IP"))
nformat:value("r", translate("NTRIP v2.0 RSTP/RTP"))
nformat:value("n", translate("NTRIP V1.0"))
nformat:value("a", translate("Automatic detection"))
nformat:value("u", translate("NTRIP v2.0 UDP"))
nformat:depends("type","ntrip")
nformat.default = "n"

nusr = sys:option(Value, "ntrip_user", translate("User name"), translate("Name for NTRIP authentication"))
nusr:depends("type","ntrip")

npsw = sys:option(Value, "ntrip_password", translate("Password"), translate("Password for NTRIP authentication. Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:; []"))
npsw:depends("type","ntrip")
npsw.password = true
npsw.datatype = "password(1)"

user_nmea = sys:option(Value, "user_nmea", translate("Initial NMEA-GGA"), translate("Initial NMEA-GGA string that will be used when initiating connection with NTRIP server"))
user_nmea:depends("type","ntrip")

if uci:get("hwinfo", "hwinfo", "gps") == '1' then
	use_router_gps = sys:option(Flag, "use_router_gps", translate("Acquire NMEA-GGA"), translate("Enable to obtain Initial NMEA-GGA string from this device GPS module. Only works when GPS fix is obtained. Make sure that the GPS antena is connected."))
	use_router_gps:depends("type","ntrip")
end

wan=luci.sys.exec(". /lib/teltonika-functions.sh; tlt_get_wan_ipaddr")
lan=luci.sys.exec("uci get -q network.lan.ipaddr")
mi = sys:option(Value, "modbus_ip", translate("Listening IP"), translate("DNS or IP address on which Modbus Gateway should listen for incoming connections."))
mi:depends("type","modbus")
mi:value("0.0.0.0")
if lan ~= nil then
	mi:value(lan)
end
if wan ~= nil then
	mi:value(wan)
end
mi.datatype = "or(ip4addr, hostname)"
mi.default = "0.0.0.0"

mp = sys:option(Value, "modbus_port", translate("Port"), translate("Port number for Modbus Gateway."))
mp:depends("type","modbus")
mp.datatype = "port"
mp.default = "502"

ms = sys:option(ListValue, "slave_id_config", translate("Slave ID configuration type"), translate("Redirects ModBus TCP to RTU"));
ms:depends("type", "modbus")
ms:value("single", translate("User defined"))
ms:value("multiple", translate("Obtained from TCP"))
ms.default = "single"

mid = sys:option(Value, "multi_slave_id", translate("Permitted slave IDs"), translate("Redirect packets whose slave IDs is on the list. Individual IDs are separated by \',\' (comma) and range is specifed by \'-'\ (hyphen)"))
mid:depends("slave_id_config", "multiple")
mid.default = "1-247"
function mid.validate(self, value)
		
		for i = 1, #value do
        	        local c = value:byte(i)
                	if (i == 1 or i == value:len()) and (c == 44 or c == 45) then
                        	return nil
	                end
        	        if c ~= 44 and c ~= 45 and c < 48 and c > 57 then
                	        return nil
	                end
        	end
	        	
        	return value
end

smid = sys:option(Value, "single_slave_id", translate("Slave ID"), translate("Redirect packets to Modbus slave ID"))
smid:depends("slave_id_config", "single")
smid.default = "1"
		                                                        
			                                                                                                                                                                                                                                                                                                                                                                                                                                                                                

prot = sys:option(ListValue, "protocol", translate("Protocol"), translate("Select which protocol to use for data transmission"))
prot:value("tcp", translate("TCP"))
prot:value("udp", translate("UDP"),{mode = "server"}, {mode = "client"})
prot:depends("type","overip")


mode = sys:option(ListValue, "mode", translate("Mode"), translate("Select mode to apply for router"))
mode:value("server", translate("Server"))
mode:value("client", translate("Client"))
mode:value("bidirect", translate("Bidirect"))
mode:depends("type","overip")

always_reconnect = sys:option(Flag, "always_reconnect", translate("Always reconnect"), translate("Make new tcp connection after sending every data package"))
always_reconnect.rmempty = false
always_reconnect:depends("protocol","tcp") 

o = sys:option(DummyValue, "getinfo", translate("Client settings:"), translate(""))
-- o:depends("mode","client")
o:depends("mode","bidirect")

addr = sys:option(Value, "ip_connect", translate("Server Address"), translate("Specify server address which client have to connect"))
addr:depends("mode","client")
addr:depends("mode","bidirect")
addr.datatype = "host"

tp = sys:option(Value, "port_connect", translate("TCP port"), translate("Specify port number for the connectivity establishment"))
tp:depends("mode","client")
tp:depends("mode","bidirect")
tp.datatype = "port"

ali= sys:option(Value, "recon_interval", translate("Reconnect interval (s)"), translate("Specify intervals connection to server if it fails"))
ali:depends("mode","client")
ali:depends("mode","bidirect")
ali.datatype = "integer"

o = sys:option(DummyValue, "getinfo2", translate("Server settings:"), translate(""))
-- o:depends("mode","server")
o:depends("mode","bidirect")

tp = sys:option(Value, "port_listen", translate("Port"), translate("Specify port number for server to listen"))
tp:depends("mode","server")
tp:depends("mode","bidirect")
tp.datatype = "port"

ali= sys:option(Value, "timeout", translate("Timeout (s)"), translate("Disconnect client if not active connection"))
ali:depends("mode","server")
ali:depends("mode","bidirect")
ali.datatype = "integer"

ali= sys:option(ListValue, "gpio", translate("Output"), translate("Output to manage"))
ali:depends("mode","bidirect")
ali:value("56", translate("OC Output"))
ali:value("57", translate("Relay Output"))

ali= sys:option(ListValue, "gpiostate", translate("Output state"), translate("Default output state value, then start application"))
ali:depends("mode","bidirect")
ali:value("0", translate("0"))
ali:value("1", translate("1"))

aa = sys:option(Value, "direct_connect", translate("Direct connect"), translate("Enter hostname:port to maintain constant connection to specified host. Leave empty to use ATD command to initiate connection"))
aa:depends("type","modem")

aa = sys:option(Value, "modem_port", translate("TCP port"), translate("Specify port to listen for incomming connections. Leave empty to disable incomming connections"))
aa:depends("type","modem")
aa.datatype = "port"

aa = sys:option(Value, "init_string", translate("Initiation string"), translate("Command string that will be sent to modem to initate it in any special way (optional)"))
aa:depends("type","modem")
aa.datatype = "nospace"

aa = sys:option(Flag, "use_alternative_crlf", translate("No extra CR LF in response"), translate("Check to remove extra CR LF before and LF after response code"))
aa:depends("type","modem")

s = m:section(TypedSection, "ip_filter_rs485", translate(""))
s.template = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.template_addremove = "rs/add_interface_rule"

function s.create(self, section)
	local a = m:formvalue("_newinput.action")
	local add = m:formvalue("cbi-ip_filter_rs485")
	exist = 0
	if add then
		uci:foreach("rs", "ip_filter_rs485", function(s)
			if s.interface == a then
				exist = 1
			end
		end)
		if exist == 0 then
			created = TypedSection.create(self, section)
			self.map:set(created, "interface", a)
		end
	end
	uci:commit("rs")

end

d = s:option(DummyValue, "interface", translate("Interface"), translate(""))
function d.cfgvalue(self, s)
	local z = self.map:get(s, "interface")
	if z == "lan" then
		return translate("LAN")
	elseif z == "wan" then
		return translate("WAN")
	elseif z == "vpn" then
		return translate("VPN")
	end
end

p = s:option(DynamicList, "allow_ip", translate("Allow IP"), translate("Allow ip connecting to server, 0.0.0.0/0 for allowing all"))
p.datatype = "ipaddr"

function m.on_commit()
	local use_router_gps = luci.util.trim(luci.sys.exec("uci -q get rs.rs485.use_router_gps")) or ""
	local have_gps_device = luci.util.trim(luci.sys.exec("uci -q get hwinfo.hwinfo.gps")) or ""
	local type = luci.util.trim(luci.sys.exec("uci -q get rs.rs485.type")) or ""
	local enabled = luci.util.trim(luci.sys.exec("uci -q get rs.rs485.enabled")) or ""

	if enabled == "1" and type == "ntrip" and have_gps_device == "1" and use_router_gps == "1"  then
		luci.sys.exec("uci -q set gps.gpsd.enabled=1")
		luci.sys.exec("uci -q commit gps")
	end
end


return m
