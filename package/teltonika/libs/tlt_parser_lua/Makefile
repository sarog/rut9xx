#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=gsm_lua_parser
PKG_VERSION:=2021-07-02
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=5103982b43c9d55f619bdf71caf9d42d5c15772d
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/tlt_parser_lua
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A library for parsing functions from LUA.
	DEPENDS:=+liblua +libgsm +libjson-c +libubus +libblobmsg-json
endef

define Package/tlt_parser_lua/description
	A simple lib with commands to parse functions from LUA.
endef


define Package/tlt_parser_lua/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/lua/
	$(CP) $(PKG_BUILD_DIR)/tlt_parser_lua.so $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,tlt_parser_lua))
