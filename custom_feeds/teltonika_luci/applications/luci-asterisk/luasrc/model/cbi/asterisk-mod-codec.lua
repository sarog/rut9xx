--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: asterisk-mod-codec.lua 3618 2008-10-23 02:25:26Z jow $
]]--

cbimap = Map("asterisk", "asterisk", "")

module = cbimap:section(TypedSection, "module", translate("Modules"), "")
module.anonymous = true

codec_a_mu = module:option(ListValue, "codec_a_mu", translate("A-law and Mulaw direct Coder/Decoder"), "")
codec_a_mu:value("yes", translate("Load"))
codec_a_mu:value("no", translate("Do Not Load"))
codec_a_mu:value("auto", translate("Load as Required"))
codec_a_mu.rmempty = true

codec_adpcm = module:option(ListValue, "codec_adpcm", translate("Adaptive Differential PCM Coder/Decoder"), "")
codec_adpcm:value("yes", translate("Load"))
codec_adpcm:value("no", translate("Do Not Load"))
codec_adpcm:value("auto", translate("Load as Required"))
codec_adpcm.rmempty = true

codec_alaw = module:option(ListValue, "codec_alaw", translate("A-law Coder/Decoder"), "")
codec_alaw:value("yes", translate("Load"))
codec_alaw:value("no", translate("Do Not Load"))
codec_alaw:value("auto", translate("Load as Required"))
codec_alaw.rmempty = true

codec_g726 = module:option(ListValue, "codec_g726", translate("ITU G.726-32kbps G726 Transcoder"), "")
codec_g726:value("yes", translate("Load"))
codec_g726:value("no", translate("Do Not Load"))
codec_g726:value("auto", translate("Load as Required"))
codec_g726.rmempty = true

codec_gsm = module:option(ListValue, "codec_gsm", translate("GSM/PCM16 (signed linear) Codec Translation"), "")
codec_gsm:value("yes", translate("Load"))
codec_gsm:value("no", translate("Do Not Load"))
codec_gsm:value("auto", translate("Load as Required"))
codec_gsm.rmempty = true

codec_speex = module:option(ListValue, "codec_speex", translate("Speex/PCM16 (signed linear) Codec Translator"), "")
codec_speex:value("yes", translate("Load"))
codec_speex:value("no", translate("Do Not Load"))
codec_speex:value("auto", translate("Load as Required"))
codec_speex.rmempty = true

codec_ulaw = module:option(ListValue, "codec_ulaw", translate("Mu-law Coder/Decoder"), "")
codec_ulaw:value("yes", translate("Load"))
codec_ulaw:value("no", translate("Do Not Load"))
codec_ulaw:value("auto", translate("Load as Required"))
codec_ulaw.rmempty = true


return cbimap
