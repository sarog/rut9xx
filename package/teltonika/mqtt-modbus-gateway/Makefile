include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=mqtt-modbus-gateway
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=18c923cab3f61f6ad894369ce92efda66b8febec
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/mqtt-modbus-gateway
		SECTION:=net
		CATEGORY:=Network
		TITLE:=MQTT-Modbus Gateway
		DEPENDS:=+libmosquitto +vuci-app-modbus-gateway

		ifeq (m, $(CONFIG_PACKAGE_mqtt-modbus-gateway))
			PKG_TLT_NAME:=MQTT-MODBUS-gateway
			PKG_ROUTER:=$(TLT_PLATFORM_NAME)
		endif
endef

define Package/mqtt-modbus-gateway/description
		This application provides MQTT-Modbus Gateway
endef

define Package/mqtt-modbus-gateway/conffiles
/etc/config/modbusgateway
endef


define Package/mqtt-modbus-gateway/install
		$(INSTALL_DIR) $(1)/etc/config
		$(INSTALL_DIR) $(1)/etc/init.d
		$(INSTALL_DIR) $(1)/usr/sbin
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/modbusgateway.conf $(1)/etc/config/modbusgateway
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/modbusgateway.init $(1)/etc/init.d/modbusgateway
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/modbusgateway $(1)/usr/sbin/modbusgateway
endef

$(eval $(call BuildPackage,mqtt-modbus-gateway))

