include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=luci-bwc
PKG_VERSION:=2021-11-12
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=c99b9024c308f050f489659e54082c043be4e260
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/luci-bwc
	SECTION:=vuci
	CATEGORY:=VuCI
	SUBMENU:=vuci-rpc-core
	DEPENDS:=+iwinfo
	TITLE:=A program for realtime bandwidth data collection 
endef

define Package/luci-bwc/description
	A program for realtime bandwidth data collection 
endef


define Package/luci-bwc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/src/luci-bwc $(1)/usr/bin/luci-bwc
endef

$(eval $(call BuildPackage,luci-bwc))
