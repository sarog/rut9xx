#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=speedtest
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=ac0a68031d68bf8da64662f9320d457362d8b202
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += $(FPIC)

define Package/speedtest
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=Speedtest
  DEPENDS:=+liblua +libcurl +vuci-app-speed-test
  ifeq (m, $(CONFIG_PACKAGE_speedtest))
  	PKG_TLT_NAME:=Speedtest
  	PKG_ROUTER:=$(TLT_PLATFORM_NAME)
  endif
endef


define Package/speedtest/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libspeedtest.so $(1)/usr/lib/lua/libspeedtest.so
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/speedtest.lua $(1)/usr/bin/speedtest
endef

$(eval $(call BuildPackage,speedtest))
