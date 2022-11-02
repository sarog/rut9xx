include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=modbusgwd
PKG_VERSION:=2022-05-06

PKG_SOURCE_VERSION:=a5a44c3a47652cc6584dba2270ba9e5014c6865c
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/modbusgwd
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Modbus RTU to TCP gateway by Teltonika
	DEPENDS:=+kmod-usb-serial
endef

define Package/modbusgwd/description
	Modbus RTU to TCP gateway by Teltonika
endef



define Package/modbusgwd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/modbusgwd $(1)/usr/sbin/modbusgwd
endef

$(eval $(call BuildPackage,modbusgwd))
