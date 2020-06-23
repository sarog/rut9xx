luci = require "luci"
sauth = require "luci.sauth"
fs = require "nixio.fs"
dispatcher =require "luci.dispatcher"
util = require "luci.util"
context = util.threadlocal()
--Nustato sesijos failo modifikavimo data tam kad atsinaujinus ntp nedingtu dabartine sesija
--arg 1 timestamp arg 2 sesijos pavadinimas
nixio.fs.utimes(arg[2], arg[1], arg[1])
