# Copyright (C) 2012-2016 PIVA Software <www.pivasoftware.com>
# 	Author: MOHAMED Kallel <mohamed.kallel@pivasoftware.com>
# 	Author: ANIS ELLOUZE <anis.ellouze@pivasoftware.com>

include $(TOPDIR)/rules.mk

PKG_NAME:=easycwmp
PKG_VERSION:=1.6.1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://easycwmp.org/download/
PKG_ROUTER:=RUT9
PKG_FIRMWARE:=06.05.0
PKG_TLT_NAME:=TR-069
PKG_VERSION_PM:=1.6.5

PKG_FIXUP:=autoreconf

PKG_CONFIG_DEPENDS:= \
	CONFIG_EASYCWMP_DEBUG \
	CONFIG_EASYCWMP_DEVEL

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/tlt_custom_pkg_easycwmp
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=CWMP client (using libcurl)
  DEPENDS:=+libubus +libuci +libubox +libmicroxml +libjson-c +libcurl
endef

define Package/tlt_custom_pkg_easycwmp/description
 An open source implementation of the client CWMP (TR-069) protocol
endef

define Package/tlt_custom_pkg_easycwmp/config
	source "$(SOURCE)/Config.in"
endef

USE_LOCAL=$(shell ls ./src/ 2>/dev/null >/dev/null && echo 1)
ifneq ($(USE_LOCAL),)
define Build/Prepare
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef
endif

TARGET_CFLAGS += \
	-D_GNU_SOURCE

TARGET_LDFLAGS += \
	-Wl,-rpath-link=$(STAGING_DIR)/usr/lib

CONFIGURE_ARGS += \
	--with-uci-include-path=$(STAGING_DIR)/usr/include \
	--with-libubox-include-path=$(STAGING_DIR)/usr/include \
	--with-libubus-include-path=$(STAGING_DIR)/usr/include

ifeq ($(CONFIG_EASYCWMP_DEBUG),y)
CONFIGURE_ARGS += \
	--enable-debug
endif

ifeq ($(CONFIG_EASYCWMP_DEVEL),y)
CONFIGURE_ARGS += \
	--enable-devel
endif

ifeq ($(CONFIG_EASYCWMP_BACKUP_DATA_CONFIG),y)
CONFIGURE_ARGS += \
	--enable-backupdatainconfig
endif

CONFIGURE_ARGS += \
	--enable-jsonc=1

define Package/tlt_custom_pkg_easycwmp/conffiles
/etc/config/easycwmp
/usr/share/easycwmp/defaults
endef

define Package/tlt_custom_pkg_easycwmp/postinst
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@easycwmp[-1]
add ucitrack easycwmp
set ucitrack.@easycwmp[-1].init='easycwmpd'
add_list ucitrack.@easycwmp[-1].affects='firewall'
commit ucitrack
endef

define Package/tlt_custom_pkg_easycwmp/postrm
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@easycwmp[-1]
commit ucitrack
endef

define Package/tlt_custom_pkg_easycwmp/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/easycwmpd $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/ext/openwrt/config/easycwmp $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ext/openwrt/init.d/easycwmpd $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/easycwmp
ifeq ($(CONFIG_EASYCWMP_SCRIPTS_FULL),y)
	$(INSTALL_DIR) $(1)/usr/share/easycwmp/functions/
	$(CP) $(PKG_BUILD_DIR)/ext/openwrt/scripts/defaults $(1)/usr/share/easycwmp
	$(CP) $(PKG_BUILD_DIR)/ext/openwrt/scripts/functions/common/* $(1)/usr/share/easycwmp/functions/
	$(CP) $(PKG_BUILD_DIR)/ext/openwrt/scripts/functions/custom/* $(1)/usr/share/easycwmp/functions/
ifeq ($(CONFIG_EASYCWMP_DATA_MODEL_TR181),y)
	$(CP) $(PKG_BUILD_DIR)/ext/openwrt/scripts/functions/tr181/* $(1)/usr/share/easycwmp/functions/
else
	$(CP) $(PKG_BUILD_DIR)/ext/openwrt/scripts/functions/tr098/* $(1)/usr/share/easycwmp/functions/
endif
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ext/openwrt/scripts/easycwmp.sh $(1)/usr/sbin/easycwmp
	chmod +x $(1)/usr/share/easycwmp/functions/*
else
	$(INSTALL_DIR) $(1)/usr/share/easycwmp/functions/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ext/openwrt/scripts/functions/common/ipping_launch $(1)/usr/share/easycwmp/functions/ipping_launch
endif

	#for package manager
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/tr069
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-tr069/luasrc/controller/tr069.lua $(1)/usr/lib/lua/luci/controller/tr069.lua
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-tr069/luasrc/model/cbi/tr069/tr069.lua $(1)/usr/lib/lua/luci/model/cbi/tr069/tr069.lua
endef

$(eval $(call BuildPackage,tlt_custom_pkg_easycwmp))
