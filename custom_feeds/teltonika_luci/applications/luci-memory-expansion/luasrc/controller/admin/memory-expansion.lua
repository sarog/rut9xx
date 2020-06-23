module("luci.controller.admin.memory-expansion", package.seeall)

function index()
	local sys = require "luci.sys"
	local utl = require "luci.util"
	local usb = utl.trim(luci.sys.exec("uci get hwinfo.hwinfo.usb"))
	if usb == "1" then
		entry({"admin", "system", "memory-expansion"}, template("memory-expansion"), _("Memory expansion"), 9)
		entry({"admin", "system", "memory-expansion", "expand"}, call("expand")).leaf = true
		entry({"admin", "system", "memory-expansion", "shrink"}, call("shrink")).leaf = true
	end
end

-- Copied and pasted from memory-expansion.htm; find a better way
local MINIMUM_USB_PARTITION_SIZE_IN_1K_BLOCKS = 524288
local function is_expanded()
	local cmd = 'block info | grep -o \'^/dev/sda1: .*MOUNT="/overlay" .*TYPE="ext2".*$\''
	local cmdoutput = luci.sys.exec(cmd)
	return cmdoutput ~= nil and #cmdoutput > 9
end
local function get_sda1_size()
	local cmd = 'block info | grep -o \'^/dev/sda1: .*MOUNT="/mnt/sda1" .*TYPE="ext2".*$\''
	local cmdoutput = luci.sys.exec(cmd)
	local mounted = cmdoutput ~= nil and #cmdoutput > 9
	local mountreturncode = 0
	if not mounted then
		luci.sys.exec("mkdir -p /mnt/sda1")
		mountreturncode = luci.sys.call("mount -t ext2 /dev/sda1 /mnt/sda1")
	end
	if mountreturncode ~= 0 then
		return "0"
	end
	local size = luci.sys.exec("df | grep -o '^/dev/sda1[ ]*[0-9]*' | grep -o [0-9]*$")
	if not mounted then
		luci.sys.exec("umount /mnt/sda1")
	end
	return tonumber(size) ~= nil and luci.util.trim(size) or "0"
end
local function get_case()
	if is_expanded() then
		return 1
	else
		if luci.sys.exec("ls -1 /dev | grep -o '^sda$'") ~= "sda\n" then
			return 2
		elseif luci.sys.exec("ls -1 /dev | grep -o '^sda1$'") ~= "sda1\n" then
			return 3
		elseif luci.sys.exec("block info | grep '^/dev/sda1: .*TYPE=\"ext2\".*$' | grep -o 'TYPE=\"ext2\"'") ~= 'TYPE="ext2"\n' then
			return 4
		elseif tonumber(get_sda1_size()) < MINIMUM_USB_PARTITION_SIZE_IN_1K_BLOCKS then
			return 5
		else
			return 6
		end
	end
end


function expand()
	local case = get_case()
	if case == 6 then
		luci.sys.exec(
		    "uci show samba | grep mnt/sda1 | grep -o '\\[.*\\]' | grep -o [0-9]* | sort -nr | sed 's/^/uci delete samba.@sambashare[/' | sed 's/$/]/' | sh ; "
		 .. "uci commit ; "
		 .. "block detect > /etc/config/fstab ; "
		 .. "sed -i s#/mnt/sda1#/overlay# /etc/config/fstab ; "
		 .. "sed -i s/option$'\\t'enabled$'\\t'\\'0\\'/option$'\\t'enabled$'\\t'\\'1\\'/ /etc/config/fstab ; "
		 .. "mount -t ext2 /dev/sda1 /mnt/sda1 ; rm -rf /mnt/sda1/* ; "
		 .. "rm -rf /overlay/upper/mnt/sda1/* ; "
		 .. "tar -C /overlay -cvf - . | tar -C /mnt/sda1 -xf -")
	end
end

function shrink()
	local case = get_case()
	if case == 1 then
		luci.sys.exec(
		    "umount /overlay ; "
		 .. "mount -t jffs2 /dev/mtdblock5 /overlay ; "
		 .. "rm /overlay/upper/etc/config/fstab")
	end
end
