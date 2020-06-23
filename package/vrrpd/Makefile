#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vrrpd
PKG_VERSION:=1.15
PKG_RELEASE:=1
PKG_SOURCE_URL:=https://github.com/fredbcode/Vrrpd/archive/
PKG_SOURCE:=v$(PKG_VERSION).tar.gz
#PKG_MD5SUM:=5fd4b1a07983a517484bf57c31f7befb
PKG_LICENSE:=GPL-2.0
PKG_SOURCE_SUBDIR:=Vrrpd-$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)
#PKG_BUILD_DIR:=$(BUILD_DIR)/vrrpd-$(PKG_VERSION)



include $(INCLUDE_DIR)/package.mk

define Package/vrrpd
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Routing and Redirection
  TITLE:=Virtual Router Redundancy Protocol daemon
  URL:=http://www.sourceforge.net/projects/vrrpd
  DEPENDS:=+libeventslog +librt +libgsm
endef

define Package/vrrpd/description
	VRRPd is an implementation of Virtual Router Redundancy 
	Protocol as specified in rfc2338. VRRPd is interoperable with 
	other RFC-based VRRP implementations, including Cisco and 
	Juniper, and is included as a standard feature on ImageStream 
	routers.
endef

define Build/Compile
	$(call Build/Compile/Default, \
		DBG_OPT="" \
		MAIN_OPT="$(TARGET_CFLAGS)" \
		vrrpd \
	)
endef

define Package/vrrpd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vrrpd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/vrrpd.config $(1)/etc/config/vrrpd
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/vrrpd.init $(1)/etc/init.d/vrrpd
endef

define Package/vrrpd/conffiles
/etc/config/vrrpd
endef

$(eval $(call BuildPackage,vrrpd))
