#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=modbusd
PKG_VERSION:=2022-08-08
PKG_RELEASE:=1

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=b86f3d50f31c43b4f194dc099afc414a17ed0f40
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/modbusd
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Modbus daemon by Teltonika
	DEPENDS:=+libuci +libtlt_uci +libubus +libubox +libmdcollect +libgsm \
		+vuci-app-modbus-slave +IO_SUPPORT:iomand +libmnfinfo +USE_GLIBC:glib2

	ifeq (m, $(CONFIG_PACKAGE_modbusd))
		PKG_TLT_NAME:=MODBUS TCP Slave
		PKG_ROUTER:=$(TLT_PLATFORM_NAME)
	endif
endef

ifeq ($(CONFIG_USE_GLIBC),y)
TARGET_CPPFLAGS +=-Dstrlcpy=g_strlcpy
TARGET_LDFLAGS +=-lglib-2.0
endif

define Package/modbusd/description
	Modbus daemon by Teltonika
endef

define Package/modbusd/conffiles
/etc/config/modbus
endef


define Package/modbusd/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/modbus.config $(1)/etc/config/modbus
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/modbusd.init $(1)/etc/init.d/modbusd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/modbusd $(1)/usr/sbin/modbusd
endef

$(eval $(call BuildPackage,modbusd))
