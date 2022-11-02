#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=port_eventsd
PKG_VERSION:=2022-09-02
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=f217ca11b82cf7b415cd952232f576e0a057c5e2
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/port_eventsd
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=Port Events Daemon
	DEPENDS:=+libjson-c +libblobmsg-json +libubox +libubus +libnl-tiny +liblog +libmnfinfo
endef

define Package/port_events/description
	Switch info parser and switch event reporter
endef

TARGET_CPPFLAGS := \
    -g3 \
	-D_GNU_SOURCE \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	-I$(PKG_BUILD_DIR) \
	$(TARGET_CPPFLAGS) \
	-I$(LINUX_DIR)/user_headers/include



define Package/port_eventsd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/port_eventsd $(1)/usr/bin/port_eventsd
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/port_eventsd.init $(1)/etc/init.d/port_eventsd
endef

$(eval $(call BuildPackage,port_eventsd))
