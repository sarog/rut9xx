#
# Copyright (C) 2022 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=ledman
PKG_VERSION:=2022-09-02
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=51a3f0a2980a8b8652b6ce95c9ebba2d247be768

include $(INCLUDE_DIR)/package.mk

define Package/ledman
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=LED status manager
	DEPENDS:=+libubus +libubox +libgsm +libtlt-logger +libuci +libmnfinfo +TARGET_ipq40xx_generic_DEVICE_teltonika_rutx:rpcd-mod-simd
endef


define Package/ledman/install
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/usr/bin $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/ledman.init $(1)/etc/init.d/ledman
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/broadcast-event.hotplug $(1)/etc/hotplug.d/iface/01-broadcast-event
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ledman $(1)/usr/bin/ledman
endef

$(eval $(call BuildPackage,ledman))
