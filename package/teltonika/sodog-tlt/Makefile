include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=sodog-tlt
PKG_SOURCE_VERSION:=89630224ec6d567ea47dc807719dc7a05f752077

include $(INCLUDE_DIR)/package.mk

define Package/sodog-tlt
	SECTION:=net
	CATEGORY:=Network
	TITLE:=SOdog establishes two bidirectional byte streams and transfers data between them by Teltonika.
	DEPENDS:=+kmod-usb-serial
endef

define Package/sodog-tlt/description
	SOdog establishes two bidirectional byte streams and transfers data between them by Teltonika.
endef


define Package/sodog-tlt/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d $(1)/etc/config $(1)/etc/hotplug.d/tty
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sodog $(1)/usr/sbin/sodog
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/01-serial-symlink.sh $(1)/etc/hotplug.d/tty/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/usb_serial_adapters.sh $(1)/usr/sbin/
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/rs.config $(1)/etc/config/rs

	if [ $(TLT_PLATFORM_RUTX) ]; then \
		$(INSTALL_DIR) $(1)/etc/uci-defaults; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/keep_rs232_section_on_rutxr1.sh $(1)/etc/uci-defaults/; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/migration_rs232_usb_hash.sh $(1)/etc/uci-defaults/; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rs232.init $(1)/etc/init.d/rs232; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rs232_usb.init $(1)/etc/init.d/rs232_usb; \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/rutx_rs.config $(1)/etc/config/rs; \
	elif [ $(TLT_PLATFORM_TRB2) ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rs232.init $(1)/etc/init.d/rs232; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rs485.init $(1)/etc/init.d/rs485; \
	elif [ $(TLT_PLATFORM_RUT30X) ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rs232_usb.init $(1)/etc/init.d/rs232_usb; \
	elif [ $(TLT_PLATFORM_TRB1) ]; then \
		$(INSTALL_DIR) $(1)/etc/uci-defaults; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/keep_rs232_rs485_sections_on_trb1.sh $(1)/etc/uci-defaults/; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rs232.init $(1)/etc/init.d/rs232; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rs485.init $(1)/etc/init.d/rs485; \
	elif [ $(TLT_PLATFORM_RUT9) ] || [ $(TLT_PLATFORM_RUT9M) ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rs232.init $(1)/etc/init.d/rs232; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rs485.init $(1)/etc/init.d/rs485; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rs232_usb.init $(1)/etc/init.d/rs232_usb; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/symlink-soc-rs485.init $(1)/etc/init.d/symlink-soc-rs485; \
	fi;
endef

$(eval $(call BuildPackage,sodog-tlt))
