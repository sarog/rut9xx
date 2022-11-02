#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=liblog
PKG_VERSION:=2022-05-06
PJG_RELEASE:=1

PKG_SOURCE_VERSION:=5163c236e969d5fee92d3a56e4933dc72dd715e8
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/liblog
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A library for logd DB management. Select, insert etc.
	DEPENDS:=+libuci +libtlt_uci +libubus +libsqlite3 +libubox
endef

define Package/liblog/description
	A simple lib with ubus to manage logd.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/*.so $(STAGING_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/include/*.h $(STAGING_DIR)/usr/include/
endef


define Package/liblog/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/liblog.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,liblog))
