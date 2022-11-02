#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=generic-container
PKG_VERSION:=2020-12-23
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=7f664f9829db30dece593922016d634780a0dc05
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/generic-container
	SECTION:=base
	CATEGORY:=Base system
	TITLE:= Generic temporary storage ubus powered container
	DEPENDS:=+rpcd +libblobmsg-json +libubus +libubox +libsqlite3
endef

define Package/generic-container/description
	Network operators control process to manage whitelist/blacklist, band, auto APN, etc..
endef


define Package/generic-container/install
	$(INSTALL_DIR) $(1)/usr/lib/rpcd/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gcont.so $(1)/usr/lib/rpcd/
endef
$(eval $(call BuildPackage,generic-container))
