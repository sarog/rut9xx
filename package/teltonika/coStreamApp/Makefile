#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=coStreamApp
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=f351c00bd936b76fccfc9741c5e151af1bab238e
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

GPL_INCLUDE_SRC:=1

include $(INCLUDE_DIR)/package.mk

define Package/coStreamApp
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Cloud of Things Stream application.
	DEPENDS:=+libsera +libuci +libgsm +libmnfinfo +vuci-app-cloud-of-things
	PKG_TLT_NAME:=Cloud of Things
	PKG_ROUTER:=$(TLT_PLATFORM_NAME)
endef

define Package/coStreamApp/description
	A simple Cloud of Things IoT StreamApp package.
endef

define Package/coStreamApp/conffiles
/etc/config/cot
endef

define Package/coStreamApp/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/coStreamApp $(1)/usr/bin/coStreamApp
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/cot.init $(1)/etc/init.d/cot
	$(INSTALL_DIR) $(1)/usr/lib/lua/co
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/srtemplate $(1)/usr/lib/lua/co/srtemplate.txt
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/stream.lua $(1)/usr/lib/lua/co/stream.lua
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/cot $(1)/etc/config/cot
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/cot.hotplug $(1)/etc/hotplug.d/iface/98-cot

	if [ ! $(TLT_PLATFORM_RUTX) ]; then \
	    $(SED) "s#Teltonika-Networks_RUTX#Teltonika-Networks_$(TLT_PLATFORM_NAME)#g" $(1)/usr/lib/lua/co/srtemplate.txt; \
	fi;
endef

$(eval $(call BuildPackage,coStreamApp))
