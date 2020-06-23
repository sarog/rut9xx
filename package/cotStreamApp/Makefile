#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=cotStreamApp
PKG_VERSION:=0.0.2
#PKG_RELEASE:=1
PKG_ROUTER:=RUT9
PKG_FIRMWARE:=06.06.0
PKG_TLT_NAME:=Cloud of Things
PKG_VERSION_PM:=0.0.2

include $(INCLUDE_DIR)/package.mk

define Package/tlt_custom_pkg_cotStreamApp
	SECTION:=net
	CATEGORY:=Network
	TITLE:=CoT Stream application.
	DEPENDS:=+libsera +libuci
endef

define Package/tlt_custom_pkg_cotStreamApp/description
	A simple Cloud of Things by Deutsche Telekom StreamApp package.
endef

define Package/tlt_custom_pkg_cotStreamApp/conffiles
/etc/config/cot
endef

define Package/tlt_custom_pkg_cotStreamApp/postinst
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@cot[-1]
add ucitrack cot
set ucitrack.@cot[-1].init='cot'
commit ucitrack
endef

define Package/tlt_custom_pkg_cotStreamApp/postrm
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@cot[-1]
commit ucitrack
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/tlt_custom_pkg_cotStreamApp/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cotStreamApp $(1)/usr/bin/cotStreamApp
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/cot.init $(1)/etc/init.d/cot
	$(INSTALL_DIR) $(1)/usr/lib/lua/cot
	$(INSTALL_BIN) ./files/srtemplate $(1)/usr/lib/lua/cot/srtemplate.txt
	$(INSTALL_BIN) ./files/stream.lua $(1)/usr/lib/lua/cot/stream.lua
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) ./files/cot $(1)/etc/config/cot

	#for package manager
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-iot/luasrc/controller/cot.lua $(1)/usr/lib/lua/luci/controller/cot.lua
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-iot/luasrc/model/cbi/cot.lua $(1)/usr/lib/lua/luci/model/cbi/cot.lua

endef

$(eval $(call BuildPackage,tlt_custom_pkg_cotStreamApp))
