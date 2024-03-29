#
# Copyright (C) 2013-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rpcd
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/rpcd.git
PKG_SOURCE_DATE:=2021-03-11
PKG_SOURCE_VERSION:=ccb75178cf6a726896729c6904bd623636aa0b29
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_MIRROR_HASH:=87b1839275c209f9767057d6da5272cae973a064767fa28f380a37fb65e2e643

PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1
PKG_ASLR_PIE_REGULAR:=1

PKG_CONFIG_DEPENDS:= CONFIG_RPCD_PAM

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/rpcd $(1)/usr/include/
endef

define Package/rpcd/default
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=OpenWrt ubus RPC backend server
  DEPENDS:=+libubus +libubox +liblog
endef

define Package/rpcd
  $(Package/rpcd/default)
  DEPENDS+= +libuci +libblobmsg-json +libjson-c
  MENU:=1
endef

define Package/rpcd/config
  source "$(SOURCE)/Config.in"
endef

define Package/rpcd/description
 This package provides the UBUS RPC backend server to expose various
 functionality to frontend programs via JSON-RPC.
endef

define Package/rpcd/conffiles
/etc/config/rpcd
endef

TARGET_LDFLAGS += -lcrypt

define Package/rpcd/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/rpcd.init $(1)/etc/init.d/rpcd
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rpcd $(1)/sbin/rpcd
	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/unauthenticated.json $(1)/usr/share/rpcd/acl.d/unauthenticated.json
	$(INSTALL_BIN) ./files/superuser.json $(1)/usr/share/rpcd/acl.d/superuser.json
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/rpcd.config $(1)/etc/config/rpcd
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/50-migrate-rpcd-ubus-sock.sh $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/51-migrate-admin-user.sh $(1)/etc/uci-defaults
	if [ $(TLT_PLATFORM_RUTX) ]; then \
		$(SED) "s@%%USER_SCRIPTS_HOTSPOT%%@list read 'services/hotspot/general/userscripts'@g" \
			$(1)/etc/config/rpcd; \
	else \
		$(SED) "/%%USER_SCRIPTS_HOTSPOT%%/d" $(1)/etc/config/rpcd; \
	fi

	if [ ! $(TLT_PLATFORM_RUT30X) ]; then \
		$(SED) "s@%%MOBILE_UTILS_SMS_SEND%%@list read 'services/mobile_utilities/sms/send'@g" \
			$(1)/etc/config/rpcd; \
	else \
		$(SED) "/%%MOBILE_UTILS_SMS_SEND%%/d" $(1)/etc/config/rpcd; \
	fi
endef


# 1: plugin name
# 2: extra dependencies
# 3: plugin title/description
define BuildPlugin

  PKG_CONFIG_DEPENDS += CONFIG_PACKAGE_luci-rpc-mod-$(1)

  define Package/rpcd-mod-$(1)
    $(Package/rpcd/default)
    TITLE+= ($(1) plugin)
    DEPENDS+=rpcd $(2)
  endef

  define Package/rpcd-mod-$(1)/description
    $(3)
  endef

  define Package/rpcd-mod-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib/rpcd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(1).so $$(1)/usr/lib/rpcd/
  endef

  $$(eval $$(call BuildPackage,rpcd-mod-$(1)))

endef

CMAKE_OPTIONS += -DPAM_SUPPORT=$(if $(CONFIG_RPCD_PAM),"ON","OFF")

$(eval $(call BuildPackage,rpcd))
$(eval $(call BuildPlugin,file,,Provides ubus calls for file and directory operations.))
$(eval $(call BuildPlugin,rpcsys,,Provides ubus calls for sysupgrade and password changing.))
$(eval $(call BuildPlugin,iwinfo,+libiwinfo,Provides ubus calls for accessing iwinfo data.))
