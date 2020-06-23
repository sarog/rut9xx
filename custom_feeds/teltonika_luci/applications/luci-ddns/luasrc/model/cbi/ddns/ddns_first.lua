local uci = require "luci.model.uci".cursor()
local sys = require("luci.sys")

m = Map("ddns", translate("DDNS"))
m.spec_dir = nil
--m.pageaction = false

s = m:section( TypedSection, "service", translate("DDNS Configuration"), translate(""))

s.addremove = true
s.anonymous = true
s.template = "cbi/tblsection"
s.template_addremove = "ddns/ddns_add_rem"
s.addremoveAdd = true
s.novaluetext = translate("No DDNS records found.")

s.extedit = luci.dispatcher.build_url("admin", "services", "ddns_edit", "%s")

local name = s:option(DummyValue, "interface", translate("DDNS name"), translate("Name of DDNS (Dynamic Domain Name System) configuration. Used for easier DDNS configurations management purpose only"))

function name.cfgvalue(self, section)
	return section:gsub("^%l", string.upper) or "Unknown"
end

hName=s:option(DummyValue, "domain", translate("Hostname"), translate("Domain name which will be linked with dynamic IP address"))

state = s:option(DummyValue, "state", translate("Status"), translate("Timestamp of the last IP check or update"))

function state.cfgvalue(self, section)
	--local val = AbstractValue.cfgvalue(self, section)	
	local val = sys.exec("cat /tmp/ddns_status_" .. section)
	if val and val ~= "\n" and val ~= "" then
		return val
	else
		return "N/A"
	end
end

enb = s:option(Flag, "enabled", translate("Enable"), translate("Make a rule active/inactive"))

--[[
enb=s:option(DummyValue, "enabled", translate("Enabled"), translate("Indicates whether a configuration is active or not"))
function enb.cfgvalue(self, section)
	local val = AbstractValue.cfgvalue(self, section)	
	if val == "1" then
		return "Yes"
	else
		return "No"
	end
end
--]]

function s.parse(self, section)
	-- 	mix.echo("OpenVPN sekcijos parsinimas")
	local cfgname = luci.http.formvalue("cbid." .. self.config .. "." .. self.sectiontype .. ".name")

	-- 'Delete' button does not commit uci changes. So we will do it manually. And here another problem 
	-- occurs: 'Delete' button has very long name including vpn instance name and I don't know that 
	-- instance name. So I will scan through uci config and try to find out if such instance name exists
	-- as form element. FIXME investigate if another more inteligent approach is available here (O_o)
	local delButtonFormString = "cbi.rts." .. self.config .. "."
	local delButtonPress
	local configName
	local uFound

	uci:foreach("ddns", "service", function(x)
		configName = x[".name"] or ""
		if luci.http.formvalue(delButtonFormString .. configName) then
			delButtonPress = true
			sys.exec("rm -f /tmp/ddns_status_" .. configName)--Delete status file
		end
	end)

	if delButtonPress then
		--luci.sys.call("/etc/init.d/openvpn restart >/dev/null")
		-- delete buttons is pressed, don't execute function 'openvpn_new'
		cfgname = false
	end

	if cfgname and cfgname ~= '' then
		uci:foreach("ddns", "service", function(x)
			if x[".name"] == cfgname then
				uFound = 1
			end
		end)
		if uFound ~= 1 then
			ddns_new(self, cfgname)
		end
	end
	TypedSection.parse( self, section )
	if uFound == 1 then
		m.message = translate("err: Cannot create new instance with the same name.")
		return nil
	end
end

function ddns_new(self,name)
	local t = {}

	if name and #name > 0 then
		if not (string.find(name, "[%c?%p?%s?]+") == nil) then
			m.message = translate("err: Only alphanumeric characters are allowed.")
		else
			t["service_name"] = "dyndns.org"
			t["domain"] = "mypersonaldomain.dyndns.org"
			t["username"] = "myusername"
			t["password"] = "mypassword"

			uci:section("ddns", "service", name,t)
			uci:save("ddns")
			m.message = translate("scs: New DDNS instance created successfully. Configure it now")
		end
	else
		m.message = translate("To create a new DDNS instance it's name has to be entered!")
	end
end

local save = m:formvalue("cbi.apply")
if save then
	--Delete all usr_enable from ddns config
	m.uci:foreach("ddns", "service", function(s)
		ddns_inst = s[".name"] or ""
		ddnsEnable = m:formvalue("cbid.ddns." .. ddns_inst .. ".enabled") or "0"
		ddns_enable = s.enabled or "0"
		if ddnsEnable ~= ddns_enable then
			m.uci:foreach("ddns", "service", function(a)
				ddns_inst2 = a[".name"] or ""
				local usr_enable = a.usr_enable or ""
				if usr_enable == "1" then
					m.uci:delete("ddns", ddns_inst2, "usr_enable")
				end
			end)
		end
	end)
	m.uci:save("ddns")
	m.uci.commit("ddns")
end

function m.on_commit()
	--set dnsmasq rebind protection for private IPs 
	--checking if atleast one enabled ddns needs to work with private IPs
	local private_ip_possible = "0"
	local usr_enable = "0"

	m.uci:foreach("ddns", "service", function(s)
		usr_enable = s.enabled or "0"
		if usr_enable == "1" then
			if s.ip_source == "network" or s.ip_source == "interface" or s.ip_source == "script" then
				private_ip_possible = "1"
			end
		end
	end)

	if private_ip_possible == "1" then
		sys.exec("uci -q set dhcp.@dnsmasq[0].rebind_protection=0")
	else
		sys.exec("uci -q set dhcp.@dnsmasq[0].rebind_protection=1")
	end

	sys.exec("uci -q commit dhcp")
end

return m 
