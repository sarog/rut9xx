#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=libtlt-logger
PKG_VERSION:=2020-01-06
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=a1b97e0660d110f92f334bd964e5e98ab3eafbc6
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

GPL_INCLUDE_HEADERS:=1

include $(INCLUDE_DIR)/package.mk

define Package/libtlt-logger
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=Library providing logging utilities.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/lib $(STAGING_DIR)/usr/include
	$(CP) $(PKG_BUILD_DIR)/libtlt_logger.so $(STAGING_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/tlt_logger.h $(STAGING_DIR)/usr/include/
endef


define Package/libtlt-logger/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libtlt_logger.so $(1)/usr/lib/libtlt_logger.so
endef

$(eval $(call BuildPackage,libtlt-logger))
