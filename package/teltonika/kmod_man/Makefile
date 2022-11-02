#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=kmod_man
PKG_VERSION:=2020-01-06
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=aacdb260230c5ad3b8b1e18a55ff61e4bbc95371
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/kmod_man
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=Kernel module manager
endef

define Package/kmod_man/conffiles
	/etc/config/kmod_man
endef



define Package/kmod_man/install
	$(INSTALL_DIR) $(1)/etc/init.d/ $(1)/etc/config/ $(1)/usr/sbin/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/files/kmod_man.conf $(1)/etc/config/kmod_man
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/kmod_man.init $(1)/etc/init.d/kmod_man
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/kmod_man.sh $(1)/usr/sbin/kmod_man.sh
endef

$(eval $(call BuildPackage,kmod_man))
