#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=azure_iothub
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=4c739f5e46170d5826b393d44afe6ed68ea683b6
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/azure_iothub
	SECTION:=base
	CATEGORY:=Base system
	DEPENDS:=+libc +USE_EGLIBC:librt +USE_EGLIBC:libpthread +libuci +libgsm +libubus +libopenssl +libblobmsg-json +libuuid +libmosquitto-ssl +vuci-app-azure-iothub
	TITLE:=Azure IoT MQTT client
	PKG_TLT_NAME:=Azure IoT Hub
	PKG_ROUTER:=$(TLT_PLATFORM_NAME)
endef

define Package/azure_iothub/conffiles
/etc/config/azure_iothub
endef


define Package/azure_iothub/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/teltonika_client/azure_iothub $(1)/sbin/azure_iothub

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/azure_iothub.init $(1)/etc/init.d/azure_iothub

	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/azure_iothub.conf $(1)/etc/config/azure_iothub

	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/azure_iothub.hotplug $(1)/etc/hotplug.d/iface/97-azure_iothub
endef

$(eval $(call BuildPackage,azure_iothub))
