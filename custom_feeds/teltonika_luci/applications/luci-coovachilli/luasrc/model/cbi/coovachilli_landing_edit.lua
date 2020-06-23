require "luci.fs"
require "luci.sys"
require "luci.util"

local inits = { }

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

m = SimpleForm("rc", translate("Landing Page Template Editor"), translate("Modify login page template by your needs"))
m.template = "cbi/simpleformtmpl"
m.reset = false
m.reset1 = true
m.submit = false
m.submit1 = true

t = m:field(TextValue, "rcs")
t.rmempty = true
t.rows = 20

function t.cfgvalue()
	if luci.sys.exec("uci get -q landingpage.general.loginPage") ~= "" or nil then
		return luci.fs.readfile("/lib/uci/upload/cbid.landingpage.general.loginPage")
	else
		return luci.fs.readfile("/etc/chilli/www/hotspotlogin.tmpl")
	end
end

function m.handle(self, state, data)
	local file = luci.sys.exec("uci get -q landingpage.general.loginPage")
	if state == FORM_VALID then
		if data.rcs then
			if file ~= "" or nil then
				luci.fs.writefile("/lib/uci/upload/cbid.landingpage.general.loginPage", data.rcs:gsub("\r\n", "\n"))
			elseif file == "" or nil then
				luci.fs.writefile("/etc/chilli/www/hotspotlogin.tmpl", data.rcs:gsub("\r\n", "\n"))
			end
		end
	end
	return true
end

return m
