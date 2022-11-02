#
# Copyright (C) 2022 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=quota_limit
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=df68008b206871313e0b27b7770405d10293acf3
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/quota_limit
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=quota_limit
	DEPENDS:=+libuci +libtlt_uci +libgsm +libtlt-logger +iptables-mod-quota2
endef

define Package/quota_limit/description
	3G data counter
endef

define Package/quota_limit/conffiles
/etc/config/quota_limit
endef


define Package/quota_limit/install
	$(INSTALL_DIR) $(1)/etc/init.d/ $(1)/etc/config/ $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/quota_limit.init $(1)/etc/init.d/quota_limit
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/quota_limit $(1)/usr/bin/quota_limit
endef

$(eval $(call BuildPackage,quota_limit))
