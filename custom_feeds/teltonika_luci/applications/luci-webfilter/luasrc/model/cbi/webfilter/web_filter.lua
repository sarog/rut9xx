local m, sc, sc1, enb_block, mode, enb_col, dns_col


m = Map("hostblock", translate("Site Blocking Settings"), translate(""))
m.addremove = false

FileUpload.size = "100000"
FileUpload.sizetext = translate("Selected file is too large, max size is 100 KB")
FileUpload.sizetextempty = translate("Selected file is empty")

sc = m:section(NamedSection, "config", "hostblock", translate("Site Blocking"))

enb_block = sc:option(Flag, "enabled", translate("Enable"), translate("Enable DNS based site blocking"))
enb_block.rmempty = false

mode = sc:option(ListValue, "mode", translate("Mode"), translate("Whitelist - allow every site on the list and block everything else. Blacklist - block every site on the list and allow everything else"))
mode:value("whitelist", translate("Whitelist"))
mode:value("blacklist", translate("Blacklist"))
mode.default = "blacklist"

file_to_upload = sc:option( FileUpload, "site_blocking_hosts", translate("Hosts list"), translate("Upload a file with many hosts, one hostname per file line (max file size allowed is 100KB)"))

sc1 = m:section(TypedSection, "block")
sc1.addremove = true
sc1.anonymous = true
sc1.template = "cbi/tblsection"
sc1.novaluetext = translate("There are no site blocking rules created yet")

enb_col = sc1:option(Flag, "enabled", translate("Enable"), translate("Check to enable site blocking"))
enb_col.rmempty = false
enb_col.default = "1"

dns_col = sc1:option(Value, "host", translate("Hostname"), translate("Block/allow site with this hostname. example.com"))
dns_col.datatype = "hostname"

m.message = translate("err: Be careful not to block Yourself when using VPN or other services!")

return m
