include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=udprelay
PKG_VERSION:=2020-02-26
PKG_RELEASE:=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/nomeata/udp-broadcast-relay.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=a853a1430bade86c183a46e9d2271ed6d6c291bb
PKG_MIRROR_HASH:=95c81e42c179627adae359adf3d074a79e1cd14de0447e32db055a1cf1953c49
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/udprelay
  SECTION:=net
  CATEGORY:=Network
  TITLE:=UDP packages relay
  PKG_TLT_NAME:=UDP Broadcast Relay
  PKG_ROUTER:=$(TLT_PLATFORM_NAME)
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
