--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: asterisk.lua 3618 2008-10-23 02:25:26Z jow $
]]--

cbimap = Map("asterisk", "asterisk", "")

asterisk = cbimap:section(TypedSection, "asterisk", translate("Asterisk General Options"), "")
asterisk.anonymous = true

agidir = asterisk:option(Value, "agidir", translate("AGI directory"), "")
agidir.rmempty = true

cache_record_files = asterisk:option(Flag, "cache_record_files", translate("Cache recorded sound files during recording"), "")
cache_record_files.rmempty = true

debug = asterisk:option(Value, "debug", translate("Debug Level"), "")
debug.rmempty = true

dontwarn = asterisk:option(Flag, "dontwarn", translate("Disable some warnings"), "")
dontwarn.rmempty = true

dumpcore = asterisk:option(Flag, "dumpcore", translate("Dump core on crash"), "")
dumpcore.rmempty = true

highpriority = asterisk:option(Flag, "highpriority", translate("High Priority"), "")
highpriority.rmempty = true

initcrypto = asterisk:option(Flag, "initcrypto", translate("Initialise Crypto"), "")
initcrypto.rmempty = true

internal_timing = asterisk:option(Flag, "internal_timing", translate("Use Internal Timing"), "")
internal_timing.rmempty = true

logdir = asterisk:option(Value, "logdir", translate("Log directory"), "")
logdir.rmempty = true

maxcalls = asterisk:option(Value, "maxcalls", translate("Maximum number of calls allowed"), "")
maxcalls.rmempty = true

maxload = asterisk:option(Value, "maxload", translate("Maximum load to stop accepting new calls"), "")
maxload.rmempty = true

nocolor = asterisk:option(Flag, "nocolor", translate("Disable console colors"), "")
nocolor.rmempty = true

record_cache_dir = asterisk:option(Value, "record_cache_dir", translate("Sound files Cache directory"), "")
record_cache_dir.rmempty = true
record_cache_dir:depends({ ["cache_record_files"] = "true" })

rungroup = asterisk:option(Flag, "rungroup", translate("The Group to run as"), "")
rungroup.rmempty = true

runuser = asterisk:option(Flag, "runuser", translate("The User to run as"), "")
runuser.rmempty = true

spooldir = asterisk:option(Value, "spooldir", translate("Voicemail Spool directory"), "")
spooldir.rmempty = true

systemname = asterisk:option(Value, "systemname", translate("Prefix UniquID with system name"), "")
systemname.rmempty = true

transcode_via_sln = asterisk:option(Flag, "transcode_via_sln", translate("Build transcode paths via SLINEAR, not directly"), "")
transcode_via_sln.rmempty = true

transmit_silence_during_record = asterisk:option(Flag, "transmit_silence_during_record", translate("Transmit SLINEAR silence while recording a channel"), "")
transmit_silence_during_record.rmempty = true

verbose = asterisk:option(Value, "verbose", translate("Verbose Level"), "")
verbose.rmempty = true

zone = asterisk:option(Value, "zone", translate("Time Zone"), "")
zone.rmempty = true


hardwarereboot = cbimap:section(TypedSection, "hardwarereboot", translate("Reload Hardware Config"), "")

method = hardwarereboot:option(ListValue, "method", translate("Reboot Method"), "")
method:value("web", translate("Web URL (wget)"))
method:value("system", translate("program to run"))
method.rmempty = true

param = hardwarereboot:option(Value, "param", translate("Parameter"), "")
param.rmempty = true


iaxgeneral = cbimap:section(TypedSection, "iaxgeneral", translate("IAX General Options"), "")
iaxgeneral.anonymous = true
iaxgeneral.addremove = true

allow = iaxgeneral:option(MultiValue, "allow", translate("Allow Codecs"), "")
allow:value("alaw", "alaw")
allow:value("gsm", "gsm")
allow:value("g726", "g726")
allow.rmempty = true

canreinvite = iaxgeneral:option(ListValue, "canreinvite", translate("Reinvite/redirect media connections"), "")
canreinvite:value("yes", translate("Yes"))
canreinvite:value("nonat", translate("Yes when not behind NAT"))
canreinvite:value("update", translate("Use UPDATE rather than INVITE for path redirection"))
canreinvite:value("no", translate("No"))
canreinvite.rmempty = true

static = iaxgeneral:option(Flag, "static", translate("Static"), "")
static.rmempty = true

writeprotect = iaxgeneral:option(Flag, "writeprotect", translate("Write Protect"), "")
writeprotect.rmempty = true


sipgeneral = cbimap:section(TypedSection, "sipgeneral", translate("Section sipgeneral"), "")
sipgeneral.anonymous = true
sipgeneral.addremove = true

allow = sipgeneral:option(MultiValue, "allow", translate("Allow codecs"), "")
allow:value("ulaw", "ulaw")
allow:value("alaw", "alaw")
allow:value("gsm", "gsm")
allow:value("g726", "g726")
allow.rmempty = true

port = sipgeneral:option(Value, "port", translate("SIP Port"), "")
port.rmempty = true

realm = sipgeneral:option(Value, "realm", translate("SIP realm"), "")
realm.rmempty = true


moh = cbimap:section(TypedSection, "moh", translate("Music On Hold"), "")

application = moh:option(Value, "application", translate("Application"), "")
application.rmempty = true
application:depends({ ["asterisk.moh.mode"] = "custom" })

directory = moh:option(Value, "directory", translate("Directory of Music"), "")
directory.rmempty = true

mode = moh:option(ListValue, "mode", translate("Option mode", ""))
mode:value("system", translate("program to run"))
mode:value("files", translate("Read files from directory"))
mode:value("quietmp3", translate("Quite MP3"))
mode:value("mp3", translate("Loud MP3"))
mode:value("mp3nb", translate("unbuffered MP3"))
mode:value("quietmp3nb", translate("Quiet Unbuffered MP3"))
mode:value("custom", translate("Run a custom application"))
mode.rmempty = true

random = moh:option(Flag, "random", translate("Random Play"), "")
random.rmempty = true


return cbimap
