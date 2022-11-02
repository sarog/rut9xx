#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=storage-memory-expansion
PKG_VERSION:=2020-07-08
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	DEPENDS:=+fstools +fmt-usb-msd
	TITLE:=Storage Memory Expansion script
	SUBMENU:=Filesystem
endef

define Package/$(PKG_NAME)/description
	Sets up overlay on a USB Mass Storage Device
endef



define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) ./files/sme.sh $(1)/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
