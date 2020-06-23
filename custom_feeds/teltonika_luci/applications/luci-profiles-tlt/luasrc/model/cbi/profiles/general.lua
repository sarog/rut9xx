local fs = require "nixio.fs"
local sys = require("luci.sys")
local eventlog = require'tlt_eventslog_lua'

local path
local m, s, o

m = Map("profiles", translate("Configuration Profiles"))
m.force_apply = true
path = m.uci:get("profiles", "general", "path") or "/etc/profiles/"

if path:sub(-1) ~= "/" then
    path = path .. "/"
end

s = m:section(TypedSection, "profile", translate("Profile setting"))
s.anonymous = false
s.addremove = true
s.template = "profiles/tblsection"
s.nosectionname = true
s.create = function(self, section)
    local res
    local ts = os.time()
    local tar_file = string.format("%s_%s.tar.gz", section, ts)
    local md5_file =  string.format("%s_%s.md5", section, ts)
    local tmpl_tar_file = path .. "template.tar.gz"
    local tmpl_md5_file = path .. "template.md5"
    local id = self:next_id()

    if id > 9 then
        self.map.message = translate("err: Only 9 profiles is available.")
        return false
    end

    if not section:match("^[%w_]+$") or #section < 1 or #section > 20 then
        self.map.message = translate("err: Invalid profile name. No special characters are allowed & max length is 20 characters.")
        return
    end

    if not fs.stat(path) then
        fs.mkdir(path)
    end

    if not fs.access(path .. tar_file) and fs.access(tmpl_tar_file) then
        fs.copy(tmpl_tar_file, path .. tar_file)
        fs.copy(tmpl_md5_file, path .. md5_file)
    else
        self.map.message =translate("err: There was an error creating new profile.")
        return false
    end

    self.defaults = {
        archive = tar_file,
        md5file = md5_file,
        id = id,
        updated = ts
    }

    return TypedSection.create(self, section)
end

s.remove = function(self, section)
    local current_profile = self.map:get("general", "profile")
    local tar_file = self.map:get(section, "archive")
    local md5_file = self.map:get(section, "md5file")

    if current_profile and current_profile == section then
        return false
    end

    if tar_file then fs.unlink(path .. tar_file) end
    if md5_file then fs.unlink(path .. md5_file) end
    self:clear_scheduler(section)
    TypedSection.remove(self, section)
end

s.clear_scheduler = function(self, section)
    local _, d, hrs
    local days = {}
    local id = self.map:get(section, "id")

    if id then
        self.map.uci:foreach(self.config, "scheduler", function(s)
            if s.days then
                for _, d in ipairs(s.days) do
                    hrs = string.gsub(d, tostring(id), "0")
                    table.insert(days, hrs)
                end

                self.map:set(s[".name"], "days", days)
            end
        end)
    end
end

s.cfgsections = function (self)
    local sections = {}
    self.map.uci:foreach(self.config, self.sectiontype, function(s)
        if self:checkscope(s[".name"]) and
                (s.archive and fs.access(path .. s.archive)) then
            table.insert(sections, s[".name"])
        end
    end)

    return sections
end

s.parse = function(self, novld)
    local _, section
    local crval = "cbi.apply." .. self.config
    local uval = "cbi.duplicate." .. self.config
    local name = next(self.map:formvaluetable(crval))
    local dname = next(self.map:formvaluetable(uval))

    if name then
        if self.cfgvalue(self, name) then
            local scheduler = self.map:get("general", "scheduler") or "0"

            if scheduler == "1" then
                self.map.message = translate("err: Unable to apply profile. Scheduler is enabled.")
            else
                self:apply_profile(name)
                self.map.apply_needed = true
                self.map.force_apply = true
            end
        end
    elseif dname then
        if self.cfgvalue(self, dname) then
            self:duplicate_profile(dname, true)
        end
    end

    TypedSection.parse(self, novld)
end

s.apply_profile = function(self, section)
    if section then
        local tar_file = self.map:get(section, "archive")
        local full_tar_file = path .. tar_file

        if tar_file and fs.access(full_tar_file) then
            local t = {requests = "insert", table = "events", sender = "Profile", priority = "notice",
                text = "Profile ".. section .." was applied " }
            eventlog:insert(t)

            luci.http.redirect(
                luci.dispatcher.build_url("admin/system/profiles/apply/%s" % section))

            return true
        end
    end

    return false
end

--TODO: for the future
s.duplicate_profile = function(self, section, verbose)
    --    if section then
    --        local tar_file = self.map:get(section, "archive")
    --
    --        if tar_file and md5_file then
    --            local ts = os.time()
    --            --FIXME: check name duplication
    --            local new_name = section .. "_1"
    --            local tar_file_path = path .. tar_file
    --            local new_tar_file_path = string.format("%s%s_%s.tar.gz", path, new_name, ts)
    --
    --            self:create(new_name)
    --            fs.copy(tar_file_path, new_tar_file_path)
    --
    --            return true
    --        end
    --    end
    --
    return false
end

s.next_id = function(self)
    local n, i
    local ids = {}

    for i= 1, 10 do ids[i] = false end

    self.map.uci:foreach(self.config, self.sectiontype, function(s)
        if s.id then
            ids[tonumber(s.id)] = true
        end
    end)

    for n, i in ipairs(ids) do
        if not i then
            return n
        end
    end

    return 10
end

o = s:option(DummyValue, "name", translate("Profile name"))
o.cfgvalue = function (_, section) return section end

o = s:option(DummyValue, "date", translate("Created") .. " / " .. translate("Updated"))
o.cfgvalue = function(self, section)
    local updated = self.map:get(section, "updated")
    return updated and os.date("%m/%d/%Y %H:%M", updated) or "-"
end

return m

