#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=ntpclient
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1

GPL_INCLUDE_SRC:=1

PKG_SOURCE_VERSION:=28821e52dd91d595a31600be28d57c1a3c20981e
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/ntpclient
  SUBMENU:=Time Synchronization
  SECTION:=net
  CATEGORY:=Network
  TITLE:=NTP (Network Time Protocol) client
  DEPENDS:=+librt +libubus +libubox +libblobmsg-json
endef

define Package/ntpclient/description
	NTP client for setting system time from NTP servers.
endef

define Package/ntpclient/conffiles
/etc/config/ntpclient
endef

MAKE_FLAGS += \
	all adjtimex

define Package/ntpclient/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/files/ntpclient.hotplug $(1)/etc/hotplug.d/iface/20-ntpclient
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/ntpclient.init $(1)/etc/init.d/ntpclient
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/ntpclient.config $(1)/etc/config/ntpclient
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ntpclient $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/adjtimex $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/rate.awk $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,ntpclient))
