include $(TOPDIR)/rules.mk

PKG_NAME:=host-block
PKG_RELEASE:=1
PKG_VERSION:=1

include $(INCLUDE_DIR)/package.mk

define Package/host-block
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=Host Block
	DEPENDS:=+dnsmasq-dhcpv6
endef

define Package/host-block/description
	Host Blocker.
endef

define Package/host-block/conffiles
	/etc/config/hostblock
endef

define Build/Prepare

endef

define Build/Compile

endef

define Package/host-block/install
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/hostblock.init $(1)/etc/init.d/hostblock
	$(INSTALL_DATA) ./files/hostblock.conf $(1)/etc/config/hostblock
	$(INSTALL_BIN) ./files/hostblock.sh $(1)/usr/sbin/hostblock.sh
endef

$(eval $(call BuildPackage,host-block))
