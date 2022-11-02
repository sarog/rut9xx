#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=rms-packages
PKG_VERSION:=2022-08-08
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=4ffb942d25b2af5f8f88d9086b65f8593ec05478
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/rms_mqtt
	SUBMENU:=RMS
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=MQTT application for RMS by Teltonika
	DEPENDS:=+libmosquitto +libmnfinfo +libopenssl +libuci +libtlt_uci +libcurl +libusb-1.0 +libgsm
endef

define Package/rms_connect
	SUBMENU:=RMS
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=First initialization of connection with RMS
	DEPENDS:=+libuci +libopenssl +libmnfinfo +libgsm
endef

define Package/rms_json
  	SUBMENU:=RMS
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=Collects and prints out information about device.
	DEPENDS:=+libusb-1.0 +libuci +libgsm +GPS_SUPPORT:libgps +libmnfinfo +libblobmsg-json +libubus \
		+libubox +libjson-c +libmdcollect +MOBILE_SUPPORT:rpcd-mod-simd +IO_SUPPORT:iomand
endef

TARGET_CFLAGS+=-DRUTX -Isrc/cJSON


define Package/rms_mqtt/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rms_mqtt $(1)/usr/sbin/rms_mqtt

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rms_mqtt.init $(1)/etc/init.d/rms_mqtt
	chmod 755 $(1)/etc/init.d/rms_mqtt

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rms_mqtt.conf $(1)/etc/config/rms_mqtt

	$(INSTALL_DIR) $(1)/etc/rms_mqtt
endef

define Package/rms_connect/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rms_connect $(1)/usr/sbin/rms_connect

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rms_connect.init $(1)/etc/init.d/rms_connect

	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rms_connect_mqtt.conf $(1)/etc/config/rms_connect_mqtt
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rms_connect_timer.conf $(1)/etc/config/rms_connect_timer

	$(INSTALL_DIR) $(1)/etc/rms_mqtt/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/RutOS_teltonika.lt.ca $(1)/etc/rms_mqtt/RutOS@teltonika.lt.ca
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/RutOS_teltonika.lt.crt $(1)/etc/rms_mqtt/RutOS@teltonika.lt.crt
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/RutOS_teltonika.lt.key $(1)/etc/rms_mqtt/RutOS@teltonika.lt.key
endef

define Package/rms_json/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rms_json $(1)/sbin/rms_json
endef

$(eval $(call BuildPackage,rms_mqtt))
$(eval $(call BuildPackage,rms_connect))
$(eval $(call BuildPackage,rms_json))
