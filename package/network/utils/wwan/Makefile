include $(TOPDIR)/rules.mk

PKG_NAME:=wwan
PKG_VERSION:=2019-04-29
PKG_RELEASE=5

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=John Crispin <john@phrozen.org>

include $(INCLUDE_DIR)/package.mk

define Package/wwan
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Generic OpenWrt 3G/4G proto handler
  DEPENDS:=+modems
endef

define Build/Compile
	true
endef

define Package/wwan/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto/
	$(CP) ./files/wwan.sh $(1)/lib/netifd/proto/
	$(INSTALL_BIN) ./files/wwan_up.sh $(1)/lib/netifd/wwan_up.sh
	$(INSTALL_DIR) $(1)/etc/hotplug.d/usb
	$(INSTALL_DIR) $(1)/lib/functions/
	$(INSTALL_DATA) ./files/modem.sh $(1)/lib/functions/
endef

$(eval $(call BuildPackage,wwan))
