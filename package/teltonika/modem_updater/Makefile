#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=modem_updater
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=010a2b41d19680f2faf94dd80e23433fde5bd316
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/modem_updater
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Modem firmware update tool by Teltonika
	DEPENDS:=+libusb-1.0 +libstdcpp

	PKG_TLT_NAME:=modem_updater
	PKG_ROUTER:=$(TLT_PLATFORM_NAME)
	PKG_HIDDEN:=1
endef

define Package/modem_updater/description
	Modem firmware update tool by Teltonika
endef


define Package/modem_updater/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/quectel_flash/quectel_flash $(1)/usr/bin/quectel_flash
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/Meig_Firehose/meig_firehose $(1)/usr/bin/meig_firehose
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/modem_updater.sh $(1)/usr/bin/modem_updater

endef

$(eval $(call BuildPackage,modem_updater))
