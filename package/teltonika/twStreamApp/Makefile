#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=twStreamApp
PKG_VERSION:=2021-06-22
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=71b956ce537aec3769938be92315f9a2db98ee60
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/twStreamApp
  SECTION:=net
  CATEGORY:=Network
  TITLE:=ThingWorx Stream application.
  DEPENDS:=+libtwCSdk +libgsm +libtlt_uci +libuci +libopenssl +vuci-app-thingworx
  PKG_TLT_NAME:=ThingWorx
  PKG_ROUTER:=$(TLT_PLATFORM_NAME)
endef

define Package/twStreamApp/description
	Teltonika ThingWorx API package.
endef

define Package/twStreamApp/conffiles
/etc/config/iottw
endef



define Package/twStreamApp/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/twStreamApp $(1)/usr/bin/twStreamApp
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/twd.init $(1)/etc/init.d/twd
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/iottw $(1)/etc/config/iottw
endef

$(eval $(call BuildPackage,twStreamApp))
