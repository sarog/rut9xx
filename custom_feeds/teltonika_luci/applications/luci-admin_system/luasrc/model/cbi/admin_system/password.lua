--~ Made by Teltonika 2018
local fs = require "luci.fs"
local types = require "luci.cbi.datatypes"

m = Map("system", translate("Change password"),
	translate("You must change password to leave this page! Password requirements: Minimum 8 characters, at least one uppercase letter, one lowercase letter and one number."))

s = m:section(TypedSection, "_dummy", translate("Administrator Password"))
s.addremove = false
s.anonymous = true

pw1 = s:option(Value, "pw1", translate("New password"), translate("Enter your new administration password."))
pw1.password = true
pw1.datatype = "root_password_validate(8,32)"

pw2 = s:option(Value, "pw2", translate("Confirm new password"), translate("Re-enter your new administration password"))
pw2.password = true
pw2.datatype = "root_password_validate(8,32)"

function s.cfgsections()
	return { "_pass" }
end

function m.parse(map)
	local v1 = pw1:formvalue("_pass")
	local v2 = pw2:formvalue("_pass")
	local ledsman = luci.http.formvalue("cbid.system.cfg0a036d.enable")
	if ledsman == nil then
		ledsman = "0"
	end

	if v1 and v2 then
		if #v1 > 0 and #v2 > 0 then
			if v1 == v2 then
				if types.root_password(v1) then
					if luci.sys.user.setpasswd(luci.dispatcher.context.authuser, v1) == 0 then
						m.message = translate("scs: Password successfully changed!")
						m.uci:set("teltonika", "sys", "pass_changed", "1")
						m.uci:save("teltonika")
						m.uci:commit("teltonika")
						if fs.access("/tmp/first_boot") then
							luci.http.redirect(luci.dispatcher.build_url("admin/system/wizard/step-pwd"))
						else
							luci.http.redirect(luci.dispatcher.build_url("admin/status/overview"))
						end
					else
						map.message = translate("err: Unknown error, password not changed!")
						return
					end
				else
					m.message = translate("err: Password too weak, password not changed!")
					return
				end
			else
				map.message = translate("err: Given password confirmation did not match, password not changed!")
				return
			end
		else
			map.message = translate("err: Password cannot be empty, password not changed!")
			return
		end
	else
		return
	end

	Map.parse(map)
end

return m
