include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=modbus_tcp_test
PKG_VERSION:=2020-09-11
PKG_RELEASE:=1

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=36119abd4f1c2d5440ee6e636608db538742c6dc
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/modbus_tcp_test
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Modbus TCP test application by Teltonika
	DEPENDS:=+libuci +libtlt_uci
endef

define Package/modbus_tcp_test/description
	Modbus TCP test application by Teltonika
endef


define Package/modbus_tcp_test/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/modbus_tcp_test $(1)/usr/sbin/modbus_tcp_test
endef

$(eval $(call BuildPackage,modbus_tcp_test))
