include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=modbus_serial_master
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=629065a96601f8869438b9d005ff746e3a5789ae
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/modbus_serial_master
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Modbus serial master daemon by Teltonika
	DEPENDS:=+libuci +libtlt_uci +libsqlite3 +vuci-app-modbus-serial
	PKG_TLT_NAME:=Modbus Serial Master
	ifeq (y, $(TLT_PLATFORM_TRB1))
		PKG_ROUTER:=TRB142 TRB145
	else ifeq (y, $(TLT_PLATFORM_RUT9))
		PKG_ROUTER:=RUT955
	else ifeq (y, $(TLT_PLATFORM_RUTX))
		PKG_ROUTER:=RUTXR1
	else ifeq (y, $(TLT_PLATFORM_RUT9M))
		PKG_ROUTER:=RUT956
	else
		PKG_ROUTER:=$(TLT_PLATFORM_NAME)
	endif
endef

define Package/modbus_serial_master/description
	Modbus Serial master daemon by Teltonika
endef

define Package/modbus_serial_master/conffiles
/etc/config/modbus_serial_master
endef


define Package/modbus_serial_master/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/modbus_serial_master $(1)/usr/sbin/modbus_serial_master
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/modbus_serial_test $(1)/usr/sbin/modbus_serial_request_test
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/modbus_serial_master.conf $(1)/etc/config/modbus_serial_master
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/modbus_serial_master.init $(1)/etc/init.d/modbus_serial_master
endef

$(eval $(call BuildPackage,modbus_serial_master))
