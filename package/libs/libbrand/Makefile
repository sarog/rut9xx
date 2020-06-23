include $(TOPDIR)/rules.mk

PKG_NAME:=libbrand
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libbrand/Default
  SECTION:=base
  CATEGORY:=Base system
  SUBMENU:=Branding
endef

define Package/libbrand
  $(call Package/libbrand/Default)
  TITLE:=Web branding application
endef

define Package/libbrand-versioning
  $(call Package/libbrand/Default)
  TITLE:=Teltonika version file
  MENU:=1
endef

define Package/libbrand-versioning/config
	source "$(SOURCE)/Config.in"
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
	$(CP) $(TOPDIR)/brand/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += $(FPIC)

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/include/libbrand
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/libbrand
	$(CP) $(PKG_BUILD_DIR)/libbrand.so $(1)/usr/lib/
endef

define Package/libbrand/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libbrand.so $(1)/usr/lib/
	#first boot script
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/branding $(1)/etc/uci-defaults/
	$(CP) $(PKG_BUILD_DIR)/files/* $(1)/
	@if [ -f $(1)/etc/shadow ]; then \
	chmod 0600 $(1)/etc/shadow;\
	fi
	
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

	CONFIG_TLT_VERS:=$$(./src/get_firmware_name.sh $(CONFIG_BRAND_SCRIPT_PARAMS))
endif

$(eval $(call BuildPackage,libbrand))
$(eval $(call BuildPackage,libbrand-versioning))
