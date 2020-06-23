#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=coova-chilli
PKG_VERSION:=1.3.0+20141128
PKG_MAINTAINER:=Imre Kaloz <kaloz@openwrt.org>
PKG_LICENSE:=GPL-2.0+
PKG_LICENSE_FILES:=COPYING
PKG_FIXUP:=forceautoreconf
PKG_RELEASE:=4

# PKG_SOURCE_PROTO:=git
# PKG_SOURCE_URL:=git://github.com/coova/coova-chilli
# PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
# PKG_SOURCE_VERSION:=b93de20a288c01c2ba28e96e31ad6da01627f45f
# PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
# PKG_MD5SUM:=2adb27ec56172b18c5beee359dd7898d

PKG_INSTALL:=1

PKG_CONFIG_DEPENDS := \
  COOVACHILLI_MINIPORTAL \
  COOVACHILLI_REDIR \
  COOVACHILLI_USERAGENT \
  COOVACHILLI_DNSLOG \
  COOVACHILLI_UAMDOMAINFILE \
  COOVACHILLI_LARGELIMITS \
  COOVACHILLI_NOSSL \
  COOVACHILLI_MATRIXSSL \
  COOVACHILLI_CYASSL \
  COOVACHILLI_OPENSSL

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/coova-chilli
  SUBMENU:=Captive Portals
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+kmod-tun +librt +COOVACHILLI_MATRIXSSL:libmatrixssl +COOVACHILLI_CYASSL:libcyassl +COOVACHILLI_OPENSSL:libopenssl +libtlt_uci
  TITLE:=Wireless LAN HotSpot controller (Coova Chilli Version)
  URL:=http://www.coova.org/CoovaChilli
  MENU:=1
endef

define Package/coova-chilli/description
	CoovaChilli is an open source access controller for wireless LAN
	access points and is based on ChilliSpot. It is used for authenticating
	users of a wireless (or wired) LAN. It supports web based login (UAM)
	which is today's standard for public HotSpots and it supports Wireless
	Protected Access (WPA) which is the standard of the future.
	Authentication,	authorization and accounting (AAA) is handled by your
	favorite radius server.
endef

define KernelPackage/ipt-coova
  URL:=http://www.coova.org/CoovaChilli
  SUBMENU:=Netfilter Extensions
  DEPENDS:=coova-chilli +kmod-ipt-core +libxtables
  TITLE:=Coova netfilter module
  FILES:=$(PKG_BUILD_DIR)/src/linux/xt_*.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoProbe,xt_coova)
endef

define KernelPackage/ipt-coova/description
	Netfilter kernel module for CoovaChilli
	Includes:
	- coova
endef

DISABLE_NLS=

TARGET_CFLAGS += $(FPIC)

CONFIGURE_VARS += \
       ARCH="$(LINUX_KARCH)" \
       KERNEL_DIR="$(LINUX_DIR)"

MAKE_FLAGS += \
       ARCH="$(LINUX_KARCH)" \
       KERNEL_DIR="$(LINUX_DIR)"

MAKE_INSTALL_FLAGS += \
       ARCH="$(LINUX_KARCH)" \
       KERNEL_DIR="$(LINUX_DIR)" \
       INSTALL_MOD_PATH="$(PKG_INSTALL_DIR)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/coova-chilli/config
  source "$(SOURCE)/Config.in"
endef

define Build/Configure
	$(call Build/Configure/Default, \
	$(if $(CONFIG_COOVACHILLI_PROXY),--enable,--disable)-chilliproxy \
	$(if $(CONFIG_COOVACHILLI_REDIR),--enable,--disable)-chilliredir \
	$(if $(CONFIG_COOVACHILLI_DNSLOG),--enable,--disable)-dnslog \
	$(if $(CONFIG_COOVACHILLI_MINIPORTAL),--enable,--disable)-miniportal \
	$(if $(CONFIG_COOVACHILLI_USERAGENT),--enable,--disable)-useragent \
	$(if $(CONFIG_COOVACHILLI_LARGELIMITS),--enable,--disable)-largelimits \
	$(if $(CONFIG_COOVACHILLI_UAMDOMAINFILE),--enable,--disable)-uamdomainfile \
	$(if $(CONFIG_COOVACHILLI_MATRIXSSL),--with,--without)-matrixssl \
	$(if $(CONFIG_COOVACHILLI_CYASSL),--with,--without)-cyassl \
	$(if $(CONFIG_COOVACHILLI_OPENSSL),--with,--without)-openssl \
	$(if $(CONFIG_PACKAGE_kmod-ipt-coova),--with-nfcoova) \
	)
endef

define Package/coova-chilli/conffiles
/etc/chilli.conf
endef

define Package/coova-chilli/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/chilli.conf $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/chilli
	$(CP) $(PKG_INSTALL_DIR)/etc/chilli/* $(1)/etc/chilli/
	$(INSTALL_DIR) $(1)/etc/chilli/www/themes
	$(CP) $(PKG_INSTALL_DIR)/../themes/* $(1)/etc/chilli/www/themes/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/chilli* $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/statistics.lua $(1)/usr/sbin/statistics
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so.* $(1)/usr/lib/
	$(if $(CONFIG_PACKAGE_kmod-ipt-coova), \
		$(INSTALL_DIR) $(1)/usr/lib/iptables; \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/iptables/lib*.so $(1)/usr/lib/iptables/ \
	)
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/chilli.init $(1)/etc/init.d/chilli
	$(INSTALL_BIN) ./files/99-chilli.sh $(1)/etc/uci-defaults/99-chilli
# 	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
# 	$(INSTALL_BIN) $(PKG_BUILD_DIR)/chilli.hotplug $(1)/etc/hotplug.d/iface/30-chilli
	$(INSTALL_DIR) $(1)/www
	$(INSTALL_DIR) $(1)/www/cgi-bin
	$(INSTALL_DIR) $(1)/usr
	$(INSTALL_DIR) $(1)/usr/www
	$(INSTALL_DIR) $(1)/usr/www/hotspot
	$(INSTALL_DIR) $(1)/www/hotspot
	$(CP) $(PKG_INSTALL_DIR)/../doc/hotspotlogin.cgi $(1)/usr/www/hotspot/
	$(CP) $(PKG_INSTALL_DIR)/../doc/autologin.cgi $(1)/usr/www/hotspot/
	$(CP) $(PKG_INSTALL_DIR)/../doc/demo_preview.cgi $(1)/www/cgi-bin/
	$(CP) $(PKG_INSTALL_DIR)/../www/tos.lua $(1)/usr/www/hotspot/
	$(INSTALL_DIR) $(1)/www/luci-static
	$(INSTALL_DIR) $(1)/www/luci-static/default
	$(CP) $(PKG_INSTALL_DIR)/../www/wait.gif $(1)/www/luci-static/default/
	$(INSTALL_DIR) $(1)/www/luci-static/resources
	$(CP) $(PKG_INSTALL_DIR)/../www/loginpage.css $(1)/www/luci-static/resources/
	$(LN) /etc/chilli/www/themes/active_background $(1)/www/luci-static/resources/background
	$(LN) /etc/chilli/www/themes/active_logo $(1)/www/luci-static/resources/logo_image
	$(INSTALL_DIR) $(1)/usr/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/lua/
	$(CP) $(PKG_INSTALL_DIR)/../doc/landing_page_functions.lua $(1)/usr/lib/lua/
	$(INSTALL_DIR) $(1)/etc/chilli/configs
	$(CP) $(PKG_BUILD_DIR)/conf/configs/* $(1)/etc/chilli/configs/

endef

$(eval $(call BuildPackage,coova-chilli))
$(eval $(call KernelPackage,ipt-coova))
