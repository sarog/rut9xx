#
# Copyright (C) 2022 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=libtlt_uci
PKG_VERSION:=2022-06-02
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=6a8f7af0284f444a78aded975485df507a7035e1
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

GPL_INCLUDE_HEADERS:=1

include $(INCLUDE_DIR)/package.mk

define Package/libtlt_uci
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A library with uci function.
	DEPENDS:= +libuci
endef

define Package/libtlt_uci/description
	A simple lib with uci function. Created by Teltonika
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/*.so $(STAGING_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/include/*.h $(STAGING_DIR)/usr/include/
endef


define Package/libtlt_uci/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libtlt_uci.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libtlt_uci))
