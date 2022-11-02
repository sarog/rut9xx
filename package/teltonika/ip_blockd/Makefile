include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk
  
PKG_NAME:=ip_blockd
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=84114bad538c1b59d9c3ed7560a140972a65ac89

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/ip_blockd
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=IP Block Daemon
	DEPENDS:=+rpcd +libubus +libubox +libuci +liblog +libip4tc +libip6tc libtlt-logger
endef

define Package/ip_blockd/description
	IP block daemon
endef


define Package/ip_blockd/install
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DIR)  $(1)/usr/lib/rpcd
	$(INSTALL_DIR)  $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/insertIpTables.sh $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/ip_blockd.conf $(1)/etc/config/ip_blockd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ip_blockd.so $(1)/usr/lib/rpcd/
endef

$(eval $(call BuildPackage,ip_blockd))
