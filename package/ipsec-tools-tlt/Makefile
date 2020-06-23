include $(TOPDIR)/rules.mk

PKG_NAME:=ipsec-tools-tlt
PKG_RELEASE:=1
PKG_VERSION:=1

include $(INCLUDE_DIR)/package.mk

define Package/ipsec-tools-tlt
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=VPN
	DEPENDS:=+ipsec-tools
	TITLE:=IPsec management tools Teltonika extension
endef

define Package/ipsec-tools-tlt/description
 This package contains the hw watchdog script for the RUT5xx device.
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/ipsec-tools-tlt/install
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/etc/init.d/racoon $(1)/etc/init.d/racoon
	$(INSTALL_DIR) $(1)/sbin/
	$(INSTALL_BIN) ./files/sbin/racoon-tracker $(1)/sbin/racoon-tracker
	$(INSTALL_DIR) $(1)/etc/racoon
	$(INSTALL_BIN) ./files/etc/racoon/* $(1)/etc/racoon/
	chmod 755 $(1)/etc/init.d/racoon $(1)/sbin/racoon-tracker $(1)/etc/racoon/racoon.sh $(1)/etc/racoon/firewall.sh
endef

$(eval $(call BuildPackage,ipsec-tools-tlt))
