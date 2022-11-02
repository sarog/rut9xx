#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=events_reporting
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=aa4781c4887949e03701a63dab429397f9f33c32
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/events_reporting
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=Events Reporting
	DEPENDS:=+USE_GLIBC:glib2 +libubus +libubox  +libtlt-logger \
		+libtlt_uci +libgsm +librms_mqtt +libtlt_utils
endef

ifeq ($(CONFIG_USE_GLIBC),y)
TARGET_CPPFLAGS +=-Dstrlcpy=g_strlcpy
TARGET_LDFLAGS +=-lglib-2.0
endif


define Package/events_reporting/install
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/etc/config $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/events_reporting.init $(1)/etc/init.d/events_reporting
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/events_reporting.conf $(1)/etc/config/events_reporting
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/events_reporting $(1)/usr/bin/events_reporting
endef

$(eval $(call BuildPackage,events_reporting))
