# 
# Copyright (C) 2021 teltonika.lt
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lua-pop3
PKG_VERSION:=0.1.6
PKG_RELEASE:=1

PKG_SOURCE:=v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/moteus/lua-pop3/archive/
PKG_HASH:=58b4eda85ec17b277d3f466c6c84ebd52bf9bbb2f80e2f6b76e987a207b43f76
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

MAINTAINER:=Alexey Melnichuk
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE.txt

include $(INCLUDE_DIR)/package.mk

define Package/lua-pop3
	SUBMENU:=Lua
	SECTION:=lang
	CATEGORY:=Languages
	TITLE:=LUA POP3 LIB
	DEPENDS:=+luasocket +lua-md5 +luabitop +luasec
endef

define Build/Compile

endef

define Package/lua-pop3/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/pop3
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lua/pop3.lua $(1)/usr/lib/lua/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lua/pop3/charset.lua $(1)/usr/lib/lua/pop3/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lua/pop3/message.lua $(1)/usr/lib/lua/pop3/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lua/pop3/module.lua $(1)/usr/lib/lua/pop3/
endef

$(eval $(call BuildPackage,lua-pop3))
