#
# Copyright (C) 2022 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=libtltchilli
PKG_VERSION:=2021-12-15
PKG_RELEASE:=2

PKG_SOURCE_VERSION:=885b6ec7758a82221700cb99980d441a4db7ecea
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/libtltchilli
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A library for handling coova chilli sessions.
	DEPENDS:=+libubus +libubox
endef

define Package/libtltchilli/description
	Library for handling coova chilli sessions.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/*.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/include/*.h $(1)/usr/include/
endef


define Package/libtltchilli/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libtltchilli.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libtltchilli))
