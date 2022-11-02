#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fmt-usb-msd
PKG_VERSION:=2020-11-19
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	DEPENDS:=+fstools +libubox
	TITLE:=USB Mass Storage Device formatting utility
	SUBMENU:=Filesystem
endef

define Package/$(PKG_NAME)/description
	Formats USB Mass Storage Devices to NTFS or EXT2
endef



define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin $(1)/etc/hotplug.d/block
	$(INSTALL_BIN) ./files/$(PKG_NAME).sh $(1)/bin/
	$(INSTALL_BIN) ./files/$(PKG_NAME)_last.sh $(1)/etc/hotplug.d/block/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
