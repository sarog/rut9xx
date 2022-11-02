#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=host_block
PKG_VERSION:=2022-05-10
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=f4183cc462d17835b3703631de5713da22b71a9c
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/host_block
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=Host Block
	DEPENDS:=+dnsmasq
endef

define Package/host_block/conffiles
/etc/config/hostblock
endef



define Package/host_block/install
	$(INSTALL_DIR) $(1)/etc/init.d/ $(1)/etc/config/ $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/hostblock.init $(1)/etc/init.d/hostblock
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/files/hostblock.conf $(1)/etc/config/hostblock
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/hostblock.sh $(1)/usr/sbin/hostblock.sh
endef

$(eval $(call BuildPackage,host_block))
