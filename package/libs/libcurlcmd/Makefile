#
# Copyright (C) 2017 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libcurlcmd
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libcurlcmd
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A library for easy curl command execution.
  DEPENDS:=+libeventslog
endef

define Package/libcurlcmd/description
  A library for easy curl command execution.
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
	$(INSTALL_DIR) $(1)/usr/include/libcurlcmd
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/libcurlcmd
	$(CP) $(PKG_BUILD_DIR)/libcurlcmd.so $(1)/usr/lib/
endef

define Package/libcurlcmd/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libcurlcmd.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libcurlcmd))
