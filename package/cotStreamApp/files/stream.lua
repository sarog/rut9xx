-- stream.lua
local utl = require "luci.util"
local sys = require "luci.sys"

local DEVICE_NAME = 'TLT-RUT9 Router'
local DEVICE_TYPE = 'Router'

function restart(r)
   local deviceId = r:value(2)
   c8y:send('108,'..deviceId..',SUCCESSFUL')
   sys.exec("reboot")
end

function init()
   srInfo('*** Stream Init ***')

   -- set device name and type
   c8y:send('103,'..c8y.ID..','..DEVICE_NAME..','..DEVICE_TYPE)

   -- set imei, cellid and iccid first time
   mobileDataStream()

   -- create timer which will stream mobile info data
   local m_timer = c8y:addTimer(10 * 1000, 'mobileDataStream')
   m_timer:start()   

   -- register restart handler
   c8y:addMsgHandler(502, 'restart')
   return 0
end

function mobileDataStream()
   local sign = utl.trim(sys.exec("gsmctl -q"))

   -- send mobile signal info
   c8y:send('105,'..c8y.ID..','..sign)

   local wantype = utl.trim(sys.exec("uci get -q network.wan.ifname"))
   local wanip = utl.trim(sys.exec("curl -s http://whatismyip.akamai.com"))
  
   -- send wan info
   c8y:send('106,'..c8y.ID..','..wanip..','..wantype)
end
