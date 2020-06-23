include $(TOPDIR)/rules.mk

PKG_NAME:=telenor_mqtt
PKG_VERSION:=0.0.2
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_ROUTER:=RUT9
PKG_FIRMWARE:=06.05.0
PKG_TLT_NAME:=Telenor
PKG_VERSION_PM:=0.0.2

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/tlt_custom_pkg_telenor_mqtt
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=MQTT application for Telenor by Teltonika
	DEPENDS:=+libblobmsg-json +libmosquitto-ssl +libmnfinfo +libuci +libtlt_uci +libcurl +libeventslog +libgsm +libjson-c +libtlt_socket_man +libubus +libunhandler
endef

define Package/tlt_custom_pkg_telenor_mqtt/description
	MQTT application for Telenor by Teltonika
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/tlt_custom_pkg_telenor_mqtt/conffiles
/etc/config/telenor_mqtt
endef

define Package/tlt_custom_pkg_telenor_mqtt/postinst
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@telenor_mqtt[-1]
add ucitrack telenor_mqtt
set ucitrack.@telenor_mqtt[-1].init='telenor_mqtt'
commit ucitrack
endef

define Package/tlt_custom_pkg_telenor_mqtt/postrm
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@telenor_mqtt[-1]
commit ucitrack
endef

define Package/tlt_custom_pkg_telenor_mqtt/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/telenor_mqtt $(1)/usr/sbin/telenor_mqtt

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/telenor_mqtt.init $(1)/etc/init.d/telenor_mqtt
	chmod 755 $(1)/etc/init.d/telenor_mqtt

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) files/telenor_mqtt.conf $(1)/etc/config/telenor_mqtt

	$(INSTALL_DIR) $(1)/etc/telenor_mqtt

	#WEB files
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/tlt-telenor_mqtt
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-telenor_mqtt/luasrc/controller/telenor-mqtt.lua $(1)/usr/lib/lua/luci/controller/telenor-mqtt.lua
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-telenor_mqtt/luasrc/model/cbi/telenor-mqtt.lua $(1)/usr/lib/lua/luci/model/cbi/telenor-mqtt.lua
endef

$(eval $(call BuildPackage,tlt_custom_pkg_telenor_mqtt))
