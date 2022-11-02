#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=libbrand
PKG_VERSION:=2020-01-06
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=5888477baec6581e3a052958bda5564294257586
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/libbrand/Default
  SECTION:=base
  CATEGORY:=Base system
  SUBMENU:=Branding
  DEPENDS:=+base-files
endef

define Package/libbrand-versioning
  $(call Package/libbrand/Default)
  TITLE:=Teltonika version file
  MENU:=1
endef

define Package/libbrand-versioning/config
	source "$(SOURCE)/Config.in"
endef



define Package/libbrand-versioning/install
	@-rm $(PKG_BUILD_DIR)/.built; \
	echo $$(CONFIG_TLT_VERS) > $(PKG_BUILD_DIR)/version; \
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/version $(1)/etc/version; \
	rm -f $(PKG_BUILD_DIR)/version;
endef

CONFIG_BRAND_PREFIX:=$(CONFIG_TLT_VERSIONING_PREFIX)
CONFIG_BRAND_CLIENT:=$(CONFIG_TLT_VERSIONING_CLIENT)

ifeq ($(CONFIG_TLT_VERSIONING_MANUAL_ENABLE),y)
	CONFIG_TLT_VERS:=$(CONFIG_BRAND_PREFIX)_$(CONFIG_TLT_VERSIONING_RELEASE)

	ifneq ($(CONFIG_TLT_VERSIONING_BRANCH_PREFIX),"-")
		CONFIG_TLT_VERS:=$(CONFIG_TLT_VERS)_$(CONFIG_TLT_VERSIONING_BRANCH_PREFIX)
	endif

	CONFIG_TLT_VERS:=$(CONFIG_TLT_VERS)_$(CONFIG_BRAND_CLIENT).$(CONFIG_TLT_VERSIONING_MAJOR).$(CONFIG_TLT_VERSIONING_MINOR)

	ifeq ($(CONFIG_TLT_VERSIONING_PATCH_ENABLE),y)
		CONFIG_TLT_VERS:=$(CONFIG_TLT_VERS).$(CONFIG_TLT_VERSIONING_PATCH)
	endif
else
	CONFIG_BRAND_SCRIPT_PARAMS:=--brand-prefix $(CONFIG_BRAND_PREFIX) --brand-number $(CONFIG_BRAND_CLIENT)

	ifneq ($(CONFIG_TLT_VERSIONING_BRANCH_PREFIX),"-")
		CONFIG_BRAND_SCRIPT_PARAMS:=$(CONFIG_BRAND_SCRIPT_PARAMS) --branch-prefix $(CONFIG_TLT_BRANCH_PREFIX)
	endif

	ifeq ($(CONFIG_TLT_VERSIONING_GPL),y)
		CONFIG_BRAND_SCRIPT_PARAMS:=$(CONFIG_BRAND_SCRIPT_PARAMS) --gpl
	endif

	CONFIG_TLT_VERS:=$$($(PKG_BUILD_DIR)/files/get_firmware_name.sh $(CONFIG_BRAND_SCRIPT_PARAMS))
endif

$(eval $(call BuildPackage,libbrand-versioning))
