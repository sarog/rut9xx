
local utl = require "luci.util"
local nw = require "luci.model.network"
local sys = require "luci.sys"
local ntm = require "luci.model.network".init()
local m
local filterPath = "/etc/tinyproxy/" 
local filterFile = "filter"
local not_dns = false

local function debug(string)
	luci.sys.call("echo \"" .. string .. "\" >> /tmp/filter.log")
end

m = Map("privoxy", translate("Proxy Based URL Content Blocker Configuration"),
	translate(""))
m.addremove = false
m.message = translate("err: Proxy Based Content Blocker can only be used to block HTTP websites. For blocking HTTPS websites, consider using the Site Blocking option")

FileUpload.size = "100000"
FileUpload.sizetext = translate("Selected file is too large, max size is 100 KB")
FileUpload.sizetextempty = translate("Selected file is empty")

s = m:section(NamedSection, "privoxy", "privoxy", translate("Proxy Based URL Content Blocker"))
s.addremove = false

--watcher = s:option(Value, "watcher")
--watcher.template  = "cbi/watcher"

enb = s:option(Flag, "enabled", translate("Enable"), translate("Enable proxy server based URL content blocking. Works with HTTP protocol only"))
enb.rmempty = false

mode = s:option(ListValue, "mode", translate("Mode"), translate("Whitelist - allow every part of URL on the list and block everything else. Blacklist - block every part of URL on the list and allow everything else"))
mode:value("whitelist", translate("Whitelist"))
mode:value("blacklist", translate("Blacklist"))
mode.default = "blacklist"

file_to_upload = s:option( FileUpload, "proxy_blocking_hosts", translate("Hosts list"), translate("Upload a file with many hosts, one hostname per file line (max file size allowed is 100KB)"))



s2 = m:section(TypedSection, "rule", translate("URL Filter Rules"))
s2.addremove = true
s2.anonymous = true
s2.template  = "cbi/tblsection"
s2.novaluetext = translate("There are no URL filter rules created yet")

enb_rul = s2:option(Flag, "enabled", translate("Enable"), translate("Make a rule active/inactive"))
enb_rul.rmempty = false
enb_rul.default = "1"

url_cont = s2:option(Value, "domen", translate("URL content"), translate("Block/allow any URL containing this string. example.com, example.*, *.example.com"))
url_cont:depends("custom", "")
 
return m
