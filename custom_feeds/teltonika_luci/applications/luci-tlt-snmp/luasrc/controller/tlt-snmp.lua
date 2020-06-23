module("luci.controller.tlt-snmp", package.seeall)

function index()
	entry({"admin", "services", "snmp"}, alias("admin", "services", "snmp", "snmp-settings"), _("SNMP"), 85)
	entry({"admin", "services", "snmp", "snmp-settings" }, cbi("tlt-snmp/tlt-snmp"), _("SNMP Settings"), 1).leaf = true
	entry({"admin", "services", "snmp", "trap-settings"},arcombine(cbi("tlt-snmp/tlt-trap"), cbi("tlt-snmp/tlt-trap-details")),_("Trap Settings"), 2).leaf = true
	entry({"admin", "services", "snmp", "mib_download" }, call("download_mib"))
end

function download_mib(val)
	local mib_file   = "/usr/opt/snmp/mibs/TLT-MIB.txt"
	local fp

	luci.http.setfilehandler(
		function(meta, chunk, eof)
			if not fp then
				fp = io.open(mib_file, "w")
			end

			if chunk then
				fp:write(chunk)
			end

			if eof then
				fp:close()
			end
		end
	)

	 local reader = ltn12_popen("cat " .. mib_file)
		luci.http.header('Content-Disposition', 'attachment; filename="TLT-MIB.txt"')
		luci.http.prepare_content("text/html")
		luci.ltn12.pump.all(reader, luci.http.write)
end

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
