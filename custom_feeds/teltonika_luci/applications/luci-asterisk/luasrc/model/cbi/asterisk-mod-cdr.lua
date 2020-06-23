--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: asterisk-mod-cdr.lua 3618 2008-10-23 02:25:26Z jow $
]]--

cbimap = Map("asterisk", "asterisk", "")

module = cbimap:section(TypedSection, "module", translate("Modules"), "")
module.anonymous = true

cdr_csv = module:option(ListValue, "cdr_csv", translate("Comma Separated Values CDR Backend"), "")
cdr_csv:value("yes", translate("Load"))
cdr_csv:value("no", translate("Do Not Load"))
cdr_csv:value("auto", translate("Load as Required"))
cdr_csv.rmempty = true

cdr_custom = module:option(ListValue, "cdr_custom", translate("Customizable Comma Separated Values CDR Backend"), "")
cdr_custom:value("yes", translate("Load"))
cdr_custom:value("no", translate("Do Not Load"))
cdr_custom:value("auto", translate("Load as Required"))
cdr_custom.rmempty = true

cdr_manager = module:option(ListValue, "cdr_manager", translate("Asterisk Call Manager CDR Backend"), "")
cdr_manager:value("yes", translate("Load"))
cdr_manager:value("no", translate("Do Not Load"))
cdr_manager:value("auto", translate("Load as Required"))
cdr_manager.rmempty = true

cdr_mysql = module:option(ListValue, "cdr_mysql", translate("MySQL CDR Backend"), "")
cdr_mysql:value("yes", translate("Load"))
cdr_mysql:value("no", translate("Do Not Load"))
cdr_mysql:value("auto", translate("Load as Required"))
cdr_mysql.rmempty = true

cdr_pgsql = module:option(ListValue, "cdr_pgsql", translate("PostgreSQL CDR Backend"), "")
cdr_pgsql:value("yes", translate("Load"))
cdr_pgsql:value("no", translate("Do Not Load"))
cdr_pgsql:value("auto", translate("Load as Required"))
cdr_pgsql.rmempty = true

cdr_sqlite = module:option(ListValue, "cdr_sqlite", translate("SQLite CDR Backend"), "")
cdr_sqlite:value("yes", translate("Load"))
cdr_sqlite:value("no", translate("Do Not Load"))
cdr_sqlite:value("auto", translate("Load as Required"))
cdr_sqlite.rmempty = true


return cbimap
