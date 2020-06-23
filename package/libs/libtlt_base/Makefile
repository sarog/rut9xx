#
# Copyright (C) 2018 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libtlt_base
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libtlt_base
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A library for base functions like quit check error
	DEPENDS:=
endef

define Package/libtlt_base/description
	A library for base functions like quit check error
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
	$(INSTALL_DIR) $(1)/usr/include/libtlt_base
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/libtlt_base
	$(CP) $(PKG_BUILD_DIR)/libtlt_base.so $(1)/usr/lib/
endef

define Package/libtlt_base/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libtlt_base.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libtlt_base))
