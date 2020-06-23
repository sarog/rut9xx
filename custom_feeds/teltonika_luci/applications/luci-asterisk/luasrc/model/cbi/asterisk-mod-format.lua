--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: asterisk-mod-format.lua 3618 2008-10-23 02:25:26Z jow $
]]--

cbimap = Map("asterisk", "asterisk", "")

module = cbimap:section(TypedSection, "module", translate("Modules"), "")
module.anonymous = true

format_au = module:option(ListValue, "format_au", translate("Sun Microsystems AU format (signed linear)"), "")
format_au:value("yes", translate("Load"))
format_au:value("no", translate("Do Not Load"))
format_au:value("auto", translate("Load as Required"))
format_au.rmempty = true

format_g723 = module:option(ListValue, "format_g723", translate("G.723.1 Simple Timestamp File Format"), "")
format_g723:value("yes", translate("Load")
format_g723:value("no", translate("Do Not Load"))
format_g723:value("auto", translate("Load as Required"))
format_g723.rmempty = true

format_g726 = module:option(ListValue, "format_g726", translate("Raw G.726 (16/24/32/40kbps) data"), "")
format_g726:value("yes", translate("Load"))
format_g726:value("no", translate("Do Not Load"))
format_g726:value("auto", translate("Load as Required"))
format_g726.rmempty = true

format_g729 = module:option(ListValue, "format_g729", translate("Raw G729 data"), "")
format_g729:value("yes", translate("Load"))
format_g729:value("no", translate("Do Not Load"))
format_g729:value("auto", translate("Load as Required"))
format_g729.rmempty = true

format_gsm = module:option(ListValue, "format_gsm", translate("Raw GSM data"), "")
format_gsm:value("yes", translate("Load"))
format_gsm:value("no", translate("Do Not Load"))
format_gsm:value("auto", translate("Load as Required"))
format_gsm.rmempty = true

format_h263 = module:option(ListValue, "format_h263", translate("Raw h263 data"), "")
format_h263:value("yes", translate("Load"))
format_h263:value("no", translate("Do Not Load"))
format_h263:value("auto", translate("Load as Required"))
format_h263.rmempty = true

format_jpeg = module:option(ListValue, "format_jpeg", translate("JPEG (Joint Picture Experts Group) Image"), "")
format_jpeg:value("yes", translate("Load"))
format_jpeg:value("no", translate("Do Not Load"))
format_jpeg:value("auto", translate("Load as Required"))
format_jpeg.rmempty = true

format_pcm = module:option(ListValue, "format_pcm", translate("Raw uLaw 8khz Audio support (PCM)"), "")
format_pcm:value("yes", translate("Load"))
format_pcm:value("no", translate("Do Not Load"))
format_pcm:value("auto", translate("Load as Required"))
format_pcm.rmempty = true

format_pcm_alaw = module:option(ListValue, "format_pcm_alaw", translate("load => .so ; Raw aLaw 8khz PCM Audio support"), "")
format_pcm_alaw:value("yes", translate("Load")
format_pcm_alaw:value("no", translate("Do Not Load"))
format_pcm_alaw:value("auto", translate("Load as Required"))
format_pcm_alaw.rmempty = true

format_sln = module:option(ListValue, "format_sln", translate("Raw Signed Linear Audio support (SLN)"), "")
format_sln:value("yes", translate("Load"))
format_sln:value("no", translate("Do Not Load"))
format_sln:value("auto", translate("Load as Required"))
format_sln.rmempty = true

format_vox = module:option(ListValue, "format_vox", translate("Dialogic VOX (ADPCM) File Format"), "")
format_vox:value("yes", translate("Load"))
format_vox:value("no", translate("Do Not Load"))
format_vox:value("auto", translate("Load as Required"))
format_vox.rmempty = true

format_wav = module:option(ListValue, "format_wav", translate("Microsoft WAV format (8000hz Signed Line)"), "")
format_wav:value("yes", translate("Load"))
format_wav:value("no", translate("Do Not Load"))
format_wav:value("auto", translate("Load as Required"))
format_wav.rmempty = true

format_wav_gsm = module:option(ListValue, "format_wav_gsm", translate("Microsoft WAV format (Proprietary GSM)"), "")
format_wav_gsm:value("yes", translate("Load"))
format_wav_gsm:value("no", translate("Do Not Load"))
format_wav_gsm:value("auto", translate("Load as Required"))
format_wav_gsm.rmempty = true


return cbimap
