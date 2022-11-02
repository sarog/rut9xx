#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=rut_fota
PKG_VERSION:=2022-07-15
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=d514e90f384b78a127465f60940326900a9e3a50
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/librut_fota
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=Library for handling rut_fota information
	DEPENDS:=+libubus +libubox +libblobmsg-json +libjson-c
endef

define Package/rut_fota
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=Firmware Over The Air Utility
	DEPENDS:=+libuci +libmnfinfo +libgsm +libubus +libubox +libopenssl +libblobmsg-json +libjson-c +librut_fota
endef

define Package/rut_fota/conffiles
	/etc/config/rut_fota
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/librut_fota.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/lib/librut_fota.h $(1)/usr/include/
endef


define Package/librut_fota/install
	$(INSTALL_DIR) $(1)/usr/lib/ $(1)/usr/lib/rpcd/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fota.so $(1)/usr/lib/rpcd/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/librut_fota.so $(1)/usr/lib/
endef

define Package/rut_fota/install
	$(INSTALL_DIR) $(1)/etc/init.d/ $(1)/etc/config/ $(1)/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rut_fota $(1)/sbin/rut_fota
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rut_fota.init $(1)/etc/init.d/rut_fota
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/rut_fota.config $(1)/etc/config/rut_fota
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/files/RUT_FOTA_CA.pem $(1)/etc/
endef

$(eval $(call BuildPackage,librut_fota))
$(eval $(call BuildPackage,rut_fota))
