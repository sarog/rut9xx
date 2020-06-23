# 
# Copyright (C) 2014 teltonika.lt
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libTeltonika-lua
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libTeltonika-lua
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Teltonika LUA functions
endef

define Build/Prepare

endef

define Build/Compile

endef

define Package/libTeltonika-lua/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) ./files/teltonika_lua_functions.lua $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,libTeltonika-lua))
