#
# Copyright (C) 2018 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libtlt_socket_man
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libtlt_socket_man
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A library for manage socket opening and closing.
	DEPENDS:= +libtlt_base
endef

define Package/libtlt_socket_man/description
	A library for manage socket opening and closing.
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
	$(INSTALL_DIR) $(1)/usr/include/libtlt_socket_man
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/libtlt_socket_man
	$(CP) $(PKG_BUILD_DIR)/libtlt_socket_man.so $(1)/usr/lib/
endef

define Package/libtlt_socket_man/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libtlt_socket_man.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libtlt_socket_man))
