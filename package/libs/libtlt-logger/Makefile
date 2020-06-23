#
# Copyright (C) 2019 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libtlt-logger
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=Library providing logging utilities.
	DEPENDS:=+libuci
endef

define Package/$(PKG_NAME)/description
	A library providing logging utilities.
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
	$(INSTALL_DIR) $(1)/usr/include/libtlt_logger
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/libtlt_logger
	$(CP) $(PKG_BUILD_DIR)/libtlt_logger.so $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libtlt_logger.so $(1)/usr/lib/
endef


$(eval $(call BuildPackage,$(PKG_NAME)))
