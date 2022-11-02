#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=smslimit
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=19cb976fda05f77928da28a8d803e96fde1aae19
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

GPL_INCLUDE_HEADERS:=1

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += $(FPIC)

define Package/rpcd-mod-smslimit
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=SMS Limit rpcd module
  DEPENDS:=+rpcd +lsqlite3 +libubus +rpcd-mod-simd
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/*.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/libsms_limit.h $(1)/usr/include/
endef


define Package/rpcd-mod-smslimit/install
	$(INSTALL_DIR) $(1)/usr/lib/rpcd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/smslimit.so $(1)/usr/lib/rpcd/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libsms_limit.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,rpcd-mod-smslimit))
