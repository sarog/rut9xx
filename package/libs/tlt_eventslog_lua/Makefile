#
# Copyright (C) 2015 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tlt_eventslog_lua
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/tlt_eventslog_lua
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A library for manage Eventslog from LUA.
	DEPENDS:=+libeventslog +liblua
endef

define Package/tlt_eventslog_lua/description
	A simple lib with at commands to manage Eventslog from LUA.
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
	$(INSTALL_DIR) $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/include/tlt_eventslog_lua
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/tlt_eventslog_lua
	$(CP) $(PKG_BUILD_DIR)/tlt_eventslog_lua.so $(1)/usr/lib/lua/
endef

define Package/tlt_eventslog_lua/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/lua/
	$(CP) $(PKG_BUILD_DIR)/tlt_eventslog_lua.so $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,tlt_eventslog_lua))
