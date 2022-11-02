#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=web_filter
PKG_VERSION:=2021-02-11
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/web_filter
	CATEGORY:=Base system
	TITLE:=Meta package for Web filter packages
	DEPENDS:=+host_block +privoxy +vuci-app-webfilter

	ifeq (m, $(CONFIG_PACKAGE_web_filter))
		PKG_TLT_NAME:=Web filter
		PKG_ROUTER:=$(TLT_PLATFORM_NAME)
	endif
endef



define Package/web_filter/install
	true
endef

$(eval $(call BuildPackage,web_filter))
