#
# Copyright (C) 2015 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libsms_utils
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libsms_utils
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A library for sms_utils
	DEPENDS:=+libtlt_uci +libeventslog
endef

define Package/libsms_utils/description
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
	$(INSTALL_DIR) $(1)/usr/include/libsms_utils
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/libsms_utils
	$(CP) $(PKG_BUILD_DIR)/libsms_utils.so $(1)/usr/lib/
endef

define Package/libsms_utils/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libsms_utils.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libsms_utils))
