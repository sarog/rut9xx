#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=quectel-CM
PKG_VERSION:=1.1.38
PKG_BUILD_DIR:=$(BUILD_DIR)/quectel-CM

include $(INCLUDE_DIR)/package.mk

define Package/quectel-CM
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Connection control utility for Quectel EC25 modem
  DEPENDS:=+libpthread +libusb-1.0
endef

define Package/quectel-CM/description
  Connection control utility for Quectel EC25 modem
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(call Build/Prepare/Default)
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
	$(TARGET_CONFIGURE_OPTS)
endef

define Package/quectel-CM/install
	$(INSTALL_DIR) $(1)/usr/sbin/ $(1)/lib/netifd/proto/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/quectel-CM $(1)/usr/sbin/quectel-CM
	$(INSTALL_BIN) ./files/mobile.script $(1)/lib/netifd/
#	$(INSTALL_BIN) ./files/gobinet.sh $(1)/lib/netifd/proto/
	$(INSTALL_BIN) ./files/qmi2.sh $(1)/lib/netifd/proto/
	$(INSTALL_BIN) ./files/dhcp_mobile.script $(1)/lib/netifd/
endef

$(eval $(call BuildPackage,quectel-CM))
