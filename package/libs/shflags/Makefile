#
# Copyright (C) 2012-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=shflags
PKG_VERSION:=2018-01-25
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/kward/shflags.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=b028d78a70b04986f65d0166ec0cf33e50eec598
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_LICENSE:=LGPL-2.1

include $(INCLUDE_DIR)/package.mk

define Package/shflags
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=shflags
  DEPENDS:=+getopt
  URL:=http://code.google.com/p/shflags/
  MAINTAINER:=Luka Perkov <luka@openwrt.org>
endef

define Package/shflags/description
 command-line flags module for Unix shell scripts
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/Install
endef

define Package/shflags/install
	$(INSTALL_DIR) $(1)/usr/share/shflags
	$(CP) $(PKG_BUILD_DIR)/shflags  $(1)/usr/share/shflags/shflags.sh
endef

$(eval $(call BuildPackage,shflags))
