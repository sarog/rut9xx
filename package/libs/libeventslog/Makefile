#
# Copyright (C) 2015 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libeventslog
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libeventslog
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A library for manage Eventslog. Select, insert etc.
	DEPENDS:=+libsqlite3
endef

define Package/libeventslog/description
	A simple lib with at commands to manage Eventslog.
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
	$(INSTALL_DIR) $(1)/usr/include/libeventslog
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/libeventslog
	$(CP) $(PKG_BUILD_DIR)/libeventslog.so $(1)/usr/lib/
endef

define Package/libeventslog/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libeventslog.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libeventslog))
