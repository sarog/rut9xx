local map, section, vpn_config, vpn_certificates, empty = ...

e = section:taboption("secondarytab", ListValue, "_generate_vpn", translate("Generate SMS"), translate("Generate new SMS or use current configuration"))
	e:value("new", translate("New"))
	e:value("current", translate("From current configuration"))
	e.default = "new"
	function e.write() end

e = section:taboption("secondarytab", Flag, "_vpn", translate("OpenVPN"), translate("Include configuration for OpenVPN"))
	e:depends("_generate_vpn", "current")

function e.write(self, section, value)
	table.insert(vpn_config, "openvpn")
end

o = section:taboption("secondarytab", Flag, "_new_vpn", translate("OpenVPN"), translate("Include configuration for OpenVPN"))
o:depends("_generate_vpn", "new")

o = section:taboption("secondarytab", Flag, "enable", translate("Enable"), translate("Enable OpenVPN configuration"))
	o:depends({_generate_vpn = "new", _new_vpn = "1"})
	function o.write() end

o = section:taboption("secondarytab", ListValue, "role", translate("Role"), translate("A role that new OpenVPN instance will have"))
	o:value("server", "Server")
	o:value("client", "Client ")
	o:depends({_generate_vpn = "new", _new_vpn = "1"})
	o.nowrite = true
	function o.write() end

local dev_type = section:taboption("secondarytab", ListValue, "dev", translate("TUN/TAP"), translate("A type that virtual VPN interface will have") )
	dev_type:value("tun", translate("TUN (tunnel)"))
	dev_type:value("tap", translate("TAP (bridged)"))
	dev_type.default = "tun"
	dev_type:depends({_generate_vpn = "new", _new_vpn = "1"})
	function dev_type.write() end

local proto = section:taboption("secondarytab", ListValue, "proto", translate("Protocol"), translate("A transport protocol that will be used for connection"))
	proto:value("udp", translate("UDP"))
	proto:value("tcp", translate("TCP"))
	proto.default = "udp"
	proto:depends({_generate_vpn = "new", _new_vpn = "1"})
	function proto.write() end

o = section:taboption("secondarytab", Value, "port", translate("Port"), translate("TCP/UDP port that will be used for both local and remote endpoint. Make sure that this port will be open in firewall") )
	o.datatype = "port"
	o.default = "1194"
	o:depends({_generate_vpn = "new", _new_vpn = "1"})
	function o.write() end

lzo = section:taboption("secondarytab", Flag, "comp_lzo", "LZO", translate("Use fast LZO compression. With LZO compression your VPN connection will generate less network traffic") )
	lzo:depends({_generate_vpn = "new", _new_vpn = "1"})
	function lzo.write() end

-----client----
auth = section:taboption("secondarytab", ListValue,"_auth", translate("Authentication"), translate("Authentication mode that will be used to secure data session") )
	auth:value("skey", translate("Static key"))
	auth:value("tls", translate("TLS"))
	auth:value("pass", translate("Password"))
	auth:depends({role = "client", _generate_vpn = "new", _new_vpn = "1"})
	auth.default = "tls"
	function auth.write() end

auth = section:taboption("secondarytab", ListValue,"_auth_serv", translate("Authentication"), translate("Authentication mode that will be used to secure data session") )
	auth.default = "tls"
	auth:value("skey", translate("Static key"))
	auth:value("tls", translate("TLS"))
	auth:depends({role  = "server", _generate_vpn = "new", _new_vpn = "1"})
	function auth.write() end

o = section:taboption("secondarytab", Flag, "client_to_client", translate("Client to client"), translate("Allow client-to-client traffic") )
	o:depends({ _auth_serv="tls", role = "server", _generate_vpn = "new", _new_vpn = "1"})
	function o.write() end

oi = section:taboption("secondarytab", Value,"remote", translate("Remote host/IP address"), translate("IP address or domain name of OpenVPN server that will be used"))
	oi:depends({role = "client", _generate_vpn = "new", _new_vpn = "1"})
	oi.datatype = "ip4addr"
	function oi.write() end

o = section:taboption("secondarytab", Value, "resolv_retry" ,translate("Resolve retry"), translate("A time period in seconds that will be used to try to resolve server hostname before giving up"))
	o.default = "Infinite"
	o:depends({role = "client", _generate_vpn = "new", _new_vpn = "1"})
	o:depends({role = "server", _generate_vpn = "new", _auth_serv="skey", _new_vpn = "1"})
	function o.write() end

o = section:taboption("secondarytab", Value, "keepalive", translate("Keep alive"), translate("Try to keep a connection alive. Two values are required: ping_interval and ping_restart, e.g. 10 120") )
	o.default = "10 20"
	o:depends({role = "client", _generate_vpn = "new", _new_vpn = "1"})
	o:depends({dev="tap", role = "server", _generate_vpn = "new", _new_vpn = "1"})
	o:depends({dev = "tun", _auth_serv="tls", role = "server", _generate_vpn = "new", _new_vpn = "1"})
	function o.write() end

local ifconf_local = section:taboption("secondarytab", Value, "_ifconfig" ,translate("Local tunnel endpoint IP"), translate("IP address that will be used for virtual local network interface") )
	ifconf_local:depends({dev="tun", _auth="skey", role = "client", _generate_vpn = "new", _new_vpn = "1"})
	ifconf_local:depends({dev="tun", _auth_serv="skey", role = "server", _generate_vpn = "new", _new_vpn = "1"})
	ifconf_local.datatype = "ip4addr"
	function ifconf_local.write() end

local ifconf_remote = section:taboption("secondarytab", Value, "ifconfig", translate("Remote tunnel endpoint IP"), translate("IP address that will be used for virtual remote network interface") )
	ifconf_remote:depends({dev="tun", _auth="skey", role = "client", _generate_vpn = "new", _new_vpn = "1"})
	ifconf_remote:depends({dev="tun", _auth_serv="skey", role = "server", _generate_vpn = "new", _new_vpn = "1"})
	ifconf_remote.datatype = "ip4addr"
	function ifconf_remote.write() end

local route_ip = section:taboption("secondarytab", Value, "_route" ,translate("Remote network IP address"), translate("IP address that will be used for virtual remote network"))
	route_ip:depends({dev = "tun", role = "client", _generate_vpn = "new"})
	route_ip:depends({dev="tun", _auth_serv="skey", role = "server", _generate_vpn = "new"})
	route_ip:depends("_new_vpn", "1")
	--MANO route_ip:depends({_auth="skey"})
	route_ip.datatype = "ipaddr"
	function route_ip.write() end

local route_mask = section:taboption("secondarytab", Value, "route", translate("Remote network IP netmask"), translate("Subnet mask that will be used for remote virtual network"))
	route_mask:depends({dev = "tun", role = "client", _generate_vpn = "new", _new_vpn = "1"})
	route_mask:depends({dev="tun", _auth_serv="skey", role = "server", _generate_vpn = "new", _new_vpn = "1"})
	--MANO route_mask:depends({_auth="skey"})
	route_mask.datatype = "ip4addr"
	function route_mask.write() end

o = section:taboption("secondarytab", Value, "user", translate("User name"), translate("VPN client user name that will be used"))
	o:depends({_auth="pass", role = "client", _generate_vpn = "new", _new_vpn = "1"})
	function o.write() end

o = section:taboption("secondarytab", Value, "pass", translate("Password"), translate("VPN client password that will be used. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
	o:depends({_auth="pass", role = "client", _generate_vpn = "new", _new_vpn = "1"})
	o.datatype = "fieldvalidation('^[a-zA-Z0-9!@#$%%&*+/=?^_`{|}~.-]+$',5)"
	function o.write() end

--------server----------

local serv_ip = section:taboption("secondarytab", Value, "_server", translate("Virtual network IP address"), translate("IP address that will be used for virtual network"))
	--serv_ip:depends({dev="tun", _auth="tls"})
	serv_ip:depends({_auth_serv="tls", dev="tun", role = "server", _generate_vpn = "new", _new_vpn = "1"})
	serv_ip.datatype = "ip4addr"
	function serv_ip.write() end

local serv_mask = section:taboption("secondarytab", Value, "server" ,translate("Virtual network netmask"), translate("Subnet mask that will be used for virtual network"))
	--serv_mask:depends({dev="tun", _auth="tls"})
	serv_mask:depends({_auth_serv="tls", dev="tun", role = "server", _generate_vpn = "new", _new_vpn = "1"})
	serv_mask.forcewrite = true
	serv_mask.datatype = "ipaddr"
	function serv_mask.write() end

local multi_cl = section:taboption("secondarytab", Flag, "duplicate_cn", translate("Allow duplicate certificates"), translate("All clients can have same certificates"))
	--MANO multi_cl:depends("dev", "tap")
	multi_cl:depends({_auth_serv = "tls", role = "server", _generate_vpn = "new", _new_vpn = "1"})
	function multi_cl.write() end

o = section:taboption("secondarytab", FileUpload, "ca", translate("Certificate authority"), translate("The digital certificate verifies the ownership of a public key by the named subject of the certificate"))
	o:depends({_auth_serv="tls", role = "server", _new_vpn = "1"})
	o:depends({_auth="tls", role = "client", _new_vpn = "1"})
	o:depends({_auth="pass", role = "client", _new_vpn = "1"})
	o:depends("_new_vpn", "1")

o = section:taboption("secondarytab", FileUpload, "cert", translate("Server certificate"), translate("Certificate servers validate or certify keys as part of a public key infrastructure"))
	o:depends({_auth_serv="tls", role = "server", _new_vpn = "1"})
	o:depends({_auth="tls", role = "client", _new_vpn = "1"})
	o:depends("_new_vpn", "1")

o = section:taboption("secondarytab", FileUpload, "key", translate("Server key"), translate("It has been generated for the same purpose as server certificate"))
	o:depends({_auth_serv="tls", role = "server", _new_vpn = "1"})
	o:depends({_auth="tls", role = "client", _new_vpn = "1"})
	o:depends("_new_vpn", "1")

o = section:taboption("secondarytab", FileUpload, "dh", translate("Diffie Hellman parameters"), translate("Diffie-Hellman key exchange is a specific method of exchanging cryptographic keys"))
	o:depends({_auth_serv="tls", role = "server", _new_vpn = "1"})
	o:depends("_new_vpn", "1")

o = section:taboption("secondarytab", FileUpload, "secret", translate("Static pre-shared key"), translate("Pre-shared key (PSK) is a shared secret which was previously shared between the two parties using some secure channel before it needs to be used") )
	o:depends({_auth_serv="skey", role = "server", _new_vpn = "1"})
	o:depends({_auth="skey", role = "client", _new_vpn = "1"})
	o:depends("_new_vpn", "1")

function debug(string)
	os.execute("logger -s \""..string.."\"")
end
function m.on_parse(self)
	--cecho("Entering on parse.")

	local _auth
	_new_vpn = m:formvalue("cbid.sms_utils.cfgsms._new_vpn")
	_auth = m:formvalue("cbid.sms_utils.cfgsms._auth_serv") or m:formvalue("cbid.sms_utils.cfgsms._auth")
	role = m:formvalue("cbid.sms_utils.cfgsms.role")
	enable = m:formvalue("cbid.sms_utils.cfgsms.enable")
	proto = m:formvalue("cbid.sms_utils.cfgsms.proto")
	port = m:formvalue("cbid.sms_utils.cfgsms.port")
	keepalive = m:formvalue("cbid.sms_utils.cfgsms.keepalive")
	duplicate_cn = m:formvalue("cbid.sms_utils.cfgsms.duplicate_cn")
	server = m:formvalue("cbid.sms_utils.cfgsms.server")
	_server = m:formvalue("cbid.sms_utils.cfgsms._server")
	client_to_client = m:formvalue("cbid.sms_utils.cfgsms.client_to_client")
	comp_lzo = m:formvalue("cbid.sms_utils.cfgsms.comp_lzo")
	dev = m:formvalue("cbid.sms_utils.cfgsms.dev")

	route = m:formvalue("cbid.sms_utils.cfgsms.route")
	_route = m:formvalue("cbid.sms_utils.cfgsms._route")
	ifconfig = m:formvalue("cbid.sms_utils.cfgsms.ifconfig")
	_ifconfig = m:formvalue("cbid.sms_utils.cfgsms._ifconfig")
	remote = m:formvalue("cbid.sms_utils.cfgsms.remote")
	resolv_retry = m:formvalue("cbid.sms_utils.cfgsms.resolv_retry")

	ca = m:formvalue("cbid.sms_utils.cfgsms.ca")
	cert = m:formvalue("cbid.sms_utils.cfgsms.cert")
	key = m:formvalue("cbid.sms_utils.cfgsms.key")
	dh = m:formvalue("cbid.sms_utils.cfgsms.dh")
	secret = m:formvalue("cbid.sms_utils.cfgsms.secret")

	if not _auth or not role or not _new_vpn then
		return
	end

	local name = "sms"
	local vpn_inst = role.."_"..name

	if enable then
		table.insert(vpn_config, string.format("openvpn.%s=openvpn", vpn_inst))
		table.insert(vpn_config, string.format("openvpn.%s.persist_key=1", vpn_inst))
		table.insert(vpn_config, string.format("openvpn.%s.persist_tun=1", vpn_inst))
		table.insert(vpn_config, string.format("openvpn.%s.verb=5", vpn_inst))
		table.insert(vpn_config, string.format("openvpn.%s.enable=1", vpn_inst))
	else
		table.insert(vpn_config, string.format("openvpn.%s=openvpn", vpn_inst))
		table.insert(vpn_config, string.format("openvpn.%s.enable=0", vpn_inst))
	end

	if role == "client" then
		table.insert(vpn_config, string.format("openvpn.%s.nobind=1", vpn_inst))
		if _auth == "tls" then
			table.insert(vpn_config, string.format("openvpn.%s.client=1", vpn_inst))
		elseif _auth == "pass" then
			table.insert(vpn_config, string.format("openvpn.%s.client=1", vpn_inst))
			table.insert(vpn_config, string.format("openvpn.%s.auth_user_pass=/etc/openvpn/auth_%s", vpn_inst, vpn_inst))
		end
		if dev then
			if dev == "tun" then
				table.insert(vpn_config,  string.format("openvpn.%s.dev=%s_c_sms", vpn_inst, dev))
			else
				table.insert(vpn_config,  string.format("openvpn.%s.dev=%s", vpn_inst, dev))
			end
		end

		if _route then
			table.insert(vpn_config, string.format("openvpn.%s.route=%s", vpn_inst, _route))
		end

		if remote then
			table.insert(vpn_config, string.format("openvpn.%s.remote=%s", vpn_inst, remote))
		else
			table.insert(vpn_config, string.format("openvpn.%s.remote=Infinite", vpn_inst))
		end

		if proto then
			if proto == "tcp" then
				table.insert(vpn_config, string.format("openvpn.%s.proto=%s-client", vpn_inst, proto))
			else
				table.insert(vpn_config, string.format("openvpn.%s.proto=%s", vpn_inst, proto))
			end
		end
	elseif role == "server" then
		table.insert(vpn_config,  string.format("openvpn.%s.status=/tmp/openvpn-status_%s.log", vpn_inst, name))
		if _auth == "tls" then
			table.insert(vpn_config, string.format("openvpn.%s.client_config_dir=/etc/openvpn/ccd", vpn_inst))
			if dev_type == "tap" then
				table.insert(vpn_config, string.format("openvpn.%s.mode=server", vpn_inst))
			end
		end
		if dev then
			if dev == "tun" then
				table.insert(vpn_config,  string.format("openvpn.%s.dev=%s_s_sms", vpn_inst, dev))
			else
				table.insert(vpn_config,  string.format("openvpn.%s.dev=%s", vpn_inst, dev))
			end
		end

		if proto then
			if proto == "tcp" then
				table.insert(vpn_config, string.format("openvpn.%s.proto=%s-server", vpn_inst, proto))
			else
				table.insert(vpn_config, string.format("openvpn.%s.proto=%s", vpn_inst, proto))
			end
		end
	end

	if port and port ~= "" then
		table.insert(vpn_config, string.format("openvpn.%s.port=%s",vpn_inst, port))
	end

	if keepalive and keepalive ~= "" then
		table.insert(vpn_config, string.format("openvpn.%s.keepalive=%s", vpn_inst, keepalive))
	end

	if duplicate_cn and duplicate_cn ~= "" then
		table.insert(vpn_config, string.format("openvpn.%s.duplicate_cn=%s", vpn_inst, duplicate_cn))
	end

	if server and _server then
		if server ~= "" and _server ~= "" then
		table.insert(vpn_config, string.format("openvpn.%s.server=%s %s", vpn_inst, _server, server))
		end
	end

	if client_to_client and client_to_client ~= "" then
		table.insert(vpn_config, string.format("openvpn.%s.client_to_client=%s", vpn_inst, client_to_client))
	end

	if comp_lzo and comp_lzo == "1" then
		table.insert(vpn_config, string.format("openvpn.%s.comp_lzo=yes", vpn_inst))
	end

	if route and _route then
		if route ~= "" and _route ~= "" then
			table.insert(vpn_config, string.format("openvpn.%s.route=%s %s", vpn_inst, _route, route))
		end
	end

	if ifconfig and _ifconfig then
		if ifconfig ~= "" and _ifconfig ~= "" then
			table.insert(vpn_config, string.format("openvpn.%s.ifconfig=%s %s", vpn_inst, _ifconfig, ifconfig))
		end
	end

	if resolv_retry and resolv_retry ~= "" then
		table.insert(vpn_config, string.format("openvpn.%s.resolv_retry=%s", vpn_inst, resolv_retry))
	end

	if ca and ca ~= "" then
		table.insert(vpn_config, string.format("openvpn.%s.ca=%s", vpn_inst, ca))
	end

	if cert and cert ~= "" then
		table.insert(vpn_config, string.format("openvpn.%s.cert=%s", vpn_inst, cert))
	end

	if key and key ~= "" then
		table.insert(vpn_config, string.format("openvpn.%s.key=%s", vpn_inst, key))
	end

	if dh and dh ~= "" then
		table.insert(vpn_config, string.format("openvpn.%s.dh=%s", vpn_inst, dh))
	end

	if secret and secret ~= "" then
		table.insert(vpn_config, string.format("openvpn.%s.secret=%s", vpn_inst, secret))
	end

	if _auth ~= "" then
		table.insert(vpn_config, string.format("openvpn.%s._auth=%s", vpn_inst, _auth))
	end

--cecho("Exiting on parse.")
end
