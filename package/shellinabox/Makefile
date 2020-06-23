#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=shellinabox
PKG_VERSION:=2.20
PKG_BUILD_DIR:=$(BUILD_DIR)/shellinabox
PKG_BUILD_DEPENDS:=zlib

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/uclibc++.mk
include $(INCLUDE_DIR)/package.mk

define Package/shellinabox
  SECTION:=net
  CATEGORY:=Network
  TITLE:=SSH over HTTP
  DEPENDS:=+zlib +libopenssl
endef

define Package/shellinabox/description
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)

endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS) -lm" \
		BINDIR="/usr/sbin" \
		all
endef

define Build/InstallDev
endef

define Package/shellinabox/install
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/cli.conf $(1)/etc/config/cli
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/shellinabox.init $(1)/etc/init.d/shellinabox
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/shellinaboxd $(1)/usr/sbin/shellinaboxd
	$(INSTALL_DIR) $(1)/www/cgi-bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/shellinabox.sh $(1)/www/cgi-bin/cli
endef

CONFIGURE_ARGS+= -disable-runtime-loading

$(eval $(call BuildPackage,shellinabox))
