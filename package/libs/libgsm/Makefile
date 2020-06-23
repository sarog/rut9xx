#
# Copyright (C) 2015 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libgsm
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libgsm
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A library for manage gsm API. Read, list, delete, etc.
  DEPENDS:=+libusb-1.0 +libuci +libeventslog  +libtlt_base +libgps +libtlt_socket_man
endef

define Package/libgsm/description
  A simple lib with at commands to manage gsm.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += $(FPIC) -std=gnu11

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include/libgsm
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/libgsm
	$(CP) $(PKG_BUILD_DIR)/libgsm.so $(1)/usr/lib/
endef

define Package/libgsm/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libgsm.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libgsm))
