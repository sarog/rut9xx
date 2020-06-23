
module("luci.tools.profiles", package.seeall)

local uci = require "luci.model.uci".cursor()
local sys = require "luci.sys"
local fs = require "nixio.fs"

function update()
    local msg, code
    local path = uci:get("profiles", "general", "path") or "/etc/profiles/"
    local section = uci:get("profiles", "general", "profile")
    
    if path:sub(-1) ~= "/" then
        path = path .. "/"
    end

    if section and uci:get("profiles", section) == "profile" then
        local tar_file = uci:get("profiles", section, "archive")
        local md5_file = uci:get("profiles", section, "md5file")

        if tar_file and md5_file then
            local tar_file_path = path .. tar_file
            local md5_file_path = path .. md5_file

            if not fs.access(tar_file_path) then
                return 500, "Archive not found"
            else
                fs.unlink(tar_file_path)
                fs.unlink(md5_file_path)
                sys.exec("/sbin/sysupgrade -p %s" % tar_file_path)
                sys.exec("md5sum /etc/config/* /etc/shadow /etc/scheduler/config |" ..
                        "grep -v profiles | md5sum > %s" % md5_file_path)

                if not fs.access(tar_file_path) then
                    return 500, "There was an error updating profile."
                else
                    return 200, string.format("Profile '%s' updated successfully",
                        section)
                end
            end
        end
    else
        return 500, "Profile not found"
    end
end
