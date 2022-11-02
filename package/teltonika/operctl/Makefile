#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=operctl
PKG_VERSION:=2022-09-02
PKG_RELEASE:=2

PKG_SOURCE_VERSION:=35f0105cb1157ccecc4c820774643d50c78bd798
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/operctl
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=Operators control process to manage whitelist/blacklist, band, auto APN, etc..
	DEPENDS:=+libusb-1.0 +libpthread +libblobmsg-json +libgsm +liblog +libtlt_uci \
	+libubus +libuci +libubox +libsqlite3 +libjson-c +lsqlite3
endef

define Package/operctl/description
	Network operators control process to manage whitelist/blacklist, band, auto APN, etc..
endef


define Package/operctl/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/config $(1)/usr/bin
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/operctl.conf $(1)/etc/config/operctl
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/operctl $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/check_operator_selection_state.sh \
					$(1)/usr/bin/check_operator_selection_state.sh
endef
$(eval $(call BuildPackage,operctl))
