#
# Copyright (C) 2022 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=mdcollect
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=b40721dfa4304e71bf6d7f9c0a99b28094470781
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

GPL_INCLUDE_HEADERS:=1

include $(INCLUDE_DIR)/package.mk

define Package/mdcollectd
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=mdcollectd
	DEPENDS:=+lsqlite3 +libuci +libtlt_uci +libmdcollect +libubus +libubox \
		+libtlt-logger +libgsm +iptables-mod-quota2 +rpcd-mod-simd
endef

define Package/libmdcollect
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A library for mdcollectd
	DEPENDS:=+lsqlite3 +libjson-c +libubus +libubox +libblobmsg-json +rpcd-mod-simd
endef

define Package/mdcollectd/conffiles
/etc/config/mdcollectd
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/lib $(STAGING_DIR)/usr/include
	$(CP) $(PKG_BUILD_DIR)/*.so $(STAGING_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/include/*.h $(STAGING_DIR)/usr/include/
endef


define Package/mdcollectd/install
	$(INSTALL_DIR) $(1)/etc/init.d/ $(1)/etc/config/ $(1)/etc/uci-defaults/  \
		$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/mdcollectd.config $(1)/etc/config/mdcollectd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/mdcollectd.init $(1)/etc/init.d/mdcollectd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/99-mdcollect_config_migration.sh $(1)/etc/uci-defaults/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mdcollectd $(1)/usr/bin/mdcollectd
endef

define Package/libmdcollect/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libmdcollect.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,mdcollectd))
$(eval $(call BuildPackage,libmdcollect))
