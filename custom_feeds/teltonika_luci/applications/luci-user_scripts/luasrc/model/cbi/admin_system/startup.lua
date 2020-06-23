--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2010-2012 Jo-Philipp Wich <xm@subsignal.org>
Copyright 2010 Manuel Munz <freifunk at somakoma dot de>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: startup.lua 8674 2012-05-06 09:48:06Z jow $
]]--

local fs = require "nixio.fs"
local http = require("luci.http")

require "luci.fs"
require "luci.sys"
require "luci.util"
require "luci.http"

local inits = { }
 
real_local_file = ""
tmp_local_file = ""

-- for _, name in ipairs(luci.sys.init.names()) do
-- 	local index   = luci.sys.init.index(name)
-- 	local enabled = luci.sys.init.enabled(name)
-- 
-- 	if index < 255 then
-- 		inits["%02i.%s" % { index, name }] = {
-- 			name    = name,
-- 			index   = tostring(index),
-- 			enabled = enabled
-- 		}
-- 	end
-- end

--	if index < 255 then
--		inits["%02i.%s" % { index, name }] = {
--			name    = name,
--			index   = tostring(index),
--			enabled = enabled
--		}
--	end
--end

-- m = SimpleForm("initmgr", translate("Initscripts"), translate("You can enable or disable installed init scripts here. Changes will applied after a device reboot.<br /><strong>Warning: If you disable essential init scripts like \"network\", your device might become inaccesable!</strong>"))
-- m.reset = false
-- m.submit = false
--[[
s = m:section(Table, inits)

i = s:option(DummyValue, "index", translate("Start priority"))
n = s:option(DummyValue, "name", translate("Initscript"))


e = s:option(Button, "endisable", translate("Enable/Disable"))

e.render = function(self, section, scope)
	if inits[section].enabled then
		self.title = translate("Enabled")
		self.inputstyle = "save"
	else
		self.title = translate("Disabled")
		self.inputstyle = "reset"
	end

	Button.render(self, section, scope)
end

e.write = function(self, section)
	if inits[section].enabled then
		inits[section].enabled = false
		return luci.sys.init.disable(inits[section].name)
	else
		inits[section].enabled = true
		return luci.sys.init.enable(inits[section].name)
	end
end


start = s:option(Button, "start", translate("Start"))
start.inputstyle = "apply"
start.write = function(self, section)
	luci.sys.call("/etc/init.d/%s %s >/dev/null" %{ inits[section].name, self.option })
end

restart = s:option(Button, "restart", translate("Restart"))
restart.inputstyle = "reload"
restart.write = start.write

stop = s:option(Button, "stop", translate("Stop"))
stop.inputstyle = "remove"
stop.write = start.write

]]

function ltn12_popen(command)

	local fdi, fdo = nixio.pipe()
	local pid = nixio.fork()

	if pid > 0 then
		fdo:close()
		local close
		return function()
			local buffer = fdi:read(2048)
			local wpid, stat = nixio.waitpid(pid, "nohang")
			if not close and wpid and stat == "exited" then
				close = true
			end

			if buffer and #buffer > 0 then
				return buffer
			elseif close then
				fdi:close()
				return nil
			end
		end
	elseif pid == 0 then
		nixio.dup(fdo, nixio.stdout)
		fdi:close()
		fdo:close()
		nixio.exec("/bin/sh", "-c", command)
	end
end

local download, upload, upload_button
local _define_rc_upload_file = "cbid.rc.1.rc_upl"
local _define_rc_upload_path = "/lib/uci/upload/"

f = SimpleForm("rc", translate("Startup Script Management"), 
	translate("Insert your own commands to execute at the end of the boot process."))
--
-- manual edit
--	
t = f:field(TextValue, "rcs")
t.rmempty = true
t.rows = 20
t.maxlength="10000"

FileUpload.size = "204800"
FileUpload.sizetext = translate("File size is too large, max is 200 KiB")
FileUpload.sizetextempty = translate("Selected file is empty")
 
function t.cfgvalue()
	if real_local_file == "" then
		return luci.fs.readfile("/etc/rc.local") or ""
	end
	if real_local_file == "true" then
		if tmp_local_file == "true" then
			return luci.fs.readfile("/etc/rc.local") or ""
		else
			return luci.fs.readfile("/tmp/tmp-rc-local") or ""
		end
	end	
end	

-- 
-- upload script
--

upload = f:field(FileUpload, "rc_upl", translate("Upload script file"), translate("Upload your own scrip file from your personal computer"))

function FileUpload.cfgvalue(self, section)
	local val = _define_rc_upload_path .. _define_rc_upload_file
	if val and luci.fs.access(val) then
		return val
	end
	return nil
end

--
-- upload script button
--
upload_button = f:field(Button, "rc_upl_button", " ")
upload_button.inputtitle = translate("Upload")
upload_button.inputstyle = "apply"

--
-- download script
--

download = f:field(Button, "rc_downl", translate("Backup script file"), translate("Download current script file to personal computer"))
download.inputtitle = translate("Download")
download.inputstyle = "apply"

-- 
-- download/upload submit buttons
--
function f.handle(self, state, data)

	if state == FORM_VALID then
		local buffer
		if f:formvalue("cbi.rlf.1.rc_upl.x") then
			tmp_local_file = "true"
			real_local_file = "true"
		end
		if data.rc_upl_button then
			buffer = luci.fs.readfile(_define_rc_upload_path .. _define_rc_upload_file) or ""
			if buffer and buffer ~= "" then
				luci.fs.writefile("/tmp/tmp-rc-local", buffer:gsub("\r\n", "\n"))
				real_local_file = "true"
			end
		elseif data.rc_downl then
			local reader = ltn12_popen("cat /etc/rc.local")
			luci.http.header('Content-Disposition', 'attachment; filename="rc.local"')
			luci.http.prepare_content("text/cmd")
			luci.ltn12.pump.all(reader, luci.http.write)
		else
			local value3=f:formvalue("cbid.rc.1.rcs")
			--if real_local_file == "" then
				if value3~="" and value3~=nil then
					local value = value3:gsub("\r\n?", "\n")
					fs.writefile("/etc/rc.local", value)
				else
					luci.sys.call("echo '' >/etc/rc.local")
				end
			--end
		end
	end
return true
end

return f
