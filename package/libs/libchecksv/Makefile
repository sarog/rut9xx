#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libchecksv
PKG_VERSION:=0.0.1

include $(INCLUDE_DIR)/package.mk

define Package/libchecksv
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=lib for luci-reload by Teltonika
  DEPENDS:=+libuci
endef

define Package/libchecksv/description
  Test package template by Teltonika
endef

#CONFIGURE_ARGS = ''
	
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += $(FPIC)

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include/libchecksv
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/libchecksv
	$(CP) $(PKG_BUILD_DIR)/libchecksv.so $(1)/usr/lib/
endef

define Package/libchecksv/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libchecksv.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libchecksv))
