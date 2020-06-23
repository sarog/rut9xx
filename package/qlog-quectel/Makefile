#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=qlog-quectel
PKG_VERSION:=0.0.1
PKG_BUILD_DIR:=$(BUILD_DIR)/qlog-quectel

include $(INCLUDE_DIR)/package.mk

define Package/qlog-quectel
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Quectel EC25 firmware update tool by Teltonika
  DEPENDS:=+libstdcpp +libusb-1.0 +libpthread
endef

define Package/qlog-quectel/description
  Test package template by Teltonika
endef

#CONFIGURE_ARGS = ''

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
	$(TARGET_CONFIGURE_OPTS)
endef

define Build/InstallDev
endef

define Package/qlog-quectel/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/qlog-quectel $(1)/usr/sbin/qlog-quectel
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/qlog-filter.cfg $(1)/etc/qlog-filter.cfg
endef

$(eval $(call BuildPackage,qlog-quectel))
