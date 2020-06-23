module("luci.controller.packages", package.seeall)

local fs = require "nixio.fs"

function index()
    entry({"admin", "system", "packages"}, alias("admin", "system", "packages", "packages"), _("Package Manager"), 8)
    entry({"admin", "system", "packages", "packages"}, template("packages/packages"), _("Packages"), 1).leaf = true
    entry({"admin", "system", "packages", "upload"}, call("upload_package"), _("Upload"), 2).leaf = true
    entry({"admin", "system", "packages", "check"}, call("update_packages"), nil, nil)
    entry({"admin", "system", "packages", "checkav"}, call("update_packages_available"), nil, nil)
    entry({"admin", "system", "packages", "install"}, call("opkg_install"), nil, nil)
    entry({"admin", "system", "packages", "remove"}, call("opkg_remove"), nil, nil)
    entry({"admin", "system", "packages", "remove_pending"}, call("remove_pending"), nil, nil)
end

local function _opkg(cmd, name, ...)
    local opkg = "opkg --force-removal-of-dependent-packages --force-overwrite --force-maintainer --force-depends --nocase"
    local pkg = ""
    local action
    for k, v in pairs({...}) do
        pkg = pkg .. " '" .. v:gsub("'", "") .. "'"
    end

    if cmd == "upload" then
        action = "install"
    else
        action = cmd
    end

    local c = "%s %s %s >/tmp/opkg.stdout 2>/tmp/opkg.stderr" %{ opkg, action, pkg }
    local r = os.execute(c)
    local e = fs.readfile("/tmp/opkg.stderr")
    local o = fs.readfile("/tmp/opkg.stdout")

    fs.unlink("/tmp/opkg.stderr")
    fs.unlink("/tmp/opkg.stdout")

    local req

    if tonumber(r) == 0 then
        req = {error = "0"}
        if action == "install" then
            local f = io.open("/usr/lib/opkg/info/"..name..".list", "r")
            local files = f:read("*all")
            f:close()
            for file in string.gmatch(files, "([%w%.%-%+_+/*:]+)\n") do
                if file:find("/etc/init.d/") then
                    luci.util.exec(file.." enable")
                end
            end
        end
    else
        req = {error = "1"}
    end

    fs.remove(...)
    luci.util.exec("/sbin/reload_config")
    luci.util.exec("rm -fr /tmp/luci-indexcache/ 2> /dev/null")

    if cmd ~= "upload" then
        luci.http.prepare_content("application/json")
        luci.http.write_json(req)
    else
        return tonumber(r) == 0 and 0 or 1
    end
end

function opkg_install()
    local package_link = luci.http.formvalue("link")
    local package_name = luci.http.formvalue("package")

    local result = luci.util.exec("/usr/bin/curl -y 30 -o /tmp/tlt_custom_pkg.ipk "..package_link.." 2>/dev/null; echo $?")
    if tonumber(result) == 0 then
        _opkg("install", package_name, "/tmp/tlt_custom_pkg.ipk")
    else
        fs.remove("/tmp/tlt_custom_pkg.ipk")
        luci.http.prepare_content("application/json")
        luci.http.write_json({error = "1"})
    end
end

function opkg_remove()
    local package_name = luci.http.formvalue("package")
    local f = io.open("/usr/lib/opkg/info/"..package_name..".list", "r")
    local files = f:read("*all")
    f:close()
    for file in string.gmatch(files, "([%w%.%-%+_+/*:]+)\n") do
        if file:find("/etc/init.d/") then
            luci.util.exec(file.." stop")
            luci.util.exec(file.." disable")
        elseif file:find("/etc/config/") then
            fs.remove(file)
        end
    end
    _opkg("remove", nil, package_name)
end

function remove_pending(pkg)
    local package_name
    if pkg then
        package_name = pkg
    else
        package_name = luci.http.formvalue("package")
    end
    luci.util.exec("sed -i /"..package_name.."/d /etc/package_restore.txt 2>/dev/null")
    luci.util.exec("sed -i /"..package_name.."/d /etc/failed_packages 2>/dev/null")
end

function round(number, decimal)
    local multiplier = 10^(decimal or 0)
    return math.floor(number * multiplier + 0.5) / multiplier
end

function update_packages_available()
    local package, list, pending, is_pending, enough_space, is_installed
    local packages = {}
    local error = 0
    local uci = require "luci.model.uci".cursor()

    fs.remove("/tmp/available_packages")
    luci.util.exec("/sbin/rut_fota -p")

    local f = io.open("/tmp/available_packages", "r")
    pending = update_packages_pending()

    if f then
        list = f:read("*all")
        f:close()

        local free_space_string = luci.util.exec("df | grep rootfs")
        local free_space_kb = tonumber(string.match(free_space_string, "rootfs%s*%d*%s*%d*%s*(%d*).*"))

        for pkg_name, pkg_version, pkg_size, pkg_link, pkg_description, pkg_tlt_name, pkg_source_version in
        string.gmatch(list, "([%w%.%-%+_%s]+)%s+%-%s+([%w%.%-%+_]+)%s+%-%s+(%d+)%s+%-%s+([%w%.%-%+_+/*:]+)%s+%-%s+(.-)%s+%-%s+([%w%.%-%+_%s]+)%s+%-%s+([%w%.%-%+_]+)%c-\n") do
            is_installed = 0
            if fs.access("/usr/lib/opkg/info/"..pkg_tlt_name..".control") then
                is_installed = 1
            end
            --local size_kb = round(tonumber(size) / 1024, 2)
            enough_space = 1
            if tonumber(pkg_size) >= free_space_kb then
                enough_space = 0
            end

            is_pending = 0
            for a,b in pairs(pending) do
                if b.name == pkg_tlt_name then
                    is_pending = 1
                end
            end

            if is_pending == 0 then
                table.insert(packages, {name = pkg_name, version = pkg_version, size = pkg_size, link = pkg_link, description = pkg_description, space = enough_space, tlt_name = pkg_tlt_name, is_installed = is_installed, source_version = pkg_source_version})
            end
        end
    else
        local err = uci:get("rut_fota", "config", "connection_state") or "connected"
        if err == "failed" then
            error = 1
        else
            error = 2
        end
    end

    return packages, error
end

function update_packages()
    local list_installed = luci.util.exec("cat /usr/lib/opkg/status | grep tlt_custom_pkg")
    local files = {}
    local pkg_size, files, pkg, f, size, info, desc, ver, src_ver

    local packages = {}

    for value in string.gmatch(list_installed, "Package%:%s+([%w%.%-%+_]+)") do
        pkg_size = 0
        f = io.open("/usr/lib/opkg/info/"..value..".list", "r")
        files = f:read("*all")
        f:close()
        for file in string.gmatch(files, "([%w%.%-%+_+/*:]+)\n") do
            if fs.realpath(file) == file then
                size = fs.lstat(file, "size")
                size = size and tonumber(size) or nil
                if size then
                    pkg_size = pkg_size + size
                end
            end
        end
        f = io.open("/usr/lib/opkg/info/"..value..".control", "r")
        info = f:read("*all")
        f:close()
        for value2 in string.gmatch(info, "Description%:%s+(.*)\n") do
            desc = value2
        end
        for value3 in string.gmatch(info, "tlt_name%:%s+(.-)\n") do
            pkg = value3
        end
        for value4 in string.gmatch(info, "tlt_version%:%s+(.-)\n") do
            ver = value4
        end
        for value5 in string.gmatch(info, "Version%:%s+(.-)\n") do
            src_ver = value5
        end
        table.insert(packages, {name = pkg, version = ver, pkg_name = value, description = desc, size = round(pkg_size/1024, 0), source_version = src_ver})
    end

    return packages
end

function update_packages_pending()
    local list
    local f = io.open("/etc/package_restore.txt", "r")
    local a = io.open("/etc/failed_packages", "r")

    local packages = {}

    if f then
        list = f:read("*all")
        f:close()

        for pkg_name, tlt_name in string.gmatch(list, "([%w%.%-%+_]+) *%-* *([%w%.%-%s%+_]-)\n") do
            table.insert(packages, {name = pkg_name, status = "In queue to download", tlt_name = tlt_name:len() > 0 and tlt_name or pkg_name})
        end
    end

    if a then
        list = a:read("*all")
        a:close()

        for pkg_name, tlt_name in string.gmatch(list, "([%w%.%-%+_]+) *%-* *([%w%.%-%s%+_]-)\n") do
            table.insert(packages, {name = pkg_name, status = "Installation failed", tlt_name = tlt_name:len() > 0 and tlt_name or pkg_name})
        end
    end

    return packages
end

function check_package_fw(fw)
    local nr1, nr2, nr3

    local firmware = fs.readfile("/etc/version")
    local _, count = firmware:gsub("%.", "")
    if count == 2 then
        firmware = firmware:gsub("\n", "")..".0"
    end
    for a, b, c, d in string.gmatch(firmware, "([%w%.%-%+_]+)%p(%d+)%p(%d+)%p(%d+)") do
        if d:len() == 1 then
            d = d.."00"
        elseif  a:len() == 2 then
            d = d.."0"
        end
        if b:len() == 1 then
            b = b.."00"
        elseif  b:len() == 2 then
            b = b.."0"
        end
        if c:len() == 1 then
            c = c.."00"
        elseif  c:len() == 2 then
            c = c.."0"
        end
        nr1 = tonumber(b)
        nr2 = tonumber(c)
        nr3 = tonumber(d)
    end

    for a, b, c in string.gmatch(fw, "(%d+)%p(%d+)%p(%d+)") do
        if a:len() == 1 then
            a = a.."00"
        elseif  a:len() == 2 then
            a = a.."0"
        end
        if b:len() == 1 then
            b = b.."00"
        elseif  b:len() == 2 then
            b = b.."0"
        end
        if c:len() == 1 then
            c = c.."00"
        elseif  c:len() == 2 then
            c = c.."0"
        end

        if nr1 > tonumber(a) then
            return 1
        elseif nr1 == tonumber(a) and nr2 > tonumber(b) then
            return 1
        elseif nr1 == tonumber(a) and nr2 == tonumber(b) and nr3 >= tonumber(c) then
            return 1
        end
    end
    return 0
end

function check_package()
    local f, ff, list, info, version, pkg_name
    local name_validation = 0
    local fw_validation = 0
    local invalid_file = 0
    local installed_version = 1
    local router_name = luci.util.exec("mnf_info name")

    luci.util.exec("tar -xzC/tmp/custom_package -f /tmp/custom_package/control.tar.gz")

    f = io.open("/tmp/custom_package/control", "r")

    if f then
        info = f:read("*all")
        f:close()
        for value in string.gmatch(info, "tlt_version%:%s+(.-)\n") do
            version = value
        end

        if not version then
            return invalid_file, name_validation, fw_validation, installed_version, pkg_name
        end

        for name, value in string.gmatch(info, "([%w%.%-%+_]+)%:%s+(.-)\n") do
            if name == "Router" and router_name:find(value) then
                name_validation = 1
            elseif name == "Firmware" then
                fw_validation = check_package_fw(value)
            elseif name == "Package" then
                pkg_name = value
                f = io.open("/usr/lib/opkg/info/"..value..".control", "r")
                if f then
                    list = f:read("*all")
                    f:close()
                    for value in string.gmatch(list, "tlt_version%:%s+([%w%.%-%+_]+)") do
                        if value >= version then
                            installed_version = 0
                        end
                    end
                end
            end
        end
    else
        invalid_file = 1
    end

    return invalid_file, name_validation, fw_validation, installed_version, pkg_name
end

function upload_package()
    local cmd = "tar -xzC/tmp/custom_package >/dev/null 2>&1"
    local fp, ff, message, pkg_name
    local name_validation = 0
    local fw_validation = 0
    local installed_version = 0
    local invalid_file = 0
    local package_ok = 0
    local package_link = "/tmp/custom_package/package.ipk"

    luci.util.exec("rm -r /tmp/custom_package")
    fs.mkdir("/tmp/custom_package")

    luci.http.setfilehandler(
        function(meta, chunk, eof)
            if not fp then
                fp = io.popen(cmd, "w")
            end
            if not ff then
                ff = io.open(package_link, "w")
            end
            if chunk then
                fp:write(chunk)
                ff:write(chunk)
            end
            if eof then
                fp:close()
                ff:close()
                invalid_file, name_validation, fw_validation, installed_version, pkg_name = check_package()
            end
        end
    )

    if luci.http.formvalue("restore") then
        local upload = luci.http.formvalue("archive")
        if upload and #upload > 0 then
            if name_validation == 1 and fw_validation == 1 and installed_version == 1 then
                package_ok = _opkg("upload", pkg_name, package_link)
                if package_ok == 0 then
                    message = "Package successfully installed"
                    if pkg_name then
                        remove_pending(pkg_name)
                    end
                else
                    message = "Package installation failed"
                end
            else
                package_ok = 1
                if invalid_file == 1 then
                    message = "Invalid file"
                elseif installed_version == 0 then
                    message = "The package is already installed with the same or higher version"
                elseif fw_validation == 0 then
                    message = "The package is not compatible with this router firmware"
                elseif name_validation == 0 then
                    message = "The package is not compatible with this router"
                end
            end

            luci.util.exec("rm -r /tmp/custom_package")

            luci.template.render("packages/upload", {
                error_code = package_ok,
                message_text = message
            })
        end
    else
        luci.template.render("packages/upload", {
        })
    end
end
