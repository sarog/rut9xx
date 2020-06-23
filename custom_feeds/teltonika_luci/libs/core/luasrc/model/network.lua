--[[
LuCI - Network model

Copyright 2009-2010 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

]]--

local type, next, pairs, ipairs, print, loadfile, table
	= type, next, pairs, ipairs, print, loadfile, table

local tonumber, tostring, math, i18n
	= tonumber, tostring, math, luci.i18n

local require = require
local bus = require "ubus"
local nxo = require "nixio"
local nfs = require "nixio.fs"
local ipc = require "luci.ip"
local sys = require "luci.sys"
local utl = require "luci.util"
local dsp = require "luci.dispatcher"
local uci = require "luci.model.uci"
local io = require "io"
module "luci.model.network"

IFACE_PATTERNS_VIRTUAL  = { }
IFACE_PATTERNS_IGNORE   = { "^wmaster%d", "^wifi%d", "^hwsim%d", "^imq%d", "^ifb%d", "^mon%.wlan%d", "^sit%d", "^lo$" }
IFACE_PATTERNS_WIRELESS = { "^wlan%d", "^wl%d", "^ath%d", "^%w+%.network%d" }


protocol = utl.class()

local _protocols = { }

local _interfaces, _bridge, _switch, _tunnel
local _uci_real, _uci_state
local _ubus, _ubusnetcache
function _filter(c, s, o, r)
	local val = _uci_real:get(c, s, o)
	if val then
		local l = { }
		if type(val) == "string" then
			for val in val:gmatch("%S+") do
				if val ~= r then
					l[#l+1] = val
				end
			end
			if #l > 0 then
				_uci_real:set(c, s, o, table.concat(l, " "))
			else
				_uci_real:delete(c, s, o)
			end
		elseif type(val) == "table" then
			for _, val in ipairs(val) do
				if val ~= r then
					l[#l+1] = val
				end
			end
			if #l > 0 then
				_uci_real:set(c, s, o, l)
			else
				_uci_real:delete(c, s, o)
			end
		end
	end
end

function cecho(string)
	sys.call("echo \"" .. string .. "\" >> /tmp/log.log")
end

function _append(c, s, o, a)
	local val = _uci_real:get(c, s, o) or ""
	if type(val) == "string" then
		local l = { }
		for val in val:gmatch("%S+") do
			if val ~= a then
				l[#l+1] = val
			end
		end
		l[#l+1] = a
		_uci_real:set(c, s, o, table.concat(l, " "))
	elseif type(val) == "table" then
		local l = { }
		for _, val in ipairs(val) do
			if val ~= a then
				l[#l+1] = val
			end
		end
		l[#l+1] = a
		_uci_real:set(c, s, o, l)
	end
end

function _stror(s1, s2)
	if not s1 or #s1 == 0 then
		return s2 and #s2 > 0 and s2
	else
		return s1
	end
end

function _get(c, s, o)
	return _uci_real:get(c, s, o)
end

function _set(c, s, o, v)
	if v ~= nil then
		if type(v) == "boolean" then v = v and "1" or "0" end
		return _uci_real:set(c, s, o, v)
	else
		return _uci_real:delete(c, s, o)
	end
end

function _wifi_iface(x)
	local _, p
	for _, p in ipairs(IFACE_PATTERNS_WIRELESS) do
		if x:match(p) then
			return true
		end
	end
	return false
end

function _wifi_lookup(ifn)
	-- got a radio#.network# pseudo iface, locate the corresponding section
	local radio, ifnidx = ifn:match("^(%w+)%.network(%d+)$")
	if radio and ifnidx then
		local sid = nil
		local num = 0

		ifnidx = tonumber(ifnidx)
		_uci_real:foreach("wireless", "wifi-iface",
			function(s)
				if s.device == radio then
					num = num + 1
					if num == ifnidx then
						sid = s['.name']
						return false
					end
				end
			end)

		return sid

	-- looks like wifi, try to locate the section via state vars
	elseif _wifi_iface(ifn) then
		local sid = nil

		_uci_state:foreach("wireless", "wifi-iface",
			function(s)
				if s.ifname == ifn then
					sid = s['.name']
					return false
				end
			end)

		return sid
	end
end

function _iface_virtual(x)
	local _, p
	for _, p in ipairs(IFACE_PATTERNS_VIRTUAL) do
		if x:match(p) then
			return true
		end
	end
	return false
end

function _iface_ignore(x)
	local _, p
	for _, p in ipairs(IFACE_PATTERNS_IGNORE) do
		if x:match(p) then
			return true
		end
	end
	return _iface_virtual(x)
end


function init(cursor)
	_uci_real  = cursor or _uci_real or uci.cursor()
	_uci_state = _uci_real:substate()

	_interfaces = { }
	_bridge     = { }
	_switch     = { }
	_tunnel     = { }

	 _ubus          = bus.connect()
    _ubusnetcache  = { }

	-- read interface information
	local n, i
	for n, i in ipairs(nxo.getifaddrs()) do
		local name = i.name:match("[^:]+")
		local prnt = name:match("^([^%.]+)%.")

		if _iface_virtual(name) then
			_tunnel[name] = true
		end

		if _tunnel[name] or not _iface_ignore(name) then
			_interfaces[name] = _interfaces[name] or {
				idx      = i.ifindex or n,
				name     = name,
				rawname  = i.name,
				flags    = { },
				ipaddrs  = { },
				ip6addrs = { }
			}

			if prnt then
				_switch[name] = true
				_switch[prnt] = true
			end

			if i.family == "packet" then
				_interfaces[name].flags   = i.flags
				_interfaces[name].stats   = i.data
				_interfaces[name].macaddr = i.addr
			elseif i.family == "inet" then
				_interfaces[name].ipaddrs[#_interfaces[name].ipaddrs+1] = ipc.IPv4(i.addr, i.netmask)
			elseif i.family == "inet6" then
				_interfaces[name].ip6addrs[#_interfaces[name].ip6addrs+1] = ipc.IPv6(i.addr, i.netmask)
			end
		end
	end

	-- read bridge informaton
	local b, l
	for l in utl.execi("brctl show") do
		if not l:match("STP") then
			local r = utl.split(l, "%s+", nil, true)
			if #r == 4 then
				b = {
					name    = r[1],
					id      = r[2],
					stp     = r[3] == "yes",
					ifnames = { _interfaces[r[4]] }
				}
				if b.ifnames[1] then
					b.ifnames[1].bridge = b
				end
				_bridge[r[1]] = b
			elseif b then
				b.ifnames[#b.ifnames+1] = _interfaces[r[2]]
				b.ifnames[#b.ifnames].bridge = b
			end
		end
	end

	return _M
end

function save(self, ...)
	_uci_real:save(...)
	_uci_real:load(...)
end

function commit(self, ...)
	_uci_real:commit(...)
	_uci_real:load(...)
end

function ifnameof(self, x)
	if utl.instanceof(x, interface) then
		return x:name()
	elseif utl.instanceof(x, protocol) then
		return x:ifname()
	elseif type(x) == "string" then
		return x:match("^[^:]+")
	end
end

function get_protocol(self, protoname, netname)
	local v = _protocols[protoname]
	if v then
		return v(netname or "__dummy__")
	end
end

function get_protocols(self)
	local p = { }
	local _, v
	for _, v in ipairs(_protocols) do
		p[#p+1] = v("__dummy__")
	end
	return p
end

function register_protocol(self, protoname)
	local proto = utl.class(protocol)

	function proto.__init__(self, name)
		self.sid = name
	end

	function proto.proto(self)
		return protoname
	end

	_protocols[#_protocols+1] = proto
	_protocols[protoname]     = proto

	return proto
end

function register_pattern_virtual(self, pat)
	IFACE_PATTERNS_VIRTUAL[#IFACE_PATTERNS_VIRTUAL+1] = pat
end


function has_ipv6(self)
	return nfs.access("/proc/net/ipv6_route")
end

function add_network(self, n, options)
	local oldnet = self:get_network(n)
	if n and #n > 0 and n:match("^[a-zA-Z0-9_]+$") and not oldnet then
		if _uci_real:section("network", "interface", n, options) then
			return network(n)
		end
	elseif oldnet and oldnet:is_empty() then
		if options then
			local k, v
			for k, v in pairs(options) do
				oldnet:set(k, v)
			end
		end
		return oldnet
	end
end

function get_network(self, n)
	if n and _uci_real:get("network", n) == "interface" then
		return network(n)
	end
end

function get_networks(self)
	local nets = { }
	local nls = { }

	_uci_real:foreach("network", "interface",
		function(s)
			nls[s['.name']] = network(s['.name'])
		end)

	local n
	for n in utl.kspairs(nls) do
		nets[#nets+1] = nls[n]
	end

	return nets
end

function del_network(self, n)
	local r = _uci_real:delete("network", n)
	if r then
		_uci_real:delete_all("network", "alias",
			function(s) return (s.interface == n) end)

		_uci_real:delete_all("network", "route",
			function(s) return (s.interface == n) end)

		_uci_real:delete_all("network", "route6",
			function(s) return (s.interface == n) end)

		_uci_real:foreach("wireless", "wifi-iface",
			function(s)
				if s.network == n then
					_uci_real:delete("wireless", s['.name'], "network")
				end
			end)
	end
	return r
end

function rename_network(self, old, new)
	local r
	if new and #new > 0 and new:match("^[a-zA-Z0-9_]+$") and not self:get_network(new) then
		r = _uci_real:section("network", "interface", new, _uci_real:get_all("network", old))

		if r then
			_uci_real:foreach("network", "alias",
				function(s)
					if s.interface == old then
						_uci_real:set("network", s['.name'], "interface", new)
					end
				end)

			_uci_real:foreach("network", "route",
				function(s)
					if s.interface == old then
						_uci_real:set("network", s['.name'], "interface", new)
					end
				end)

			_uci_real:foreach("network", "route6",
				function(s)
					if s.interface == old then
						_uci_real:set("network", s['.name'], "interface", new)
					end
				end)

			_uci_real:foreach("wireless", "wifi-iface",
				function(s)
					if s.network == old then
						_uci_real:set("wireless", s['.name'], "network", new)
					end
				end)

			_uci_real:delete("network", old)
		end
	end
	return r or false
end
function rename_network_custom(self, old, new)
	local r
	if new and #new > 0 and new:match("^[a-zA-Z0-9_]+$") and not self:get_network(new) then
		r = _uci_real:section("network", "interface", new, _uci_real:get_all("network", old))

		if r then
			_uci_real:foreach("network", "alias",
				function(s)
					if s.interface == old then
						_uci_real:set("network", s['.name'], "interface", new)
					end
				end)

			_uci_real:foreach("network", "route6",
				function(s)
					if s.interface == old then
						_uci_real:set("network", s['.name'], "interface", new)
					end
				end)

			_uci_real:foreach("wireless", "wifi-iface",
				function(s)
					if s.network == old then
						_uci_real:set("wireless", s['.name'], "network", new)
					end
				end)

			_uci_real:delete("network", old)
		end
	end
	return r or false
end

function rename_multiwan_section(self, old, new)
	local r
	if new and #new > 0 and new:match("^[a-zA-Z0-9_]+$") and not _uci_real:get("multiwan", new) then
		r = _uci_real:section("multiwan", "interface", new, _uci_real:get_all("multiwan", old))

		if r then
			_uci_real:delete("multiwan", old)
		end
	end
	return r or false
end

function get_interface(self, i)
	if _interfaces[i] or _wifi_iface(i) then
		return interface(i)
	else
		local ifc
		local num = { }
		_uci_real:foreach("wireless", "wifi-iface",
			function(s)
				if s.device then
					num[s.device] = num[s.device] and num[s.device] + 1 or 1
					if s['.name'] == i then
						ifc = interface(
							"%s.network%d" %{s.device, num[s.device] })
						return false
					end
				end
			end)
		return ifc
	end
end

function get_interfaces(self)
	local iface
	local ifaces = { }
	local seen = { }
	local nfs = { }
	local baseof = { }

	-- find normal interfaces
	_uci_real:foreach("network", "interface",
		function(s)
			for iface in utl.imatch(s.ifname) do
				if not _iface_ignore(iface) and not _wifi_iface(iface) then
					seen[iface] = true
					nfs[iface] = interface(iface)
				end
			end
		end)

	for iface in utl.kspairs(_interfaces) do
		if not (seen[iface] or _iface_ignore(iface) or _wifi_iface(iface)) then
			nfs[iface] = interface(iface)
		end
	end

	-- find vlan interfaces
	_uci_real:foreach("network", "switch_vlan",
		function(s)
			if not s.device then
				return
			end

			local base = baseof[s.device]
			if not base then
				if not s.device:match("^eth%d") then
					local l
					for l in utl.execi("swconfig dev %q help 2>/dev/null" % s.device) do
						if not base then
							base = l:match("^%w+: (%w+)")
						end
					end
					if not base or not base:match("^eth%d") then
						base = "eth0"
					end
				else
					base = s.device
				end
				baseof[s.device] = base
			end

			local vid = tonumber(s.vid or s.vlan)
			if vid ~= nil and vid >= 0 and vid <= 4095 then
				local iface = "%s.%d" %{ base, vid }
				if not seen[iface] then
					seen[iface] = true
					nfs[iface] = interface(iface)
				end
			end
		end)

	for iface in utl.kspairs(nfs) do
		ifaces[#ifaces+1] = nfs[iface]
	end

	-- find wifi interfaces
	local num = { }
	local wfs = { }
	_uci_real:foreach("wireless", "wifi-iface",
		function(s)
			if s.device then
				num[s.device] = num[s.device] and num[s.device] + 1 or 1
				local i = "%s.network%d" %{ s.device, num[s.device] }
				wfs[i] = interface(i)
			end
		end)

	for iface in utl.kspairs(wfs) do
		ifaces[#ifaces+1] = wfs[iface]
	end

	return ifaces
end

function ignore_interface(self, x)
	return _iface_ignore(x)
end

function get_wifidev(self, dev)
	if _uci_real:get("wireless", dev) == "wifi-device" then
		return wifidev(dev)
	end
end

function get_wifidevs(self)
	local devs = { }
	local wfd  = { }

	_uci_real:foreach("wireless", "wifi-device",
		function(s) wfd[#wfd+1] = s['.name'] end)

	local dev
	for _, dev in utl.vspairs(wfd) do
		devs[#devs+1] = wifidev(dev)
	end

	return devs
end

function get_wifinet(self, net)
	local wnet = _wifi_lookup(net)
	if wnet then
		return wifinet(wnet)
	end
end

function add_wifinet(self, net, options)
	if type(options) == "table" and options.device and
		_uci_real:get("wireless", options.device) == "wifi-device"
	then
		local wnet = _uci_real:section("wireless", "wifi-iface", nil, options)
		return wifinet(wnet)
	end
end

function del_wifinet(self, net)
	local wnet = _wifi_lookup(net)
	if wnet then
		_uci_real:delete("wireless", wnet)
		return true
	end
	return false
end


function network(name, proto)
	if name then
		local p = proto or _uci_real:get("network", name, "proto")
		local c = p and _protocols[p] or protocol
		return c(name)
	end
end

function protocol.__init__(self, name)
	self.sid = name
end

function protocol._get(self, opt)
	local v = _uci_real:get("network", self.sid, opt)
	if type(v) == "table" then
		return table.concat(v, " ")
	end
	return v or ""
end

function protocol._ip(self, opt, family, list)
	local ip = _uci_state:get("network", self.sid, opt)
	local fc = (family == 6) and ipc.IPv6 or ipc.IPv4
	if ip or list then
		if list then
			local l = { }
			for ip in utl.imatch(ip) do
				ip = fc(ip)
				if ip then l[#l+1] = ip:string() end
			end
			return l
		else
			ip = fc(ip)
			return ip and ip:string()
		end
	end
end

--"ubus call network.interface.(wan, lan, t.t...) status" funkcija informacijai gauti--
---------------------------------------------------------------------------------------


function protocol._ubus(self, field)
	-- workaround
	local prot = utl.trim(sys.exec("uci get -q network.ppp.proto"))
	local ppp_backup = utl.trim(sys.exec("uci get -q network.ppp.backup"))
	local ifname = _uci_real:get("network", self.sid, "ifname") or _uci_state:get("network", self.sid, "ifname")

	if ifname ~= "eth1" and ifname ~= "wlan0" and self.sid == "ppp" then
		if prot == "ndis" and ppp_backup == "0" then
			self.sid = "wan"
		elseif prot == "ndis" and ppp_backup == "1" then
			self.sid = "wan2"
		elseif prot == "qmi" then
			self.sid = "ppp_dhcp"
		elseif prot == "ncm" or prot == "qmi2" then
			self.sid = "ppp_4"
		end
	elseif self.sid == "ppp_usb" then
		self.sid = "ppp_usb_4"
	end

	if not _ubusnetcache[self.sid] then
		_ubusnetcache[self.sid] = _ubus:call("network.interface.%s" % self.sid, "status", { })
	end
	if _ubusnetcache[self.sid] and field then
		return _ubusnetcache[self.sid][field]
	end
	return _ubusnetcache[self.sid]
end

function protocol.get(self, opt)
	return _get("network", self.sid, opt)
end

function protocol.set(self, opt, val)
	return _set("network", self.sid, opt, val)
end

function protocol.ifname(self)
	local p = self:proto()
	if self:is_bridge() then
		return "br-" .. self.sid
	elseif self:is_virtual() then
		return p .. "-" .. self.sid
	else
		local num = { }
		--self.sid wan, ppp , lan , gre ...
		local dev = _uci_real:get("network", self.sid, "ifname") or
			_uci_state:get("network", self.sid, "ifname")
		dev = (type(dev) == "table") and dev[1] or dev
		dev = (dev ~= nil) and dev:match("%S+")

		if not dev then
			_uci_real:foreach("wireless", "wifi-iface",
				function(s)
					if s.device then
						num[s.device] = num[s.device]
							and num[s.device] + 1 or 1

						if s.network == self.sid then
							dev = "%s.network%d" %{ s.device, num[s.device] }
							return false
						end
					end
				end)
		end

		return dev
	end
end

function protocol.proto(self)
	return "none"
end

function protocol.get_i18n(self)
	local p = self:proto()
	if p == "none" then
		return i18n.translate("Unmanaged")
	elseif p == "static" then
		return i18n.translate("Static address")
	elseif p == "dhcp" then
		return i18n.translate("DHCP client")
	else
		return i18n.translate("Unknown")
	end
end

function protocol.type(self)
	return self:_get("type")
end

function protocol.name(self)
	return self.sid
end


--Funkcijos ip, netmask, dns addr ir tt. gavimui--
--///////////////////////////////////////////////-

function protocol.uptime(self)
    return self:_ubus("uptime") or 0
end


function protocol.expires(self)
	local a = tonumber(_uci_state:get("network", self.sid, "lease_acquired"))
	local l = tonumber(_uci_state:get("network", self.sid, "lease_lifetime"))
	if a and l then
		l = l - (nxo.sysinfo().uptime - a)
		return l > 0 and l or 0
	end
	return -1
end

function protocol.metric(self)
	return tonumber(_uci_state:get("network", self.sid, "metric")) or 0
end

function protocol.ipaddr(self)
    local addrs = self:_ubus("ipv4-address")
	return addrs and #addrs > 0 and addrs[1].address
end

function protocol.netmask(self)
    local addrs = self:_ubus("ipv4-address")
    return addrs and #addrs > 0 and
    ipc.IPv4("0.0.0.0/%d" % addrs[1].mask):mask():string()
end


function protocol.gwaddr(self)
    local _, route
    for _, route in ipairs(self:_ubus("route") or { }) do
        if route.target == "0.0.0.0" and route.mask == 0 and not route.table then
            return route.nexthop
        end
    end
end


function protocol.dnsaddrs(self)
    local dns = { }
    local _, addr
    for _, addr in ipairs(self:_ubus("dns-server") or { }) do
        if not addr:match(":") then
            dns[#dns+1] = addr
        end
    end
    return dns
end


function protocol.ip6addr(self)
    local addrs = self:_ubus("ipv6-address")
    if addrs and #addrs > 0 then
        return "%s/%d" %{ addrs[1].address, addrs[1].mask }
    else
        addrs = self:_ubus("ipv6-prefix-assignment")
        if addrs and #addrs > 0 then
            return "%s/%d" %{ addrs[1].address, addrs[1].mask }
        end
    end
end

function protocol.gw6addr(self)
    local _, route
    for _, route in ipairs(self:_ubus("route") or { }) do
        if route.target == "::" and route.mask == 0 then
            return ipc.IPv6(route.nexthop):string()
        end
    end
end


function protocol.dns6addrs(self)
    local dns = { }
    local _, addr
    for _, addr in ipairs(self:_ubus("dns-server") or { }) do
        if addr:match(":") then
            dns[#dns+1] = addr
        end
    end
    return dns
end
--////////////////////////////////////////////////////////--
-------------------------------------------------------------


function protocol.is_bridge(self)
	return (not self:is_virtual() and self:type() == "bridge")
end

function protocol.opkg_package(self)
	return nil
end

function protocol.is_installed(self)
	return true
end

function protocol.is_virtual(self)
	return false
end

function protocol.is_floating(self)
	return false
end

function protocol.is_empty(self)
	if self:is_floating() then
		return false
	else
		local rv = true

		if (self:_get("ifname") or ""):match("%S+") then
			rv = false
		end

		_uci_real:foreach("wireless", "wifi-iface",
			function(s)
				if s.network == self.sid then
					rv = false
					return false
				end
			end)

		return rv
	end
end

function protocol.add_interface(self, ifname)
	ifname = _M:ifnameof(ifname)
	if ifname and not self:is_floating() then
		-- remove the interface from all ifaces
		_uci_real:foreach("network", "interface",
			function(s)
				_filter("network", s['.name'], "ifname", ifname)
			end)

		-- if its a wifi interface, change its network option
		local wif = _wifi_lookup(ifname)
		if wif then
			_uci_real:set("wireless", wif, "network", self.sid)

		-- add iface to our iface list
		else
			_append("network", self.sid, "ifname", ifname)
		end
	end
end

function protocol.del_interface(self, ifname)
	ifname = _M:ifnameof(ifname)
	if ifname and not self:is_floating() then
		-- if its a wireless interface, clear its network option
		local wif = _wifi_lookup(ifname)
		if wif then	_uci_real:delete("wireless", wif, "network") end

		-- remove the interface
		_filter("network", self.sid, "ifname", ifname)
	end
end

function protocol.get_interface(self, ignoreVirtuality)
	if self:is_virtual() and not ignoreVirtuality then
		_tunnel[self:proto() .. "-" .. self.sid] = true
		return interface(self:proto() .. "-" .. self.sid, self)
	elseif self:is_bridge() then
		_bridge["br-" .. self.sid] = true
		return interface("br-" .. self.sid, self)
	else
		local ifn = nil
		local num = { }
		for ifn in utl.imatch(_uci_real:get("network", self.sid, "ifname")) do
			ifn = ifn:match("^[^:/]+")
			return ifn and interface(ifn, self)
		end
		ifn = nil
		_uci_real:foreach("wireless", "wifi-iface",
			function(s)
				if s.device then
					num[s.device] = num[s.device] and num[s.device] + 1 or 1
					if s.network == self.sid then
						ifn = "%s.network%d" %{ s.device, num[s.device] }
						return false
					end
				end
			end)
		return ifn and interface(ifn, self)
	end
end

function protocol.get_interfaces(self)
	if self:is_bridge() or (self:is_virtual() and not self:is_floating()) then
		local ifaces = { }

		local ifn
		local nfs = { }
		for ifn in utl.imatch(self:get("ifname")) do
			ifn = ifn:match("^[^:/]+")
			nfs[ifn] = interface(ifn, self)
		end

		for ifn in utl.kspairs(nfs) do
			ifaces[#ifaces+1] = nfs[ifn]
		end

		local num = { }
		local wfs = { }
		_uci_real:foreach("wireless", "wifi-iface",
			function(s)
				if s.device then
					num[s.device] = num[s.device] and num[s.device] + 1 or 1
					if s.network == self.sid then
						ifn = "%s.network%d" %{ s.device, num[s.device] }
						wfs[ifn] = interface(ifn, self)
					end
				end
			end)

		for ifn in utl.kspairs(wfs) do
			ifaces[#ifaces+1] = wfs[ifn]
		end

		return ifaces
	end
end

function protocol.contains_interface(self, ifname)
	ifname = _M:ifnameof(ifname)
	if not ifname then
		return false
	elseif self:is_virtual() and self:proto() .. "-" .. self.sid == ifname then
		return true
	elseif self:is_bridge() and "br-" .. self.sid == ifname then
		return true
	else
		local ifn
		for ifn in utl.imatch(self:get("ifname")) do
			ifn = ifn:match("[^:]+")
			if ifn == ifname then
				return true
			end
		end

		local wif = _wifi_lookup(ifname)
		if wif then
			return (_uci_real:get("wireless", wif, "network") == self.sid)
		end
	end

	return false
end

function protocol.adminlink(self)
	return dsp.build_url("admin", "network", "network", self.sid)
end


interface = utl.class()

function interface.__init__(self, ifname, network)
	local wif = _wifi_lookup(ifname)
	if wif then
		self.wif    = wifinet(wif)
		self.ifname = _uci_state:get("wireless", wif, "ifname")
	end

	self.ifname  = self.ifname or ifname
	self.dev     = _interfaces[self.ifname]
	self.network = network
end

function interface.name(self)
	return self.wif and self.wif:ifname() or self.ifname
end

function interface.mac(self)
	return (self.dev and self.dev.macaddr or "00:00:00:00:00:00"):upper()
end

function interface.ipaddrs(self)
	return self.dev and self.dev.ipaddrs or { }
end

function interface.ip6addrs(self)
	return self.dev and self.dev.ip6addrs or { }
end

function interface.type(self)
	if self.wif or _wifi_iface(self.ifname) then
		return "wifi"
	elseif _bridge[self.ifname] then
		return "bridge"
	elseif _tunnel[self.ifname] then
		return "tunnel"
	elseif self.ifname:match("%.") then
		return "vlan"
	elseif _switch[self.ifname] then
		return "switch"
	else
		return "ethernet"
	end
end

function interface.shortname(self)
	if self.wif then
		return "%s %q" %{
			self.wif:active_mode(),
			self.wif:active_ssid() or self.wif:active_bssid()
		}
	else
		return self.ifname
	end
end

function interface.get_i18n(self)
	if self.wif then
		return "%s: %s %q" %{
			i18n.translate("Wireless Network"),
			self.wif:active_mode(),
			self.wif:active_ssid() or self.wif:active_bssid()
		}
	else
		return "%s: %q" %{ self:get_type_i18n(), self:name() }
	end
end

function interface.get_type_i18n(self)
	local x = self:type()
	if x == "wifi" then
		return i18n.translate("Wireless Adapter")
	elseif x == "bridge" then
		return i18n.translate("Bridge")
	elseif x == "switch" then
		return i18n.translate("Ethernet Switch")
	elseif x == "vlan" then
		return i18n.translate("VLAN Interface")
	elseif x == "tunnel" then
		return i18n.translate("Tunnel Interface")
	else
		return i18n.translate("Ethernet Adapter")
	end
end

function interface.adminlink(self)
	if self.wif then
		return self.wif:adminlink()
	end
end

function interface.ports(self)
	if self.br then
		local iface
		local ifaces = { }
		for _, iface in ipairs(self.br.ifnames) do
			ifaces[#ifaces+1] = interface(iface.name)
		end
		return ifaces
	end
end

function interface.bridge_id(self)
	if self.br then
		return self.br.id
	else
		return nil
	end
end

function interface.bridge_stp(self)
	if self.br then
		return self.br.stp
	else
		return false
	end
end

function interface.is_up(self)
	if self.wif then
		return self.wif:is_up()
	else
		return self.dev and self.dev.flags and self.dev.flags.up or false
	end
end

function interface.is_bridge(self)
	return (self:type() == "bridge")
end

function interface.is_bridgeport(self)
	return self.dev and self.dev.bridge and true or false
end

function interface.tx_bytes(self)
	return self.dev and self.dev.stats
		and self.dev.stats.tx_bytes or 0
end

function interface.rx_bytes(self)
	return self.dev and self.dev.stats
		and self.dev.stats.rx_bytes or 0
end

function interface.tx_packets(self)
	return self.dev and self.dev.stats
		and self.dev.stats.tx_packets or 0
end

function interface.rx_packets(self)
	return self.dev and self.dev.stats
		and self.dev.stats.rx_packets or 0
end

function interface.get_network(self)
	return self:get_networks()[1]

end

function interface.get_networks(self)
	if not self.networks then
		local nets = { }
		local _, net
		for _, net in ipairs(_M:get_networks()) do
			if net:contains_interface(self.ifname) or
				net:ifname() == self.ifname
			then

				nets[#nets+1] = net
			end
		end
		table.sort(nets, function(a, b) return a.sid < b.sid end)
		self.networks = nets
		return nets
	else
		return self.networks
	end
end

function interface.get_wifinet(self)
	return self.wif
end


wifidev = utl.class()

function wifidev.__init__(self, dev)
	self.sid    = dev
	self.iwinfo = dev and sys.wifi.getiwinfo(dev) or { }
end

function wifidev.get(self, opt)
	return _get("wireless", self.sid, opt)
end

function wifidev.set(self, opt, val)
	return _set("wireless", self.sid, opt, val)
end

function wifidev.name(self)
	return self.sid
end

function wifidev.hwmodes(self)
	local l = self.iwinfo.hwmodelist
	if l and next(l) then
		return l
	else
		return { b = true, g = true }
	end
end

function wifidev.get_i18n(self)
	local t = "Generic"
	if self.iwinfo.type == "wl" then
		t = "Broadcom"
	elseif self.iwinfo.type == "madwifi" then
		t = "Atheros"
	end

	local m = ""
	local l = self:hwmodes()
	if l.a then m = m .. "a" end
	if l.b then m = m .. "b" end
	if l.g then m = m .. "g" end
	if l.n then m = m .. "n" end

	return "%s 802.11%s Wireless Controller (%s)" %{ t, m, self:name() }
end

function wifidev.is_up(self)
	local up = false

	_uci_state:foreach("wireless", "wifi-iface",
		function(s)
			if s.device == self.sid then
				if s.up == "1" then
					up = true
					return false
				end
			end
		end)

	return up
end

function wifidev.get_wifinet(self, net)
	if _uci_real:get("wireless", net) == "wifi-iface" then
		return wifinet(net)
	else
		local wnet = _wifi_lookup(net)
		if wnet then
			return wifinet(wnet)
		end
	end
end

function wifidev.get_wifinets(self)
	local nets = { }

	_uci_real:foreach("wireless", "wifi-iface",
		function(s)
			if s.device == self.sid then
				nets[#nets+1] = wifinet(s['.name'])
			end
		end)

	return nets
end

function get_wifi_config()
	local nets = { }
	nets["sta"]={ }
	nets["ap"]={ }
	_uci_real:foreach("wireless", "wifi-iface",
		function(s)
			if( s.mode == "sta" ) then
				nets["sta"][#nets["sta"]+1] = s
			else
				nets["ap"][#nets["ap"]+1] = s
			end
		end)

	return nets
end

function round(num, idp)
	if idp and idp>0 then
		local mult = 10^idp
		return math.floor(num * mult + 0.5) / mult
	end
	return math.floor(num + 0.5)
end

function encryption(enc)
	local ent
	if enc == "none" then
		ent = i18n.translate("no encryption")
	elseif enc == "psk" then
		ent = i18n.translate("WPA-PSK")
	elseif enc == "psk2" then
		ent = i18n.translate("WPA2-PSK")
	elseif enc == "psk-mixed" then
		ent = i18n.translate("WPA-PSK/WPA2-PSK mixed mode")
	else
		ent = enc
	end
	return ent
end

function mode_name(m)
	local s
	if     m == "ap"   or  m == "Master" then s = i18n.translate("Access Point (AP)")
	elseif m == "sta"   or  m == "Client"  then s= i18n.translate("Station (STA)")
	elseif m == "adhoc"   then s = i18n.translate("Ad-Hoc")
	elseif m == "mesh"    then s = i18n.translate("Mesh")
	elseif m == "monitor" then s = i18n.translate("Monitor")
	end
	return s
end

function iwinfo_parser()
	local nets = { }
	str=sys.exec("iwinfo")
	i=0
	for line in str:gmatch("[^\r\n]+") do
		if line:find("ESSID: ") then
			i=i+1
			nets[i]= { }
			a,b = line:find("ESSID: ")
			nets[i]["name"]=line:sub(b+2,#line-1)
		elseif line:find("Access Point: ") then
			a,b = line:find("Access Point: ")
			nets[i]["macaddr"]=line:sub(b+1)
		elseif line:find("Link Quality: ") then
			local a
			local b
			a,b = line:find("Link Quality: ")
			quality=line:sub(b+1)
			a,b = quality:find("/")
			a=quality:sub(0,a-1)
			b=quality:sub(b+1)
			if tonumber(a)~=nil and tonumber(b)~=nil then
				nets[i]["signal"]=round(tonumber(a)/tonumber(b)*100)
			else
				nets[i]["signal"]=0
			end
		elseif line:find("Encryption: ") then
			a,b = line:find("Encryption: ")
			nets[i]["encryption"]=encryption(line:sub(b+1))
		elseif line:find("Mode: ") then
			a,b = line:find("Mode: ")
			nets[i]["mode"]=mode_name(line:sub(b+1,b+6))
		elseif line:find("Bit Rate: ") then
			a,b = line:find("Bit Rate: ")
			if line:sub(b+1) == "unknown" then
				nets[i]["bitrate"]="N/A"
			else
				nets[i]["bitrate"]=line:sub(b+1)
			end
		end
	end
	return nets
end

function wifidev.add_wifinet(self, options)
	options = options or { }
	options.device = self.sid

	local wnet = _uci_real:section("wireless", "wifi-iface", nil, options)
	if wnet then
		return wifinet(wnet, options)
	end
end

function wifidev.del_wifinet(self, net)
	if utl.instanceof(net, wifinet) then
		net = net.sid
	elseif _uci_real:get("wireless", net) ~= "wifi-iface" then
		net = _wifi_lookup(net)
	end

	if net and _uci_real:get("wireless", net, "device") == self.sid then
		_uci_real:delete("wireless", net)
		return true
	end

	return false
end


wifinet = utl.class()

function wifinet.__init__(self, net, data)
	self.sid = net

	local num = { }
	local netid
	_uci_real:foreach("wireless", "wifi-iface",
		function(s)
			if s.device then
				num[s.device] = num[s.device] and num[s.device] + 1 or 1
				if s['.name'] == self.sid then
					netid = "%s.network%d" %{ s.device, num[s.device] }
					return false
				end
			end
		end)

	local dev = _uci_state:get("wireless", self.sid, "ifname") or netid
	if dev == "radio0.network1" then
		dev="wlan0"
	elseif dev == "radio0.network2" then
		dev = "wlan0-1"
	end

	self.netid  = netid
	self.wdev   = dev

	self.iwinfo = dev and sys.wifi.getiwinfo(dev) or { }
	self.iwdata = data or _uci_state:get_all("wireless", self.sid) or
		_uci_real:get_all("wireless", self.sid) or { }
end

function wifinet.get(self, opt)
	return _get("wireless", self.sid, opt)
end

function wifinet.set(self, opt, val)
	return _set("wireless", self.sid, opt, val)
end

function wifinet.mode(self)
	return _uci_state:get("wireless", self.sid, "mode") or "ap"
end

function wifinet.disabled(self)
	return _set("wireless", self.sid, "disabled")
end

function wifinet.ifdisabled(self)
        return _get("wireless", self.sid, "disabled")
end

function wifinet.ssid(self)
	return _uci_state:get("wireless", self.sid, "ssid")
end

function wifinet.bssid(self)
	return _uci_state:get("wireless", self.sid, "bssid")
end

function wifinet.network(self)
	return _uci_state:get("wifinet", self.sid, "network")
end

function wifinet.id(self)
	return self.netid
end

function wifinet.name(self)
	return self.sid
end

function wifinet.ifname(self)
	local ifname = self.iwinfo.ifname
	if not ifname or ifname:match("^wifi%d") or ifname:match("^radio%d") then
		ifname = self.wdev
	end
	return ifname
end

function wifinet.get_device(self)
	if self.iwdata.device then
		return wifidev(self.iwdata.device)
	end
end

function wifinet.is_up(self)
	return (self.iwdata.up == "1")
end

function wifinet.active_mode(self)
	local m = _uci_state:get("wireless", self.sid, "mode") or "ap"
	if     m == "ap"   or  m == "Master" then m = "Access Point (AP)"
	elseif m == "sta"   or  m == "Client"  then m = "Station (STA)"
	elseif m == "adhoc"   then m = "Ad-Hoc"
	elseif m == "mesh"    then m = "Mesh"
	elseif m == "monitor" then m = "Monitor"
	end

	return m
end

function wifinet.active_mode_i18n(self)
	return i18n.translate(self:active_mode())
end

function wifinet.active_ssid(self)
	return _stror(self.iwinfo.ssid, self.iwdata.ssid)
end

function wifinet.active_bssid(self)
	return _stror(self.iwinfo.bssid, self.iwdata.bssid) or "00:00:00:00:00:00"
-- 	local bssid = sys.exec("cat /sys/class/net/wlan0/address")
-- 	return bssid or "00:00:00:00:00:00"
end

function wifinet.active_encryption(self)
-- 	local enc = self.iwinfo and self.iwinfo.encryption
-- 	return enc and enc.description or "-"
	local ent
	local enc = _uci_state:get("wireless", self.sid, "encryption")
	if enc == "none" then
		ent = "No Encryption"
	elseif enc == "psk" then
		ent = "WPA-PSK"
	elseif enc == "psk2" then
		ent = "WPA2-PSK"
	elseif enc == "psk-mixed" then
		ent = "WPA-PSK/WPA2-PSK Mixed Mode"
	else
		ent = enc
	end
	return ent
end

function wifinet.assoclist(self)
	return self.iwinfo.assoclist or { }
end

function wifinet.frequency(self)

-- 	local channel
-- 	channel = tonumber(_uci_state:get("wireless", self.iwdata.device, "channel"))
-- 	if channel ==  1 then freq = 2412
-- 	elseif channel ==  2 then freq = 2417
-- 	elseif channel ==  3 then freq = 2422
-- 	elseif channel ==  4 then freq = 2427
-- 	elseif channel ==  5 then freq = 2432
-- 	elseif channel ==  6 then freq = 2437
-- 	elseif channel ==  7 then freq = 2442
-- 	elseif channel ==  8 then freq = 2447
-- 	elseif channel ==  9 then freq = 2452
-- 	elseif channel == 10 then freq = 2457
-- 	elseif channel == 11 then freq = 2462
-- 	end
	local freq = self.iwinfo.frequency
 	if freq and freq > 0 then
	  return "%.03f" % (freq / 1000)
 	end
end

function wifinet.bitrate(self)
	local rate = self.iwinfo.bitrate
	if rate and rate > 0 then
		return (rate / 1000)
	end
end

function wifinet.channel(self)
	return self.iwinfo.channel or
		tonumber(_uci_state:get("wireless", self.iwdata.device, "channel"))
end

function wifinet.signal(self)
	return self.iwinfo.signal or 0
end

function wifinet.noise(self)
	return self.iwinfo.noise or 0
end

function wifinet.country(self)
	--return self.iwinfo.country or "00"
	--workaround
	return _uci_state:get("wireless", self.iwdata.device, "country") or "00"
end

function wifinet.hwmode(self)
	--return self.iwinfo.country or "00"
	--workaround
	return _uci_state:get("wireless", self.iwdata.device, "hwmode") or "Auto"
end

function wifinet.countryname(self)
	local cl = self.iwinfo and self.iwinfo.countrylist
	local countryname

	if cl and #cl > 0 then
		for _, c in ipairs(cl) do
			if c.alpha2 == self.iwinfo.country then
				countryname = c.name
				break
			end
		end
	end

	return countryname or "World"
end

function wifinet.txpower(self)
	local pwr = (self.iwinfo.txpower or 0)
	return pwr + self:txpower_offset()
end

function wifinet.txpower_offset(self)
	return self.iwinfo.txpower_offset or 0
end

function wifinet.signal_level(self, s, n)
	if self:active_bssid() ~= "00:00:00:00:00:00" then
		local signal = s or self:signal()
		local noise  = n or self:noise()

		if signal < 0 and noise < 0 then
			local snr = -1 * (noise - signal)
			return math.floor(snr / 5)
		else
			return 0
		end
	else
		return -1
	end
end

function wifinet.signal_percent(self)
	local qc = self.iwinfo.quality or 0
	local qm = self.iwinfo.quality_max or 0

	if qc > 0 and qm > 0 then
		return math.floor((100 / qm) * qc)
	else
		return 0
	end
end

function wifinet.shortname(self)
	return "%s %q" %{
		i18n.translate(self:active_mode()),
		self:active_ssid() or self:active_bssid()
	}
end

function wifinet.get_i18n(self)
	return "%s: %s %q (%s)" %{
		i18n.translate("Wireless Network"),
		i18n.translate(self:active_mode()),
		self:active_ssid() or self:active_bssid(),
		self:ifname()
	}
end

function wifinet.adminlink(self)
	return dsp.build_url("admin", "network", "wireless", self.netid)
end

function wifinet.get_network(self)
	return self:get_networks()[1]
end

function wifinet.get_networks(self)
	local nets = { }
	local net
	for net in utl.imatch(tostring(self.iwdata.network)) do
		if _uci_real:get("network", net) == "interface" then
			nets[#nets+1] = network(net)
		end
	end
	table.sort(nets, function(a, b) return a.sid < b.sid end)
	return nets
end

function wifinet.get_interface(self)
	return interface(self:ifname())
end


-- setup base protocols
_M:register_protocol("static")
_M:register_protocol("dhcp")
_M:register_protocol("none")
_M:register_protocol("ndis")
_M:register_protocol("qmi")
_M:register_protocol("qmi2")
_M:register_protocol("ncm")

-- load protocol extensions
local exts = nfs.dir(utl.libpath() .. "/model/network")
if exts then
	local ext
	for ext in exts do
		if ext:match("%.lua$") then
			require("luci.model.network." .. ext:gsub("%.lua$", ""))
		end
	end
end

--
-- Teltonika extensions
-- Abandon all hope, ye who enter here
--

function get_override()
	local bridge_mode = _uci_real:get("network", "ppp", "method")
	local bridge_on = false

	if bridge_mode and bridge_mode == "bridge" or bridge_mode == "pbridge" then
		bridge_on = true
	end

	local a = not _uci_real:get("network", "wan", "ifname") and not bridge_on or _uci_real:get("network", "wan", "ifname") == "wlan0" or false
	return a
end

function get_wwan()
	local a = _uci_real:get("network", "wwan")
	if a then
		return true
	else
		return false
	end
end

function get_sta_av(self)
	local a, b
	_uci_real:foreach("wireless", "wifi-iface",
		function(s)
			if s.mode == "sta" then
				a = s.ssid
				b = s.disabled
			end
		end)
	return a, b
end

function backupLink_status(self)
	local mEn = _get("multiwan", "config", "enabled")
	if mEn == "1" then
		return true
	end
	return false
end

function backupLink_en(self)
	return _uci_real:set("multiwan", "config", "enabled", "1")
end

function backupLink_dis(self) --angry backup link will dis your hood
	return _uci_real:set("multiwan", "config", "enabled", "0")
end

function backupLink_3gBackup(self, enable) -- true - we will attempt to enable 3g on wan2; false - we will attempt to delete wan2 network
	if enable then
		local opts = {
			proto	= "dhcp",
			ifname	= "usb0"
		}
		self:add_network("wan2", opts)
	else
		self:del_network("wan2")
	end
end

function get_module(self)
	return _uci_real:get("system", "module", "type")
end

function get_module_fname(self)
	return _uci_real:get("system", "module", "iface")
end

function get_wan_ifname(self)
	return _uci_real:get("network", "wan", "ifname")
end

--
--3g status utility
--
function get3g_all()
	local allData = getInfo_3g(self, {call = "all"})
	if allData == nil then return nil end
	local dataTable = utl.split(allData, "\n")
	local returnTable = {}
	local line
	for i,line in ipairs(dataTable) do
		if line:match("State") then
			returnTable["state"] = utl.trim(utl.split(line, "=")[2])
		end
		if line:match("Signal") then
			returnTable["signal"] = utl.trim(utl.split(line, "=")[2])
		end
		if line:match("Network name") then
			returnTable["oper"]  = utl.trim(utl.split(line, "=")[2])
		end
		if line:match("Bearer") then
			returnTable["ntype"] = utl.trim(utl.split(line, "=")[2])
		end
		if line:match("Imei") then
			returnTable["imei"] = utl.trim(utl.split(line, "=")[2])
		end
		if line:match("TX bytes") then
			returnTable["txbytes"] = utl.trim(utl.split(line, "=")[2])
		end
		if line:match("RX bytes") then
			returnTable["rxbytes"] = utl.trim(utl.split(line, "=")[2])
		end
	end

	return returnTable
end
function callSucc(self, str)
	if str == nil or str.str == nil then return false end
	if str.str:match("operation failed") or str.str:match("error") then return false else return true end
end

function get_openvpn()
	local uci = uci.cursor()
	local string = require "string"

	function hex_to_string(str)
		return (str:gsub('..', function (cc)
			return string.char(tonumber(cc, 16))
		end))
	end

	tablenew = {}
	returnTable = {}

	local openvpns = ""

	uci:foreach("openvpn", "openvpn", function(s)
		if not string.match(s[".name"], "teltonika_auth_service") then
			openvpns = openvpns .. s[".name"] .. "\n"
		end
	end)

	local openvpn_fullname_string

	for openvpn_fullname in openvpns:gmatch("[^\r\n]+") do

		openvpns_status = ""
		openvpn_name = ""
		openvpn_type = ""
		openvpns_dev = ""
		openvpns_mask = ""
		openvpns_ip = ""
		openvpns_time = ""

		local is_hexed = uci:get("openvpn", openvpn_fullname, "name_is_hexed") or "0"

		if is_hexed ~= "0" then
			openvpn_fullname_string = hex_to_string(openvpn_fullname)
		else
			openvpn_fullname_string = openvpn_fullname
		end

		openvpn_type, openvpn_name = openvpn_fullname_string:match("([^,]+)_([^,]+)")
		openvpns_dev = uci:get("openvpn", openvpn_fullname, "dev") or ""

		local status_file_path = "/tmp/openvpn-" .. openvpn_fullname_string .. ".status"
		local openvpn_status_log = nfs.readfile(status_file_path, 65536)
		local is_connected = false
		local enabled = uci:get("openvpn", openvpn_fullname, "enable")

		-- If the server is not running the status file will share the same format as the clients,
		-- therefore we can simply check whether the line TCP/UDP read bytes, 0 exists to
		-- determine it's connection state.
		if openvpn_status_log and not string.match(openvpn_status_log, "TCP/UDP read bytes,0")
			and #openvpn_status_log > 0 and enabled == "1" then
			is_connected = true
		end

		openvpns_status	= "Disconnected"

		if openvpns_dev == "tap" then
			openvpns_status = "Bridge"
			started = utl.trim(sys.exec("cat /tmp/state/ifuptime.log 2>/dev/null "
				.. "| grep -w tap0 | awk -F'=' '{print $2}' "))

			now = utl.trim(sys.exec("cat /proc/uptime | awk -F'.' '{print $1}'"))

			if started and tonumber(started) then
				openvpns_time = tonumber(now) - tonumber(started)
			else
				openvpns_time = 0
			end
		elseif is_connected then

			openvpns_status = (openvpn_type == "server" and "Running" or "Connected")
			local openvpn_custom_enb = uci:get("openvpn", openvpn_fullname_string,
				"enable_custom") or "0"

			if openvpn_custom_enb == "0" then

				openvpns_ip = utl.trim(sys.exec("ifconfig " .. openvpns_dev
					.. " 2>/dev/null | grep 'inet addr:' | awk -F':' '{print $2}' "
					.. "| awk -F' ' '{print $1}'"))
				openvpns_mask = utl.trim(sys.exec("ifconfig " .. openvpns_dev
					.. " 2>/dev/null | grep 'Mask:' | awk -F':' '{print $4}'"))
				started = utl.trim(sys.exec("cat /tmp/state/ifuptime.log 2>/dev/null "
					.. "| grep -w " .. openvpns_dev .. "| awk -F'=' '{print $2}' "))

				if not tonumber(started) then
					started = 0;
				end

				now = utl.trim(sys.exec("cat /proc/uptime | awk -F'.' '{print $1}'"))
				openvpns_time = tonumber(now) - tonumber(started)
			else
				openvpns_ip = "Not available in custom configuration mode"
				openvpns_mask = "Not available in custom configuration mode"
				openvpns_time = "Not available in custom configuration mode"
			end
		end

		tabletest = {
			openvpn_fullname,
			openvpns_status,
			openvpn_type,
			openvpns_ip,
			openvpns_mask,
			openvpns_time,
		}
		table.insert(tablenew, tabletest)
		tabletest = {}

		if openvpn_type == "server" then

			local name
			local raddress
			local vaddress
			local consinc
			local sinc
			local n  = 0
			local a  = 0
			local i = 1
			local com = 0
			tableconsinc = {}
			local file = utl.trim(sys.exec("cat ".. status_file_path .." 2>/dev/null"))

			for line in file:gmatch("[^\r\n]+") do

				name = ""
				raddress = ""
				vaddress = ""
				consinc = ""

				if line == "GLOBAL STATS" or line == "ROUTING TABLE" then
					n = 0
					a = 0
				end

				if n == 2 then
					a = a + 1
					name, raddress, rec, send, consinc = line:match("([^,]+),([^,]+),([^,]+),([^,]+),([^,]+)")
					local tabletest = {
						a,
						consinc
					}
					table.insert(tableconsinc, tabletest)
					tabletest = {}
					com  = a
				end

				if n == 1 then
					a = a + 1

					if a <= com then
						vaddress, name, raddress, consinc = line:match("([^,]+),([^,]+),([^,]+),([^,]+)")
						sinc = tableconsinc[a][2]
						local tabletest = {
							name,
							raddress,
							vaddress,
							sinc
						}

						table.insert(tablenew,tabletest )
						tabletest = {}

					end

				end

				if line == "Common Name,Real Address,Bytes Received,Bytes Sent,Connected Since" then
					n = 2
					a = 0
				end

				if line == "Virtual Address,Common Name,Real Address,Last Ref" then
					n = 1
					a = 0
				end
			end
		end
		table.insert(returnTable, tablenew)
		tablenew = {}
	end
	return returnTable
end

function get_services()
	local _uci_real  = cursor or _uci_real or uci.cursor()
	tablenew = {}
	returnTable = {}
	---------------------VRRP[0]---------------------------------
	local vid1_enabled = "0"
	--local vid2_enabled = utl.trim(sys.exec("uci get vrrpd.vid2.enabled 2>/dev/null")) or "0"
	--tablenew = { vid1_enabled, vid2_enabled, ping_enabled }
	_uci_real:foreach("vrrpd", "vrrpd", function(s)
		if s.enabled == "1" then
			vid1_enabled = "1"
		end
	end)
	tablenew = { vid1_enabled, ping_enabled }
	table.insert(returnTable, tablenew)
	---------------------OpenVPN[1]---------------------------------
	local openvpn_server = "0"
	local openvpn_client = "0"

	_uci_real:foreach("openvpn", "openvpn", function(s)
		if s._role == "server" and s.enable == "1" then
			openvpn_server = "1"
		elseif s._role == "client" and s.enable == "1" then
			openvpn_client = "1"
		end
	end)
	tablenew = { openvpn_server, openvpn_client }
	table.insert(returnTable, tablenew)
	---------------------IPsec[2]---------------------------------
	local ipsec_enabled = utl.trim(sys.exec("uci show strongswan | grep -w \"enabled='1'\" | wc -l")) or "0"
	local keepalive_enabled = "0"
	tablenew = { ipsec_enabled, keepalive_enabled }
	table.insert(returnTable, tablenew)
	---------------------NTP[3]---------------------------------
	local time = utl.trim(sys.exec("date"))
	local ntpclient = utl.trim(sys.exec("uci get -q ntpclient.@ntpclient[0].enabled")) or "0"
	tablenew = { time, ntpclient }
	table.insert(returnTable, tablenew)
	---------------------SNMP[4]---------------------------------
	local snmpd_agent = utl.trim(sys.exec("uci get -q snmpd.@agent[0].enabled")) or "0"
	local snmpd_trap = utl.trim(sys.exec("uci get -q snmpd.@trap[0].trap_enabled")) or "0"
	tablenew = { snmpd_agent, snmpd_trap }
	table.insert(returnTable, tablenew)
	---------------------DDNS[5]--------------------------------
	local ddns_service = utl.trim(sys.exec("uci show ddns | grep -w \"enabled='1'\" | wc -l")) or "0"
	tablenew = { ddns_service }
	table.insert(returnTable, tablenew)
	---------------------GPS[6]--------------------------------
	local gps_service = utl.trim(sys.exec("uci -q get gps.gpsd.enabled")) or "0"
	tablenew = { gps_service}
	table.insert(returnTable, tablenew)
	---------------------Hotspot[7]---------------------------------
	local hotspot = utl.trim(sys.exec("uci -q get coovachilli.hotspot1.enabled")) or "0"
	local ftp = utl.trim(sys.exec("uci -q get tcplogger.general.enabled")) or "0"
	tablenew = { hotspot, ftp }
	table.insert(returnTable, tablenew)
	---------------------Ping Reboot[8]---------------------------------
	local ping_reboot = utl.trim(sys.exec("uci show ping_reboot | grep -w \"enable='1'\" | wc -l")) or "0"
	tablenew = { ping_reboot }
	table.insert(returnTable, tablenew)
	---------------------GRE Tunnel[9]---------------------------------
	local gre_enable = "0"
	_uci_real:foreach("network", "interface", function(s)
		if s["proto"] == "gre" then
			if s["enabled"] == "0" then
				gre_enable = "0"
			else
				gre_enable = "1"
			end
		end
	end)
	tablenew = { gre_enable }
	table.insert(returnTable, tablenew)
	---------------------Input/output[10]---------------------------------
	local ioman = utl.trim(sys.exec("uci show ioman | grep -w \"enabled='1'\" | wc -l")) or "0"
	tablenew = { ioman }
	table.insert(returnTable, tablenew)
	---------------------SMS_utils[11]---------------------------------
	local sms = "0"
	_uci_real:foreach("sms_utils", "rule", function(s)
		if s.action ~= "get_configure" and s.action ~= "set_configure" then
			if s.enabled and s.enabled == "1" then
				sms = "1"
			end
		end
	end)
	tablenew = { sms }
	table.insert(returnTable, tablenew)
	---------------------Firewall[12]--------------------------------
	local privoxy = utl.trim(sys.exec("uci -q get privoxy.privoxy.enabled")) or "0"
	local hostblock = utl.trim(sys.exec("uci -q get hostblock.config.enabled")) or "0"
	tablenew = { privoxy, hostblock }
	table.insert(returnTable, tablenew)
	---------------------QoS[13]-------------------------------------
	local qos = utl.trim(sys.exec("uci show qos | grep -w \"enabled='1'\" | wc -l")) or "0"
	tablenew = { qos }
	table.insert(returnTable, tablenew)

	return returnTable
end

function get_rx_tx_info()
	local rx, tx, fd
	local file_name = "/sys/class/net/wwan0/statistics/"
	fd = io.open(file_name .. "rx_bytes")
	if fd ~= nil then
		rx = fd:read("*all")
		fd:close()
	end
	fd = io.open(file_name .. "tx_bytes")
	if fd ~= nil then
		tx = fd:read("*all")
		fd:close()
	end
	return { ["RX"] = (rx and rx or 0), ["TX"] = (tx and tx or 0) }
end

function file_exists(file_name)
	local fd = io.open(file_name)
	if fd ~= nil then
		fd:close()
		return file_name
	end
	return nil
end

function read_sim_file(file_name)
	local fd = io.open(file_name)
	if fd ~= nil then
		local file_contents = fd:read("*all")
		fd:close()
		file_contents = file_contents:split(",")
		return { ["RX"] = tonumber(file_contents[1]),
			["TX"] =  tonumber(file_contents[2]) }
	end
	return { ["RX"] = 0, ["TX"] = 0 }
end

function write_sim_file(file_name, rx, tx)
	local fd = io.open(file_name, "w")
	if fd ~= nil then
		fd:write(rx .. "," .. tx)
		fd:close()
	end
end

function update_sim_data(current_sim, file_name)
	local data_used = get_rx_tx_info()
	local other_sim = ( current_sim == "2" and "1" or "2" )
		if file_exists(file_name) ~= nil then
		local sub = read_sim_file("/tmp/sim" .. other_sim .. "_stats")
		local rx = data_used["RX"] - ( sub["RX"] and sub["RX"] or 0 )
		local tx = data_used["TX"] - ( sub["TX"] and sub["TX"] or 0 )
		write_sim_file(file_name, rx, tx)
	end
end

function get_sim_data_used()
	local current_sim = utl.trim(sys.exec("/sbin/gpio.sh get SIM")) or "1"
	current_sim = current_sim == "0" and "2" or "1"
	local file_name = "/tmp/sim" .. current_sim .. "_stats"
	update_sim_data(current_sim, file_name)
	return read_sim_file(file_name)
end

function new_get_info()

	local string = require "string"
	local uci  = cursor or _uci_real or uci.cursor()
	local intf
	local bridge_mode = uci:get("network", "ppp", "method")
	local bridge_on = false
	local sim_data_usage = { }

	if bridge_mode and bridge_mode == "bridge" or bridge_mode == "pbridge" then
		bridge_on = true
	end
	local data_want = {"netstate", "state", "signal","oper", "ntype", "imei", "imsi", "iccid", "txbytes", "rxbytes", "simstate", "pinstate",
					   "cellid", "rscp", "ecio", "rsrp", "rsrq", "sinr", "pinleft", "bands"}
	local returnTable = {}

	if bridge_on then
		intf = utl.trim(sys.exec("uci get -q network.ppp.ifname"))
	else
		intf = utl.trim(sys.exec("uci get -q system.module.iface"))
	end

	command="gsmctl -gjqotixJ -e "..intf.." -r "..intf.." -zu -C -X -E -W -M -Z -Bn -F" --for all modules the same command

	data = sys.exec(command)
	dataTable = utl.split(data, "\n")

	for key,value in pairs(data_want) do
		returnTable[value] = dataTable[key]
	end

	if intf == "wwan0" then
		sim_data_usage = get_sim_data_used()
		returnTable["rxbytes"] = sim_data_usage["RX"] and sim_data_usage["RX"] or 0
		returnTable["txbytes"] = sim_data_usage["TX"] and sim_data_usage["TX"] or 0
	end

	return returnTable
end

function usb_modem_info()

	local string = require "string"
	local uci  = cursor or _uci_real or uci.cursor()
	--local bridge_mode = uci:get("network", "ppp", "method")
	--local bridge_on = false
	--local intf

	--[[
	if bridge_mode and bridge_mode == "bridge" or bridge_mode == "pbridge" then
		bridge_on = true
	end
	]]--

	local data_want = { "netstate", "state", "signal", "oper", "ntype", "imei", "imsi", "iccid", "txbytes", "rxbytes", "simstate", "pinstate", "cellid", "rscp", "ecio", "rsrp", "rsrq", "sinr", "pinleft" }
	local returnTable = {}

	--[[
	if bridge_on then
		intf = utl.trim(sys.exec("uci get -q network.ppp.ifname"))
	else
		intf = utl.trim(sys.exec("uci get -q system.module.iface"))
	end
	]]--

	command="gsmctl -gjqotixJ -e 'wwan-usb0' -r 'wwan-usb0' -zu -C -X -E -W -M -Z -Bn"
	data = sys.exec(command)
	dataTable = utl.split(data, "\n")

	for key,value in pairs(data_want) do
		returnTable[value] = dataTable[key]
	end

	return returnTable
end


function get_netw_type()
	--gsmctl -Ugjqoti -e eth1 -r eth1 -zu -A AT^CPIN?
	local string = require "string"
	local uci  = cursor or _uci_real or uci.cursor()
	command="gsmctl -tn" --For all modules the same command

	data = sys.exec(command)
	return data
end

--
--WiMAX utility
--
local wimaxPass = utl.trim(sys.exec("uci -q get teltonika.sys.WiMAXpass"))
if wimaxPass == nil or wimaxPass == "" then
	wimaxPass = "user" -- default password for WiMAX usb stick
end
local cgiString = "http://192.168.0.1/cgi/"
local curlParams= "/usr/bin/curl -i -m 5 --user \"user:" .. wimaxPass .. "\" --anyauth "

function wimaxCGICall(self, cgiEnd)
	local str = sys.exec(curlParams .. cgiString .. cgiEnd.call)
	local tmp
	local i = 0
	local return_value
	for line in str:gmatch("[^\n]+") do
		tmp = line:match("HTTP/1.0 (%d+)")
		if tmp ~= nil and tmp ~= "200" and i ~= 0 then
			return tmp
		end
		return_value = line
		i = i + 1
	end
	return return_value
end
function rr_wimax()
	wimaxCGICall(self, { call = "reboot" })
	return true
end

function check_wimax_state()
	if wimaxCGICall(self, { call = "state" }) == "Radio disabled" then
		return false
	end
	return true
end

--
-- TELTONIKA-GCT Wimax utility
--
function gct_wimax_syscall(self)
	local Buffer = sys.exec("/etc/gct/gctconnect cm_gl")
	local StatusBuffer = sys.exec("/etc/gct/gctconnect cm_gs")

	if Buffer == nil or Buffer == "" or StatusBuffer == nil or StatusBuffer == "" then
		return nil
	end

	local DataTable = utl.split(Buffer, "\n")
	local ReturnTable = {}
	local Line

	for i,Line in ipairs(DataTable) do
		if Line:match("RSSI:") then
			ReturnTable["signal_strength"] = utl.trim(utl.split(Line, ":")[2])
		end
		if Line:match("CINR:") then
			ReturnTable["signal_quality"] = utl.trim(utl.split(Line, ":")[2])
		end
		if Line:match("BSID:") then
			ReturnTable["bsid"] = utl.trim(utl.split(Line, ":")[2])
			ReturnTable["bsid"] = ReturnTable["bsid"]:gsub("%s+", ":")
		end
		if Line:match("UL burst data FEC scheme:") then
			ReturnTable["uplink_coding"] = utl.trim(utl.split(Line, ":")[2])
		end
		if Line:match("DL burst data FEC scheme:") then
			ReturnTable["downlink_coding"] = utl.trim(utl.split(Line, ":")[2])
		end
	end

	ReturnTable["state"] = utl.trim(utl.split(StatusBuffer, "\n")[1])
	ReturnTable["state"] = utl.trim(utl.split(ReturnTable["state"], " ")[3])
	ReturnTable["state"] = ReturnTable["state"]:gsub("%[", "")
	ReturnTable["state"] = ReturnTable["state"]:gsub("%]", "")

	ReturnTable["macaddress"] = sys.exec("ifconfig wm0 | awk \'/HWaddr/ {print $5}\'")

	return ReturnTable
end

function ro_wimax() --radio on/off for the wimax dongeli
	radioEnabled = check_wimax_state()
	if radioEnabled then
		wimaxCGICall(self, { call = "disable-radio" })
	else
		wimaxCGICall(self, { call = "enable-radio" })
	end
	return true
end

-- Base64-encoding
-- Sourced from http://en.wikipedia.org/wiki/Base64
require('math')


local __author__ = 'Daniel Lindsley'
local __version__ = 'scm-1'
local __license__ = 'BSD'


local index_table = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'


function to_binary(integer)
    local remaining = tonumber(integer)
    local bin_bits = ''

    for i = 7, 0, -1 do
        local current_power = math.pow(2, i)

        if remaining >= current_power then
            bin_bits = bin_bits .. '1'
            remaining = remaining - current_power
        else
            bin_bits = bin_bits .. '0'
        end
    end

    return bin_bits
end

function from_binary(bin_bits)
    return tonumber(bin_bits, 2)
end


function to_base64(to_encode)
    local bit_pattern = ''
    local encoded = ''
    local trailing = ''

    for i = 1, to_encode:len() do
        bit_pattern = bit_pattern .. to_binary(to_encode:sub(i, i):byte())
    end

    -- Check the number of bytes. If it's not evenly divisible by three,
    -- zero-pad the ending & append on the correct number of ``=``s.
    if math.mod(bit_pattern:len(), 3) == 2 then
        trailing = '=='
        bit_pattern = bit_pattern .. '0000000000000000'
    elseif math.mod(bit_pattern:len(), 3) == 1 then
        trailing = '='
        bit_pattern = bit_pattern .. '00000000'
    end

    for i = 1, bit_pattern:len(), 6 do
        local byte = bit_pattern:sub(i, i+5)
        local offset = tonumber(from_binary(byte))
        encoded = encoded .. index_table:sub(offset+1, offset+1)
    end

    return encoded:sub(1, -1 - trailing:len()) .. trailing
end
