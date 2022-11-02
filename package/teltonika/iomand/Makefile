#
# Copyright (C) 2022 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=iomand

PKG_SOURCE_VERSION:=b05b83c578a8ff20cc78afa0c12db8bad9e12fa4

include $(INCLUDE_DIR)/package.mk

define Package/iomand
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=Daemon providing I/O related functionality.
	DEPENDS:=+libuci +libubus +libubox +libcgi +libmnfinfo +libtlt-logger
endef

define Package/iomand/conffiles
/etc/config/ioman
endef


define Package/iomand/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/config $(1)/etc/init.d $(1)/etc/hotplug.d/usb \
	 	$(1)/www/cgi-bin $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iomand $(1)/usr/bin/iomand
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scheduler $(1)/usr/bin/scheduler
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/post_get_state $(1)/www/cgi-bin/io_state
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/post_get_type $(1)/www/cgi-bin/io_type
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/post_get_value $(1)/www/cgi-bin/io_value
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/iomand.init $(1)/etc/init.d/ioman
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/ioman_scheduler.init $(1)/etc/init.d/ioman_scheduler
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/03-ioman_scheduler_migrate.sh $(1)/etc/uci-defaults/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/io_post_get_migration.sh $(1)/etc/uci-defaults/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/disable_io_features.sh $(1)/etc/uci-defaults/

	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/libioman.so $(1)/usr/lib/

	if [ $(TLT_PLATFORM_RUTX) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/ioman_RUTX.config $(1)/etc/config/ioman; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/ioman_RUTX_config.sh $(1)/etc/uci-defaults/; \
	elif [ $(TLT_PLATFORM_TRB1) ] || [ $(TLT_PLATFORM_TRB5) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/ioman_TRB14x.config $(1)/etc/config/ioman; \
	elif [ $(TLT_PLATFORM_TRB2) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/ioman_TRB2.config $(1)/etc/config/ioman; \
	elif [ $(TLT_PLATFORM_RUT30X) ] || [ $(TLT_PLATFORM_RUT36X) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/ioman_RUT3.config $(1)/etc/config/ioman; \
	elif [ $(TLT_PLATFORM_RUT9) ] || [ $(TLT_PLATFORM_RUT9M) ] || [ $(TLT_PLATFORM_RUT952) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/ioman_RUT9.config $(1)/etc/config/ioman; \
	elif [ $(TLT_PLATFORM_RUT2) ] || [ $(TLT_PLATFORM_RUT2M) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/ioman_RUT2.config $(1)/etc/config/ioman; \
	fi;

endef

ifeq ($(CONFIG_IO_SUPPORT),y)
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/libioman.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/include/libioman.h $(1)/usr/include/
endef
endif

$(eval $(call BuildPackage,iomand))
