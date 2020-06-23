#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tlt_custom_pkg_azure_iothub
PKG_VERSION:=0.0.3
PKG_BUILD_DIR:=$(BUILD_DIR)/tlt_custom_pkg_azure_iothub
PKG_ROUTER:=RUT9
PKG_FIRMWARE:=06.05.0
PKG_TLT_NAME:=Azure IoThub
PKG_VERSION_PM:=0.0.3

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/tlt_custom_pkg_azure_iothub
	SECTION:=base
	CATEGORY:=Base system
	DEPENDS:= +libc +USE_EGLIBC:librt +USE_EGLIBC:libpthread +libuci +libgsm +libubus +libtlt_base +libopenssl +libblobmsg-json +libuuid +libmosquitto
	TITLE:= Azure IoT Mqtt client
endef


define Package/tlt_custom_pkg_azure_iothub/description
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/tlt_custom_pkg_azure_iothub/conffiles
/etc/config/azure_iothub
endef

define Package/tlt_custom_pkg_azure_iothub/postinst
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@azure_iothub[-1]
add ucitrack azure_iothub
set ucitrack.@azure_iothub[-1].init='azure_iothub'
commit ucitrack
endef

define Package/tlt_custom_pkg_azure_iothub/postrm
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@azure_iothub[-1]
commit ucitrack
endef

define Package/tlt_custom_pkg_azure_iothub/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/teltonika_client/azure_iothub $(1)/sbin/azure_iothub

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/teltonika_client/azure_iothub.init $(1)/etc/init.d/azure_iothub

	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/teltonika_client/azure_iothub.conf $(1)/etc/config/azure_iothub

	#for package manager
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/azure_iothub
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-azure_iothub/luasrc/controller/azure_iothub.lua $(1)/usr/lib/lua/luci/controller/azure_iothub.lua
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-azure_iothub/luasrc/model/cbi/azure_iothub.lua $(1)/usr/lib/lua/luci/model/cbi/azure_iothub.lua
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-azure_iothub/luasrc/view/azure_iothub/azure_iothub.htm $(1)/usr/lib/lua/luci/view/azure_iothub/azure_iothub.htm

endef

$(eval $(call BuildPackage,tlt_custom_pkg_azure_iothub))
