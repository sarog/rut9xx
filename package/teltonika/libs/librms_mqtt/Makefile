#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=librms_mqtt
PKG_VERSION:=1.0
PKG_RELEASE:=1

GPL_INCLUDE_HEADERS:=1

include $(INCLUDE_DIR)/package.mk

define Package/librms_mqtt
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A library for rms_mqtt
  DEPENDS:=+libmnfinfo
endef

define Package/librms_mqtt/description
	A library, to send trigger to rms_mqtt.
endef


TARGET_CFLAGS += $(FPIC)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/include/librms_mqtt
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/librms_mqtt
	$(CP) $(PKG_BUILD_DIR)/librms_mqtt.so $(1)/usr/lib/
endef


define Package/librms_mqtt/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/librms_mqtt.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,librms_mqtt))
