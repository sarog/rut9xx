include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=udprelay
PKG_VERSION:=2019-09-18
PKG_RELEASE:=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/nomeata/udp-broadcast-relay.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=94014fbbb3d09d69f3d8dd4a228729c17e88007c
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/udprelay
  SECTION:=net
  CATEGORY:=Network
  TITLE:=UDP packages relay
endef

define Package/udprelay/description
	This program listens for packets on a specified UDP broadcast port. \
	When a packet is received, it sends that packet to all specified interfaces \
	but the one it came from as though it originated from the original sender.
endef

define Package/udprelay/conffiles
	/etc/config/udprelay
endef

define Package/udprelay/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/udp-broadcast-relay $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/udprelay.init $(1)/etc/init.d/udprelay
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/udprelay.config $(1)/etc/config/udprelay
endef

$(eval $(call BuildPackage,udprelay))
