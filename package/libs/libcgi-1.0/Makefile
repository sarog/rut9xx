#
# Copyright (C) 2016 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libcgi
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libcgi
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A library for manage cgi API.
   DEPENDS:=+libusb-1.0
endef

define Package/libcgi/description
  A simple lib to use .
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
	$(INSTALL_DIR) $(1)/usr/include/libcgi
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/libcgi
	$(CP) $(PKG_BUILD_DIR)/libcgi.so $(1)/usr/lib/
endef

define Package/libcgi/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libcgi.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libcgi))