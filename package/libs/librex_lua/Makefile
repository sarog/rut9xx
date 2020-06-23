#
# Copyright (C) 2018 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=librex_lua
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/librex_lua
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A Lua binding of various regex library APIs
	DEPENDS:=+liblua +libpcre
endef

define Package/librex_lua/description
	A Lua binding of various regex library APIs
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
	$(INSTALL_DIR) $(1)/usr/lib/lua/
	$(CP) $(PKG_BUILD_DIR)/librex_lua.so $(1)/usr/lib/lua/
endef

define Package/librex_lua/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/lua/
	$(CP) $(PKG_BUILD_DIR)/librex_lua.so $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,librex_lua))
