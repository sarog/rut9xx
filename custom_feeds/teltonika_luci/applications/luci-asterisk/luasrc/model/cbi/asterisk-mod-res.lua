--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: asterisk-mod-res.lua 3618 2008-10-23 02:25:26Z jow $
]]--

cbimap = Map("asterisk", "asterisk", "")

module = cbimap:section(TypedSection, "module", translate("Modules"), "")
module.anonymous = true

res_config_mysql = module:option(ListValue, "res_config_mysql", translate("MySQL Config Resource"), "")
res_config_mysql:value("yes", translate("Load"))
res_config_mysql:value("no", translate("Do Not Load"))
res_config_mysql:value("auto", translate("Load as Required"))
res_config_mysql.rmempty = true

res_config_odbc = module:option(ListValue, "res_config_odbc", translate("ODBC Config Resource"), "")
res_config_odbc:value("yes", translate("Load")
res_config_odbc:value("no", translate("Do Not Load"))
res_config_odbc:value("auto", translate("Load as Required"))
res_config_odbc.rmempty = true

res_config_pgsql = module:option(ListValue, "res_config_pgsql", translate("PGSQL Module"), "")
res_config_pgsql:value("yes", translate("Load"))
res_config_pgsql:value("no", translate("Do Not Load"))
res_config_pgsql:value("auto", translate("Load as Required"))
res_config_pgsql.rmempty = true

res_crypto = module:option(ListValue, "res_crypto", translate("Cryptographic Digital Signatures"), "")
res_crypto:value("yes", translate("Load"))
res_crypto:value("no", translate("Do Not Load"))
res_crypto:value("auto", translate("Load as Required"))
res_crypto.rmempty = true

res_features = module:option(ListValue, "res_features", translate("Call Parking Resource"), "")
res_features:value("yes", translate("Load"))
res_features:value("no", translate("Do Not Load"))
res_features:value("auto", translate("Load as Required"))
res_features.rmempty = true

res_indications = module:option(ListValue, "res_indications", translate("Indications Configuration"), "")
res_indications:value("yes", translate("Load"))
res_indications:value("no", translate("Do Not Load"))
res_indications:value("auto", translate("Load as Required"))
res_indications.rmempty = true

res_monitor = module:option(ListValue, "res_monitor", translate("Call Monitoring Resource"), "")
res_monitor:value("yes", translate("Load"))
res_monitor:value("no", translate("Do Not Load"))
res_monitor:value("auto", translate("Load as Required"))
res_monitor.rmempty = true

res_musiconhold = module:option(ListValue, "res_musiconhold", translate("Music On Hold Resource"), "")
res_musiconhold:value("yes", translate("Load"))
res_musiconhold:value("no", translate("Do Not Load"))
res_musiconhold:value("auto", translate("Load as Required"))
res_musiconhold.rmempty = true

res_odbc = module:option(ListValue, "res_odbc", translate("ODBC Resource"), "")
res_odbc:value("yes", translate("Load"))
res_odbc:value("no", translate("Do Not Load"))
res_odbc:value("auto", translate("Load as Required"))
res_odbc.rmempty = true

res_smdi = module:option(ListValue, "res_smdi", translate("SMDI Module"), "")
res_smdi:value("yes", translate("Load"))
res_smdi:value("no", translate("Do Not Load"))
res_smdi:value("auto", translate("Load as Required"))
res_smdi.rmempty = true

res_snmp = module:option(ListValue, "res_snmp", translate("SNMP Module"), "")
res_snmp:value("yes", translate("Load"))
res_snmp:value("no", translate("Do Not Load"))
res_snmp:value("auto", translate("Load as Required"))
res_snmp.rmempty = true


return cbimap
