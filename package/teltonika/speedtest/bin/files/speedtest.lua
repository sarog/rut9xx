#!/usr/bin/lua

local libspeedtest = require ("libspeedtest")
local socket =  require("socket")
local jsc = require("luci.jsonc")

local SERVER, WARNING, ERROR, isError, res
local STATE = "START"
local TIME = 10
local SILENT = false

local COUNT = 0
local DBYTES = 0
local UBYTES = 0

--Writes to JSON file all of the information
local function writeToJSON(downloadSpeed, currentDownloaded, uploadSpeed, currentUpload)
    local file = io.open("/tmp/speedtest.json", "w")
    file:write("{")

    if SERVER ~= nil then
        file:write("\"serverURL\" : \""..SERVER.."\",")
    end
    if tonumber(downloadSpeed) then
        file:write("\"avgDownloadSpeed\" : "..downloadSpeed..",")
    end
    if tonumber(currentDownloaded) then
        file:write("\"downloaded\" : "..currentDownloaded..",")
    end
    if tonumber(uploadSpeed) then
        file:write("\"avgUploadSpeed\" : "..uploadSpeed..",")
    end
    if tonumber(currentUpload) then
        file:write("\"uploaded\" : "..currentUpload..",")
    end
    if ERROR ~= nil then
        STATE = "ERROR"
        file:write("\"error\" : \""..ERROR.."\",")
    end
    if WARNING ~= nil then
        file:write("\"warning\" : \""..WARNING.."\",")
    end
    if STATE ~= nil then
        file:write("\"state\" : \""..STATE.."\"")
    end

    file:write("}")
    file:close()
end

--Trims the link by removing https:// or http:// and removing everything from the link that goes from / character.
local function trimLink(link)
    local cutlink, i, j

    cutlink = link

    if string.match(cutlink, "//") then
        i, j = string.find(cutlink, "//")
        i = string.find(cutlink, "\n")
        cutlink = string.sub(cutlink, j + 1, i)
    end
    if string.match(cutlink, ":") then
        i = string.find(cutlink, ":")
        cutlink = string.sub(cutlink, 0, i - 1)
    end

    return cutlink
end

-- Calculates the distances between 2 positions.
local function calculateDistance(lat1, lon1, lat2, lon2)
    local R = 6371e3
    local fi1 = math.rad(lat1)
    local fi2 = math.rad(lat2)
    local deltaLam = math.rad((lon2 - lon1))
    local d = math.acos(math.sin(fi1) * math.sin(fi2) + math.cos(fi1) * math.cos(fi2) * math.cos(deltaLam)) * R
    return d
end

-- Changes bytes to other numbers.
local function convertBytes(bytes, mbps)
    if mbps then
        if bytes > 1000000000 then 
            return string.format("%.3f", (bytes/125000000)).."gbps"
        elseif bytes > 1000000 then
            return string.format("%.3f", (bytes/125000)).."mbps"
        elseif bytes > 1000 then
            return string.format("%.3f", (bytes/125)).."kbps"
        end
        return bytes.."bps"
    else
        if bytes > (1024*1024*1024) then 
            return string.format("%.3f", (bytes/(1024*1024*1024))).."GB"
        elseif bytes > (1024*1024) then
            return string.format("%.3f", (bytes/(1024*1024))).."MB"
        elseif bytes > 1024 then
            return string.format("%.3f", (bytes/1024)).."KB"
        end
        return bytes.."B"
    end
end

--Writes to console and JSON file
local function writeToConsole(downloadSpeed, currentDownloaded, uploadSpeed, currentUpload)
    if WARNING ~= nil then
        print("Warning: "..WARNING)
    end

    if ERROR ~= nil then
        print("Error: "..ERROR)
    end

    if tonumber(currentDownloaded) and tonumber(currentDownloaded) > 0 then
        print("Average download speed is "..convertBytes(downloadSpeed, true).." Current downloaded "..convertBytes(currentDownloaded, false))
    elseif tonumber(currentDownloaded) then
        print("Average upload speed is "..convertBytes(uploadSpeed, true).." Current uploaded "..convertBytes(currentUpload, false))
    end

    writeToJSON(downloadSpeed, currentDownloaded, uploadSpeed, currentUpload)

    return true
end

--Writes by choosen paramters to specific places
function writeData(downloadSpeed, currentDownloaded, uploadSpeed, currentUpload)
    if currentDownloaded == DBYTES and currentUpload == UBYTES then
        COUNT = COUNT + 1
        if COUNT == 3 then
            ERROR = "Connection to the speed test server was lost."
            writeData()
            os.exit()
        end
    else
        COUNT = 0;
    end

    DBYTES = currentDownloaded
    UBYTES = currentUpload

    if SILENT then
        writeToJSON(downloadSpeed, currentDownloaded, uploadSpeed, currentUpload)
    else
        writeToConsole(downloadSpeed, currentDownloaded, uploadSpeed, currentUpload)
    end

    if WARNING ~= nil then
        WARNING = nil
    end
    if ERROR ~= nil then
        ERROR = nil
    end
end

--Acts by the given flag from avg
local function flagCheck(num,flag)
    local tmp = 0;

    if flag == "--help" or flag == "-h" then
        print("usage: speedtest [options]\nAvailible options are:\n--help      shows usage of file\n-s          set silent mode\n-u [url]    set server\n-t [time]    set test time\n")
        os.exit()
    elseif flag == "-s" then
        SILENT = true;
    elseif flag == "-u" then
        SERVER = arg[num + 1]
        if SERVER == nil then
            WARNING = "The link was not set correctly"
            writeData()
        end

        SERVER = trimLink(SERVER)

        if socket.connect(SERVER, 80) == nil then
            ERROR = "There was no connection to the given server"
            writeData()
        end

        tmp = 1
    elseif flag == "-t" then
        if arg[num + 1] ~= nil then
            TIME = arg[num + 1]
            tmp = 1
        else
            WARNING = "The time was not set correctly"
            writeData()
        end
    else
        print("The is no such option as "..flag)
    end

    return tmp
end

local function getServerList()
    local body, file

    file = io.open("/tmp/serverlist.json", "r")

    if file ~= nil then
        body = file:read("*all")

        if body == nil or body == "" then
            os.remove("/tmp/serverlist.json")
            ERROR = "Could not get the servers list."
            writeData()
            os.exit()
        end

        file:close()
    else
        file = io.open("/tmp/serverlist.json", "w")
        body = libspeedtest.getbody("https://www.speedtest.net/api/js/servers?engine=js&limit=100&https_functional=true")

        if body == nil or body == "" then
            ERROR = "Could not get the server list."
            writeData()
            os.exit()
        end

        file:write(body)
        file:close()
    end

    return body
end

local function getClosestServer()
    local t
    local body = getServerList()

    t = jsc.parse(body)

    -- Closest server is the first element in json file
    return trimLink(t[1]["url"]);
end

local function checkConnection(url)
    local connection, result

    connection = socket.tcp()
    connection:settimeout(1000)
    result = connection:connect(url, 80)
    connection:close()

    if result then
        return true
    end
    return false
end

writeToJSON()

--Checks if there is a any added flags and does them accordingly.
if #arg > 0 then
    local tmp
    local i = 1

    while #arg >= i do
        tmp = flagCheck(i, arg[i])
        i = i + tmp
        i = i + 1
    end
end

--Looks for internet connection
STATE = "CHECKING_CONNECTION"
writeData()
if not checkConnection("www.google.com") then
    ERROR = "Internet connection is required to use this application."
    writeData()
    os.exit()
end

if not SILENT then
    print("This speedtest can use up a lot of internet data. Do you want to continue?(y/n)")
    local info = io.read();
    if not (info == "y") and not (info == "Y") then
        os.exit()
    end
end


if not SERVER then
    if not SILENT then
        print("Finding closest server..")
    end

    STATE = "FINDING_SERVER"
    writeToJSON()

    SERVER = getClosestServer();
    if not SERVER then
        ERROR = "We could not determine the closest server to you."
        writeData()
        os.exit()
    end
end

if not checkConnection(SERVER) then
    ERROR = "There was no response from the selected server."
    writeData()
    os.exit()
end

if not SILENT then
    print("The selected server: "..SERVER)
end

STATE = "TESTING_DOWNLOAD"
isError, res = libspeedtest.testspeed(SERVER..":8080/download", TIME, false)
if isError then
    ERROR = res
    writeData()
    os.exit()
end

STATE = "COOLDOWN"
writeToJSON()
socket.sleep(3.75)

STATE = "TESTING_UPLOAD"
isError, res = libspeedtest.testspeed(SERVER..":8080/speedtest/upload.php", TIME, true)
if isError then
    ERROR = res
    writeData()
    os.exit()
end

STATE = "FINISHED"
writeToJSON(0, 0, 0, 0)
os.remove("/var/run/speedtest.pid")