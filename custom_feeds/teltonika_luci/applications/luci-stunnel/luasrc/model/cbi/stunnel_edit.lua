local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()
local fw = require "luci.model.firewall"

local section_name

if arg[1] then
	section_name = arg[1]
else
	luci.http.redirect(luci.dispatcher.build_url("admin", "services", "vpn", "stunnel"))
end

local m = Map("stunnel", translate("Stunnel"), translate(""))

m.redirect = luci.dispatcher.build_url("admin", "services", "vpn", "stunnel")
local s = m:section(NamedSection, section_name, "service", translate("Stunnel Configuration"))

local stunnel_on = s:option( Flag, "enabled", translate("Enable"), translate("Enable Stunnel"))


	function stunnel_on.on_enable(self, section)

	end

	function stunnel_on.on_disable(self, section)

    end

o = s:option( ListValue, "client", translate("Operating Mode"), translate("Stunnel operation mode. <br> * Server - Only listening on specified IP and Port. <br> * Client - Both listening and connecting to specified IPs"))
	o.default = "0"
	o:value("0",translate("Server"))
	o:value("1",translate("Client"))

o = s:option( Value, "accept_host", translate("Listen IP"), translate("IP which server will be listening to"))
    o.datatype = "or(ipaddr, hostname)"

o = s:option( Value, "accept_port", translate("Listen Port"), translate("Port number which server will be listenting to (range 0:65535)"))

local connect = s:option( DynamicList, "connect", translate("Connect IP's"), translate("Type in the server IP and Port (using \"host_ip:port\" convention (i.e. 127.0.0.1:6001)).<br> If no host IP is specified only port number, then localhost will be used as a host"))

local tls_cipher = s:option(ListValue, "ciphers", translate("TLS Cipher"), translate("Packet encryption algorithm (cipher)") )
tls_cipher:value("none", translate("None"))
tls_cipher:value("dhe_rsa", translate("Secure"))
tls_cipher:value("custom", translate("Custom"))

local tls_cipher2 = s:option(DynamicList, "_cipher", translate("Allowed TLS Ciphers"))
tls_cipher2:depends({ciphers="custom"}) 

function tls_cipher.cfgvalue(self, section)
    local tls_ciphers = uci:get("stunnel", section_name, "_cipher_type")
    if tls_ciphers ~= nil and tls_ciphers ~= "" then
        return tls_ciphers
    end
end

function tls_cipher.write(self, section, value)
    if value and value ~= "" then
        uci:set(self.config, section, "_cipher_type", value)
        uci:save(self.config)
    end

    if value == "dhe_rsa" then
        return AbstractValue.write(self, section, { "ECDHE-RSA-AES256-SHA384", "ECDHE-RSA-AES128-SHA256", "ECDHE-RSA-AES256-SHA", "AES128-GCM-SHA256", "AES256-SHA256", "AES128-SHA256", "AES256-SHA", "AES128-SHA", "DHE-RSA-AES256-SHA256"})
    end
    if value == "custom" then
        local val = luci.http.formvalue("cbid.stunnel."..section_name.."._cipher")
        return AbstractValue.write(self, section, val)
    end
    if value == "none" then
        return AbstractValue.write(self, section, { "" })
    end

end

function tls_cipher2.cfgvalue(self, section)
    local ciphers = uci:get("stunnel", section_name, "ciphers")
    if ciphers ~= nil and ciphers ~= "" then
        return ciphers
    end
end

function tls_cipher2.write(self, section)
end

o = s:option( ListValue, "protocol", translate("Applicaton Protocol"), translate("This option enables initial, protocol-specific negotiation of the TLS encryption.<br> The protocol option should not be used with TLS encryption on a separate port."))
	o.default = ""
	o:value("connect",translate("Connect"))
	o:value("smtp",translate("SMTP"))
	--o:value("cifs",translate("CIFS"))
	--o:value("imap",translate("IMAP"))
	--o:value("nntp",translate("NNTP"))
	--o:value("pgsql",translate("PostgreSQL"))
	--o:value("pop3",translate("POP3"))
	--o:value("proxy",translate("Proxy"))
	--o:value("socks",translate("SOCKS"))
	o:value("",translate("Not specified"))

local proto_auth_connect = s:option(ListValue, "protocolAuthentication_connect", translate("Protocol Authentication"), translate("Packet encryption algorithm (cipher)") )
proto_auth_connect:depends("protocol", "connect")
proto_auth_connect:value("basic", translate("Basic"))
proto_auth_connect:value("ntlm", translate("NTLM"))

function proto_auth_connect.write(self, section, value)
    self.option = "protocolAuthentication"
    return AbstractValue.write(self, section, value)
end

local proto_auth_smtp = s:option(ListValue, "protocolAuthentication_smtp", translate("Protocol Authentication"), translate("Packet encryption algorithm (cipher)") )
proto_auth_smtp:depends("protocol", "smtp")
proto_auth_smtp:value("plain", translate("Plain"))
proto_auth_smtp:value("login", translate("Login"))

function proto_auth_smtp.write(self, section, value)
    self.option = "protocolAuthentication"
    return AbstractValue.write(self, section, value)
end

local proto_domain = s:option(Value, "protocolDomain", translate("Protocol Domain"), translate("Domain for the protocol negotiations") )
proto_domain:depends({ protocol = "connect", client = "1"})

local proto_host = s:option(Value, "protocolHost", translate("Protocol Host"), translate("Specifies the final TLS server to be connected to by the proxy, and not the proxy server directly connected by stunnel") )
proto_host:depends({ protocol = "connect", client = "1"})

local proto_user = s:option(Value, "protocolUsername", translate("Protocol Username"), translate("Username for the protocol negotiations") )
proto_user:depends({ protocol = "connect", client = "1"})
proto_user:depends({ protocol = "smtp", client = "1"})

local proto_pass = s:option(Value, "protocolPassword", translate("Protocol Password"), translate("Password for the protocol negotiations. Allowed characters: a-zA-Z0-9!@#$%&*+-/=?^_`{|}~.<>:;[]") )
proto_pass.datatype = "password"
proto_pass:depends({ protocol = "connect", client = "1"})
proto_pass:depends({ protocol = "smtp", client = "1"})

local cert = s:option( FileUpload, "cert", translate("Certificate File"), translate("The parameter specifies the file containing certificates used by stunnel to authenticate itself against the remote client or server. <br>The file should contain the whole certificate chain starting from the actual server/client certificate, and ending with the self-signed root CA certificate."))
cert.size = "51200"
cert.sizetext = translate("Selected file is too large. Maximum allowed size is 50 KiB")
cert.sizetextempty = translate("Selected file is empty")

local key = s:option( FileUpload, "key", translate("Private Key"), translate("A private key is needed to authenticate the certificate owner."))
key.size = "51200"
key.sizetext = translate("Selected file is too large. Maximum allowed size is 50 KiB")
key.sizetextempty = translate("Selected file is empty")

function m.on_before_save()
    local cert_file = m:get(section_name, "cert")
    local key_file = m:get(section_name, "key")

    if cert_file then
        m:set(section_name, "cert", "/etc/stunnel/stunnel_"..section_name..".crt")
        luci.sys.exec("cp -rf "..cert_file.." /etc/stunnel/stunnel_"..section_name..".crt")
    else
        m:set(section_name, "cert", "")
    end

    if key_file then
        m:set(section_name, "key", "/etc/stunnel/stunnel_"..section_name..".key")
        luci.sys.exec("cp -rf "..key_file.." /etc/stunnel/stunnel_"..section_name..".key")
    else
        m:set(section_name, "key", "")
    end
end

function m.on_commit()
    luci.sys.exec("/etc/init.d/stunnel restart >/dev/null 2>&1 &")
end

return m
