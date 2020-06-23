# 
# Copyright (C) 2015 teltonika.lt
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libpop3-lua
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libpop3-lua
	SUBMENU:=Lua
	SECTION:=lang
	CATEGORY:=Languages
	TITLE:=LUA POP3 LIB
endef

define Build/Prepare

endef

define Build/Compile

endef

define Package/libpop3-lua/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/pop3
	$(INSTALL_BIN) ./files/pop3.lua $(1)/usr/lib/lua
	$(INSTALL_BIN) ./files/pop3/charset.lua $(1)/usr/lib/lua/pop3/
	$(INSTALL_BIN) ./files/pop3/message.lua $(1)/usr/lib/lua/pop3/
endef

$(eval $(call BuildPackage,libpop3-lua))
