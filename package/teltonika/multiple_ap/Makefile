#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=multiple_ap
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=dbecb496db8ec1a1b2425b9e07efddd673aa0b30
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/multiple_ap
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=Multiple Access Points in Station Mode
endef



define Package/multiple_ap/install
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/multiple_ap.sh $(1)/usr/sbin/multiple_ap.sh
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/multi_wifi.init $(1)/etc/init.d/multi_wifi
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/multi_wifi.config $(1)/etc/config/multi_wifi
endef


$(eval $(call BuildPackage,multiple_ap))
