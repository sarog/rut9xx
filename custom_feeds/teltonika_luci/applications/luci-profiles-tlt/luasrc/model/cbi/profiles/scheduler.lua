local m, s, o

m = Map("profiles", translate("Scheduler configuration"))
m.redirect = luci.dispatcher.build_url("admin", "system", "profiles", "scheduler")

-- Required to check if profiles update spinner needs to be shown
function m.on_commit(self)
    local uci = require("luci.model.uci").cursor()
    
    if uci:get("profiles", "general", "scheduler") ~= "1" then
        return
    end
    local required_profile
    local hour = os.date("%H")
    local day = os.date("*t").wday    
    if day == 1 then
        day = 7
    else 
        day = day -1
    end

    local scheduler_array = {}
    uci:foreach("profiles", "scheduler", function(s)
        for i = 1,7,1
        do
            table.insert(scheduler_array, s['days'][i])
        end

        local required_profile_id = scheduler_array[day]:sub(hour+1, hour+1)

        uci:foreach("profiles", "profile", function(s)
            if s['id'] == required_profile_id then
                required_profile = s['.name']
                return
            end
        end)
    end)

    luci.dispatcher.context.required_profile = required_profile
    
end

s = m:section(NamedSection, "scheduler", "scheduler", translate("Profiles scheduler"))

o = s:option(Flag, "scheduler", translate("Enable"))
o.rmempty = false
function o.write(self, section, value)
    self.map:set("general", section, value)
    return
end

function o.cfgvalue(self, section)
    return self.map:get("general", "scheduler")
end



o = s:option(Value, "days")
o.template = "profiles/scheduler"
o.cast = "table"
o.cfglabels = function(self)
    local labels = {}

    self.map.uci:foreach(self.config, "profile", function(s)
        if s.id and s[".name"] ~= "default" then
            table.insert(labels, {id = s.id, name = s[".name"]})
        end
    end)

    table.sort(labels, function(a, b) return a.id < b.id end)

    return labels
end
o.formvalue = function(self, section)
    local value = AbstractValue.formvalue(self, section)

    if type(value) == "string" then
        if self.cast == "string" then
            local x
            local t = { }
            for x in value:gmatch("%S+") do
                t[#t+1] = x
            end
            value = t
        else
            value = { value }
        end
    end

    return value
end
o.write = function(self, section, value)
    local t = { }

    if type(value) == "table" then
        local x
        for _, x in ipairs(value) do
            if x and #x > 0 then
                t[#t+1] = x
            end
        end
    else
        t = { value }
    end

    if self.cast == "string" then
        value = table.concat(t, " ")
    else
        value = t
    end

    return AbstractValue.write(self, section, value)
end


return m

