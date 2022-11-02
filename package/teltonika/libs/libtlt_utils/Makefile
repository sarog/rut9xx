#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=libtlt_utils
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=5ec3c53ade796a3ab937672c3ea6e0bec5692fe6
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

GPL_INCLUDE_HEADERS:=1

include $(INCLUDE_DIR)/package.mk

define Package/libtlt_utils
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=Utils library by Teltonika
	MENU:=1
	DEPENDS:=+libubox
endef

define Package/libtlt_utils/description
	Utilities library. Created by Teltonika
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/*.so $(STAGING_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/include/*.h $(STAGING_DIR)/usr/include/
endef


define Package/libtlt_utils/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libtlt_utils.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libtlt_utils))
