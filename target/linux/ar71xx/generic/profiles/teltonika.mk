#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/RUT900
	NAME:=Teltonika RUT900
	PACKAGES:=kmod-usb-core kmod-usb2 kmod-ledtrig-usbdev
endef

define Profile/RUT900/Description
	Package set optimized for the Teltonika RUT900.
endef
$(eval $(call Profile,RUT900))

