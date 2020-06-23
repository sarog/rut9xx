#
# Copyright (C) 2015 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libtlt_uci
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libtlt_uci
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A library with uci function.
	DEPENDS:=+libusb-1.0 +libuci
endef

define Package/libtlt_uci/description
	A simple lib with uci function.
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
	$(INSTALL_DIR) $(1)/usr/include/libtlt_uci
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/libtlt_uci
	$(CP) $(PKG_BUILD_DIR)/libtlt_uci.so $(1)/usr/lib/
endef

define Package/libtlt_uci/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libtlt_uci.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libtlt_uci))
