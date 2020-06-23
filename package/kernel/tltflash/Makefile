#
# Copyright (C) 2016 Teltonika
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=tltflash
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/tltflash
  SUBMENU:=Other modules
  TITLE:=Driver for upgrading Telit module's fw
  DEPENDS:=kmod-usb-core
  FILES:=$(PKG_BUILD_DIR)/tltflash.ko
endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,tltflash))
