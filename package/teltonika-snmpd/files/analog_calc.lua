#!/usr/bin/env lua

require("uci")

function exec(command)
	local pp   = io.popen(command)
	local data = pp:read("*a")
	pp:close()

	return data
end

local uci = uci.cursor()
local antype = "noanalogrule"
local anformultiply = uci:get("ioman","iolabels","anformultiply") or "1"
local anforoffset = uci:get("ioman","iolabels","anforoffset") or "0"
local anforadd = uci:get("ioman","iolabels","anforadd") or "0"
local anfordivide = uci:get("ioman","iolabels","anfordivide") or "1"
local analogunit = uci:get("ioman","iolabels","anformeasunit") or "V"
local ganalog = exec("cat /sys/class/hwmon/hwmon0/device/in0_input")
local gresistor = exec("uci get ioman.ioman.resistor")

uci:foreach("ioman", "rule", function(s)
  if s.type == "analog" then
    antype = s.analogtype
  end
end)

if ganalog >= "0" then
  if antype == "currenttype" then
    analogunit = " mA"
    analog = string.format("%.2f", (ganalog*(131000+gresistor))/(131000*gresistor)) .. analogunit
  else
    if ((anformultiply == "1")and(anforoffset == "0")and(anforadd == "0")and(anfordivide == "1")) then
    else
      analogunit = analogunit .. " (recalculated value)"
    end

    analog = string.format("%.2f", anformultiply * ( (ganalog/1000) + anforadd)/(anfordivide) ) + anforoffset .. analogunit
  end
else
  analog = "N/A"
end

print(analog)
