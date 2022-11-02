include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=modbus_tcp_master
PKG_VERSION:=2022-08-08
PKG_RELEASE:=1

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=1cfecdc94966797b93903e5ddbd022c9a995043f
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/modbus_tcp_master
	SECTION:=net
	CATEGORY:=Network
	TITLE:=MODBUS TCP master daemon by Teltonika
	DEPENDS:=+libuci +libtlt_uci +libsqlite3 +vuci-app-modbus +modbus_tcp_test

	ifeq (m, $(CONFIG_PACKAGE_modbus_tcp_master))
		PKG_TLT_NAME:=MODBUS TCP Master
		PKG_ROUTER:=$(TLT_PLATFORM_NAME)
	endif
endef

define Package/modbus_tcp_master/description
	MODBUS TCP master daemon by Teltonika
endef

define Package/modbus_tcp_master/conffiles
/etc/config/modbus_tcp_master
/etc/config/modbus_master_alarms
endef


define Package/modbus_tcp_master/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/modbus_tcp_master.conf $(1)/etc/config/modbus_tcp_master
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/modbus_master_alarms.conf $(1)/etc/config/modbus_master_alarms
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/modbus_tcp_master.init $(1)/etc/init.d/modbus_tcp_master
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/modbus_tcp_master $(1)/usr/sbin/modbus_tcp_master
endef

$(eval $(call BuildPackage,modbus_tcp_master))
