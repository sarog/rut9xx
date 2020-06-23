local m, m2, s, o

--
-- Password
--
eventlog = require'tlt_eventslog_lua'

m = Map("system", translate("Step 1"),
	translate("First, let's change your routerd password from the default one."))

s = m:section(TypedSection, "_dummy", "")
s.addremove = false
s.anonymous = true

pw1 = s:option(Value, "pw1", translate("Password"))
pw1.password = true

pw2 = s:option(Value, "pw2", translate("Confirmation"))
pw2.password = true

function s.cfgsections()
	return { "_pass" }
end

function m.on_commit(map)
	local v1 = pw1:formvalue("_pass")
	local v2 = pw2:formvalue("_pass")

	if v1 and v2 and #v1 > 0 and #v2 > 0 then
		if v1 == v2 then
			if luci.sys.user.setpasswd(luci.dispatcher.context.authuser, v1) == 0 then
				t = {requests = "insert", table = "EVENTS", type="Web UI", text="Administrator password was changed!"}
				eventlog:insert(t)
				m.message = translate("Password successfully changed!")
			else
				m.message = translate("Unknown Error, password not changed!")
			end
		else
			m.message = translate("Given password confirmation did not match, password not changed!")
		end
	end
end

--
-- 3g
--

m2 = Map("network_3g", translate("Step 2"), 
		translate("Next, let's configure your Mobile settings so you can start using internet right away."))

s = m2:section(TypedSection, "service3g", "");

o = s:option(Flag, "enabled", translate("Enabled"))

o.enabled  = "1"
o.disabled = "0"
o.default  = o.enabled

o = s:option(ListValue, "auth", translate("Mobile authentication method"))

o.default = "CHAP"
o:value("CHAP", translate("CHAP"))
o:value("PAP", translate("PAP"))


o = s:option(Value, "APN", translate("APN"))


o = s:option(Value, "Username", translate("Username"))

o = s:option(Value, "Password", translate("Password"))
o.password = true;

o = s:option(Value, "PIN", translate("PIN number"))



return m, m2
