include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=snmp-mod
PKG_VERSION:=2022-08-08
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=475a7c7977324742631a1c3279b2b855aa9c8958
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/snmp-mod/Default
	SUBMENU:=SNMP modules
	SECTION:=base
	CATEGORY:=Base system
	DEPENDS:=+libnetsnmp +libubus +libubox
endef

define Package/snmp
	$(call Package/snmp-mod/Default)
	DEPENDS+=+snmpd +vuci-app-snmp +snmp-mib +WIFI_SUPPORT:snmp-mod-chilli +snmp-mod-device +GPS_SUPPORT:snmp-mod-gps +MOBILE_SUPPORT:snmp-mod-gsm +IO_SUPPORT:snmp-mod-io
	TITLE:=SNMP Meta package

	ifeq (m, $(CONFIG_PACKAGE_snmp))
		PKG_TLT_NAME:=SNMP
		PKG_ROUTER:=$(TLT_PLATFORM_NAME)
	endif
endef

define Package/snmp-mod-chilli
	$(call Package/snmp-mod/Default)
	DEPENDS+=+libuci +libtltchilli
	TITLE:=SNMP coovachilli module
endef

TARGET_CFLAGS += $(if $(CONFIG_PACKAGE_snmp-mod-chilli),-DCHILLI_SUPPORT)

define Package/snmp-mod-device
	$(call Package/snmp-mod/Default)
	DEPENDS+=+libuci +libmnfinfo
	TITLE:=SNMP mnf info module
endef

define Package/snmp-mod-gps
	$(call Package/snmp-mod/Default)
	DEPENDS+=+libgps +libgsm
	TITLE:=SNMP GPS module
endef

define Package/snmp-mod-gsm
	$(call Package/snmp-mod/Default)
	DEPENDS+=+libgsm
	TITLE:=SNMP GSM module
endef

define Package/snmp-mod-io
	$(call Package/snmp-mod/Default)
	DEPENDS+=+iomand
	TITLE:=SNMP I/O module
endef

define Package/snmp-mib
	SUBMENU:=SNMP modules
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=SNMP teltonika MIBs
endef

define Package/snmp/install
	true
endef


define Package/snmp-mib/install
	$(INSTALL_DIR) $(1)/etc/snmp
	$(INSTALL_DIR) $(1)/etc/uci-defaults/

	if [ $(TLT_PLATFORM_RUTX) ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/RUTX_mib.sh $(1)/etc/uci-defaults/; \
	elif [ $(TLT_PLATFORM_TRB2) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/TRB2-MIB.txt $(1)/etc/snmp/MIB.txt; \
	elif [ $(TLT_PLATFORM_TRB1) ] || [ $(TLT_PLATFORM_TRB5) ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/TRB1_mib.sh $(1)/etc/uci-defaults/; \
	elif [ $(TLT_PLATFORM_RUT30X) ] || [ $(TLT_PLATFORM_RUT36X) ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/RUT3_mib.sh $(1)/etc/uci-defaults/; \
	elif [ $(TLT_PLATFORM_RUT9) ] || [ $(TLT_PLATFORM_RUT9M) ] || [ $(TLT_PLATFORM_RUT952) ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/RUT9_mib.sh $(1)/etc/uci-defaults/; \
	elif [ $(TLT_PLATFORM_TCR1) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/TCR1-MIB.txt $(1)/etc/snmp/MIB.txt; \
	elif [ $(TLT_PLATFORM_RUT2) ] || [ $(TLT_PLATFORM_RUT2M) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/RUT2-MIB.txt $(1)/etc/snmp/MIB.txt; \
	fi;
endef

define BuildPlugin
  define Package/snmp-mod-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib/snmpd-mod
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(1).so $$(1)/usr/lib/snmpd-mod/
  endef
  $$(eval $$(call BuildPackage,snmp-mod-$(1)))
endef

$(eval $(call BuildPackage,snmp))
$(eval $(call BuildPackage,snmp-mib))
$(eval $(call BuildPlugin,chilli))
$(eval $(call BuildPlugin,device))
$(eval $(call BuildPlugin,gps))
$(eval $(call BuildPlugin,gsm))
$(eval $(call BuildPlugin,io))
