#
# Copyright (C) 2015 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libunhandler
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libunhandler
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A library for unsolicited handler.
  DEPENDS:=+libusb-1.0 +libgps +libgsm +libtlt_uci +libeventslog
endef

define Package/libunhandler/description
  A simple lib with at commands to manage unhandler.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
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
	$(INSTALL_DIR) $(1)/usr/include/libunhandler
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/libunhandler
	$(CP) $(PKG_BUILD_DIR)/libunhandler.so $(1)/usr/lib/
endef

define Package/libunhandler/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libunhandler.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libunhandler))
