include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=modbus_data_sender
PKG_VERSION:=2022-08-08
PKG_RELEASE:=1

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=056470510bfb8103d0f25dff54887cf43c84b58e
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/modbus_data_sender
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Data sender daemon by Teltonika
	DEPENDS:=+libuci +libtlt_uci +libsqlite3 +libcurl +libmosquitto +libopenssl +libubox +libcgi +libblobmsg-json +vuci-app-data-sender

	ifeq (m, $(CONFIG_PACKAGE_modbus_data_sender))
		PKG_TLT_NAME:=Data to Server
		PKG_ROUTER:=$(TLT_PLATFORM_NAME)
	endif
endef

define Package/modbus_data_sender/description
	Data sender daemon by Teltonika
endef

define Package/modbus_data_sender/conffiles
/etc/config/modbus_data_sender
endef


define Package/modbus_data_sender/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/modbus_data_sender.conf $(1)/etc/config/modbus_data_sender
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/modbus_data_sender.init $(1)/etc/init.d/modbus_data_sender
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/modbus_data_sender $(1)/usr/sbin/modbus_data_sender
endef

$(eval $(call BuildPackage,modbus_data_sender))
