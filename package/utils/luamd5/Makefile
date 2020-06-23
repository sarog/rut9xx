#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luamd5
PKG_VERSION:=1.1.2
PKG_RELEASE:=1

PKG_SOURCE:=md5-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://files.luaforge.net/releases/md5/md5/md5-$(PKG_VERSION)/
PKG_MD5SUM:=68874cff64547f2157d5ecdc3572b544

PKG_BUILD_DIR:=$(BUILD_DIR)/md5-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/luamd5
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lua md5 library
  URL:=http://www.keplerproject.org/md5/
  DEPENDS:=+liblua
endef

define Package/luamd5/description
	MD5 offers basic cryptographic facilities for Lua 5.1: a hash (digest)
	function, a pair crypt/decrypt based on MD5 and CFB, and a pair
	crypt/decrypt based on DES with 56-bit keys.
endef

define Build/Configure
	$(call Build/Configure/Default,,bash)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
          CC="$(TARGET_CC) $(TARGET_CFLAGS) \
          -I"$(STAGING_DIR)/usr/include" \
          -std=gnu99 -fPIC" \
          all
endef

define Package/luamd5/install
	$(INSTALL_DIR)  $(1)/usr/lib/lua
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/md5.lua  $(1)/usr/lib/lua
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/src/des56.so $(1)/usr/lib/lua
	$(INSTALL_DIR)  $(1)/usr/lib/lua/md5
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/src/core.so  $(1)/usr/lib/lua/md5
endef

$(eval $(call BuildPackage,luamd5)) 
