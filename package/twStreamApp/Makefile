#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=twStreamApp
PKG_VERSION:=0.0.2
#PKG_RELEASE:=1
PKG_ROUTER:=RUT9
PKG_FIRMWARE:=06.05.0
PKG_TLT_NAME:=ThingWorx
PKG_VERSION_PM:=0.0.2

include $(INCLUDE_DIR)/package.mk

define Package/tlt_custom_pkg_twstreamapp
  SECTION:=net
  CATEGORY:=Network
  TITLE:=ThingWorx Stream application.
  DEPENDS:=+libtwCSdk
endef

define Package/tlt_custom_pkg_twstreamapp/description
	Teltonika ThingWorx API package.
endef

define Package/tlt_custom_pkg_twstreamapp/conffiles
/etc/config/iottw
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS) \
		TW_OS_INCLUDE="twLinux-openssl.h" \
		INCLUDE_DIR=$(STAGING_DIR)/usr/include/libtwCSdk
endef

define Package/tlt_custom_pkg_twstreamapp/postinst
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@iottw[-1]
add ucitrack iottw
set ucitrack.@iottw[-1].init='twd'
commit ucitrack
endef

define Package/tlt_custom_pkg_twstreamapp/postrm
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@iottw[-1]
commit ucitrack
endef

define Package/tlt_custom_pkg_twstreamapp/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/twStreamApp $(1)/usr/bin/twStreamApp
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/twd.init $(1)/etc/init.d/twd
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) ./files/iottw $(1)/etc/config/iottw

	#for package manager
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-iot/luasrc/controller/thingworx.lua $(1)/usr/lib/lua/luci/controller/thingworx.lua
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-iot/luasrc/model/cbi/thingworx.lua $(1)/usr/lib/lua/luci/model/cbi/thingworx.lua

	#libcsdk
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(BUILD_DIR)/libtwCSdk-0.1/src/libtwCSdk.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,tlt_custom_pkg_twstreamapp))
