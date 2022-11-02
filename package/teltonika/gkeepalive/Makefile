#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gkeepalive
PKG_VERSION:=1.0
PKG_RELEASE:=1


include $(INCLUDE_DIR)/package.mk

define Package/gkeepalive
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+libpcap +libpthread +libtlt-logger
  TITLE:=GRE keepalive
endef

define Package/gkeepalive/description
  GRE keepalive packet sender/receiver.
endef



define Package/gkeepalive/install
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/gre_tunnel.init $(1)/etc/init.d/gre_tunnel
	$(INSTALL_BIN) ./files/90-gre-keep-alive $(1)/etc/hotplug.d/iface/90-gre-keep-alive
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gkeepalive $(1)/usr/bin
endef

$(eval $(call BuildPackage,gkeepalive))
