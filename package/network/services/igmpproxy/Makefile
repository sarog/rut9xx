#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=igmpproxy
PKG_VERSION:=0.2.1
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/pali/igmpproxy/releases/download/${PKG_VERSION}/
PKG_HASH:=d351e623037390f575c1203d9cbb7ba33a8bdef85a3c5e1d2901c5a2a38449a1
PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

include $(INCLUDE_DIR)/package.mk

PKG_FIXUP:=autoreconf
PKG_LICENSE:=GPL-2.0+

define Package/igmpproxy
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Routing and Redirection
  DEPENDS:=+USE_GLIBC:librt +vuci-app-igmp-proxy
  TITLE:=Multicast Routing Daemon
  URL:=http://sourceforge.net/projects/igmpproxy
  PKG_TLT_NAME:=IGMP Proxy
  PKG_ROUTER:=$(TLT_PLATFORM_NAME)
endef

define Package/igmpproxy/description
	IGMPproxy is a simple dynamic Multicast Routing Daemon using
	only IGMP signalling (Internet Group Management Protocol).
endef

define Package/igmpproxy/conffiles
/etc/config/igmpproxy
endef

TARGET_CFLAGS += -Dlog=igmpproxy_log

define Package/igmpproxy/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/igmpproxy.config $(1)/etc/config/igmpproxy
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/igmpproxy.init $(1)/etc/init.d/igmpproxy
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/igmpproxy $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,igmpproxy))
