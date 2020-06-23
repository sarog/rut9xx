#
# Copyright (C) 2015 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmnfinfo
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libmnfinfo
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A library for mnf_info
endef

define Package/libmnfinfo/description
	A simple lib 
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += $(FPIC)

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/include/libmnfinfo
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/libmnfinfo
	$(CP) $(PKG_BUILD_DIR)/libmnfinfo.so $(1)/usr/lib/
endef

define Package/libmnfinfo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libmnfinfo.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libmnfinfo))
