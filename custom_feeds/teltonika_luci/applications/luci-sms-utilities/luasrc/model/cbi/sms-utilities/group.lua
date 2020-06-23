
local ds = require "luci.dispatcher"
local uci = require "luci.model.uci".cursor()

m = Map("sms_utils", translate("User Groups Configuration"),translate(""))

s = m:section(TypedSection, "group", translate("Whitelisted User Groups For SMS/Call Management"))
s.template  = "cbi/tblsection"
s.addremove = true
s.anonymous = true
s.extedit   = ds.build_url("admin/services/sms/group/%s")
s.template_addremove = "sms-utilities/add_group"
s.novaluetext = translate("There are no groups created yet")

function s.create(self, section)
	local t = m:formvalue("_newinput.name") or ""
        exist = 0
	uci:foreach("sms_utils", "group", function(x)
		if x.name == t then
                    m.message = translatef("err: Name %s already exists.", t)
                    exist = 1
		end
	end)
        if exist ~= 1 then
            created = TypedSection.create(self, section)
            self.map:set(created, "name",   t)
        end
end

function s.parse(self, section)
	TypedSection.parse(self, section)
	if exist == 0 and created then
		m.uci:save("sms_utils")
		luci.http.redirect(ds.build_url("admin/services/sms/group", created ))
	end
end

src = s:option(DummyValue, "name", translate("Group name"), translate("Name of grouped phone numbers"))
src.rawhtml = true
src.width   = "15%"

src = s:option(DummyValue, "tel", translate("Phone number"), translate("Phone number belonging to a group"))
src.rawhtml = true
src.width   = "60%"

function src.cfgvalue(self, section)
	local value = luci.sys.exec("uci get -q sms_utils."..section..".tel")
	value = string.gsub(value, " ", ", ")
	return value
end

return m
