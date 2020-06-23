include $(TOPDIR)/rules.mk

PKG_NAME:=modbus-tools
PKG_VERSION:=0.0.1b
PKG_BUILD_DIR:=$(BUILD_DIR)/modbus-tools

include $(INCLUDE_DIR)/package.mk

define Package/modbus-tools
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Modbus testing tools
  DEPENDS:=+libmodbus
endef

define Package/modbus-tools/description
	Modbus testing tools
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
	$(TARGET_CONFIGURE_OPTS)
endef

define Package/modbus-tools/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/modbus-master $(1)/usr/sbin/modbus-master
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/modbus-slave $(1)/usr/sbin/modbus-slave
endef

$(eval $(call BuildPackage,modbus-tools))
