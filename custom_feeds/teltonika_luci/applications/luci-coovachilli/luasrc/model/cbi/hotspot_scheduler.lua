require("uci")

local x = uci.cursor()
local trap = false
local tab_SSID
local pathinfo = os.getenv("PATH_INFO")
local the_paths = {}

for node in pathinfo:gmatch("[^/]+") do
	table.insert(the_paths, node)
end

local ID = the_paths[#the_paths]

--Get ssid or even ID if it is not set
x:foreach("wireless", "wifi-iface", function(s)
	if not tab_SSID then
		if ID ~= "hotspot_scheduler" then
			if s.hotspotid and ID == s.hotspotid then
				tab_SSID = s.ssid or ""
			end
		else
			tab_SSID = s.ssid or ""
			ID = s.hotspotid or ""
		end
	end
end)

m = Map( "hotspot_scheduler", translate( "Internet Access Restriction Settings" ), translate( "" ) )
s = m:section( NamedSection, ID, "ap", translate("Select Time To Restrict Access On Hotspot " .. tab_SSID), translate("" ))

tbl = s:option( Value, "days")
tbl.template="chilli/hotspot_scheduler"

function tbl.write(self, section, value)
	if not trap then
		local days={"mon", "tue", "wed", "thu", "fri", "sat", "sun" }
		local hours

		for num, name in ipairs(days)  do
			hours = string.sub(value, num*24-23, num*24)

			x:set("hotspot_scheduler", ID, name, hours)
			x:commit("hotspot_scheduler")
		end

		trap = true
	end
end

return m
