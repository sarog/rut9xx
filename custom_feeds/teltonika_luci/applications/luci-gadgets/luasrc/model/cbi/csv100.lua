--[[
LuCI - Lua Configuration Interface

Copyright 2016 @ Teltonika

]]--	

local sys = require "luci.sys"

m = Map("cam_monitoring", translate("CSV100"), translate(""))
m.addremove = false

s = m:section(TypedSection, "cam_monitoring", translate("CSV100"))
s.addremove = false

cam1_conf = m.uci:get("cam_monitoring", "cam_monitoring", "cam1")
cam2_conf = m.uci:get("cam_monitoring", "cam_monitoring", "cam2")
cam3_conf = m.uci:get("cam_monitoring", "cam_monitoring", "cam3")
cam4_conf = m.uci:get("cam_monitoring", "cam_monitoring", "cam4")

cam1 = s:option(ListValue, "cam1", "Camera 1", "Camera connected to the first swtch port")
cam1:value("OFF", translate("Off"))
cam1:value("ON", translate("On"))
    
cam2 = s:option(ListValue, "cam2", translate("Camera 2"), translate("Camera connected to the second swtch port"))
cam2:value("OFF", translate("Off"))
cam2:value("ON", translate("On"))
    
cam3 = s:option(ListValue, "cam3",translate("Camera 3"), translate("Camera connected to the third swtch port"))
cam3:value("OFF", translate("Off"))
cam3:value("ON", translate("On"))
    
cam4 = s:option(ListValue, "cam4", translate("Camera 4"), translate("Camera connected to the fourth swtch port"))
cam4:value("OFF", translate("Off"))
cam4:value("ON", translate("On"))

function m.on_commit()
    local cam1 = m:formvalue("cbid.cam_monitoring.cam_monitoring.cam1")
    local cam2 = m:formvalue("cbid.cam_monitoring.cam_monitoring.cam2")
    local cam3 = m:formvalue("cbid.cam_monitoring.cam_monitoring.cam3")
    local cam4 = m:formvalue("cbid.cam_monitoring.cam_monitoring.cam4")

    if cam1 ~= cam1_conf then
        luci.sys.exec("/sbin/cam_monitoring.sh CAM1 "..cam1.. " &")
    end
    
    if cam2 ~= cam2_conf then
        luci.sys.exec("/sbin/cam_monitoring.sh CAM2 "..cam2.. " &")
    end
    
    if cam3 ~= cam3_conf then
        luci.sys.exec("/sbin/cam_monitoring.sh CAM3 "..cam3.. " &")
    end
    
    if cam4 ~= cam4_conf then
        luci.sys.exec("/sbin/cam_monitoring.sh CAM4 "..cam4.. " &")
    end
end

return m	
