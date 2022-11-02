
#
# Copyright (C) 2022 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=opendnp3
PKG_SOURCE_VERSION:=f320e1a3dc5fc8c8ed5ff4cd0fd62faecee3ac98

PKG_LICENSE:=Apache-2.0
PKG_ORIGIN_URL:=https://github.com/dnp3/opendnp3

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/opendnp3
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=DNP3 protocol stack
	DEPENDS:=+libstdcpp
endef

define Package/opendnp3/description
	DNP3 protocol stack
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/cpp/lib/*.so $(STAGING_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/cpp/lib/include/* $(STAGING_DIR)/usr/include/
endef

define Package/opendnp3/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopendnp3.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,opendnp3))
