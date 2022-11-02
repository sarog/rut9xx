#
# Copyright (C) 2021 teltonika.lt
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=email_to_sms
PKG_VERSION:=2021-11-12
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=0575c348e000cb8b0791a20719f68a0fc2cf7953
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/email_to_sms
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=POP3 email to SMS
	DEPENDS:=+lua-pop3
endef

define Package/email_to_sms/description
 Enable this option in order to include POP3 email to SMS
endef



define Package/email_to_sms/install
		$(INSTALL_DIR) $(1)/etc/config/
		$(INSTALL_DIR) $(1)/etc/init.d/
		$(INSTALL_DIR) $(1)/usr/bin/
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/email_to_sms.init $(1)/etc/init.d/email_to_sms
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/email_to_sms.lua $(1)/usr/bin/email_to_sms
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/files/email_to_sms.conf $(1)/etc/config/email_to_sms
endef

$(eval $(call BuildPackage,email_to_sms))
