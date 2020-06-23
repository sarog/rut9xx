#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=cmStreamApp
PKG_VERSION:=0.0.2
#PKG_RELEASE:=1
PKG_ROUTER:=RUT9
PKG_FIRMWARE:=06.06.0
PKG_TLT_NAME:=Cumulocity
PKG_VERSION_PM:=0.0.2

include $(INCLUDE_DIR)/package.mk

define Package/tlt_custom_pkg_cmstreamapp
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Cumulocity Stream application.
	DEPENDS:=+libsera +libuci
endef

define Package/tlt_custom_pkg_cmstreamapp/description
	A simple Cumulocity IoT StreamApp package.
endef

define Package/tlt_custom_pkg_cmstreamapp/conffiles
/etc/config/iot
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/tlt_custom_pkg_cmstreamapp/postinst
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@iot[-1]
add ucitrack iot
set ucitrack.@iot[-1].init='cud'
commit ucitrack
endef

define Package/tlt_custom_pkg_cmstreamapp/postrm
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@iot[-1]
commit ucitrack
endef

define Package/tlt_custom_pkg_cmstreamapp/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cmStreamApp $(1)/usr/bin/cmStreamApp
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/cud.init $(1)/etc/init.d/cud
	$(INSTALL_DIR) $(1)/usr/lib/lua/cm
	$(INSTALL_BIN) ./files/srtemplate $(1)/usr/lib/lua/cm/srtemplate.txt
	$(INSTALL_BIN) ./files/stream.lua $(1)/usr/lib/lua/cm/stream.lua
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) ./files/iot $(1)/etc/config/iot

	#for package manager
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-iot/luasrc/controller/cumulocity.lua $(1)/usr/lib/lua/luci/controller/cumulocity.lua
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-iot/luasrc/model/cbi/cumulocity.lua $(1)/usr/lib/lua/luci/model/cbi/cumulocity.lua

endef

$(eval $(call BuildPackage,tlt_custom_pkg_cmstreamapp))
