#
# Copyright (C) 2016 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libsim_switch
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libsim_switch
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A library for sim switching.
  DEPENDS:=+libusb-1.0 +libgsm +libubus +libtlt_uci +libeventslog
endef

define Package/libsim_switch/description
  A simple lib with sim switching management.
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
	$(INSTALL_DIR) $(1)/usr/include/libsim_switch
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/libsim_switch
	$(CP) $(PKG_BUILD_DIR)/libsim_switch.so $(1)/usr/lib/	
endef

define Package/libsim_switch/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libsim_switch.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libsim_switch))
