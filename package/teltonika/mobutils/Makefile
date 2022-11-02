#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=mobutils
PKG_VERSION:=2022-09-02
PKG_RELEASE:=1

PKG_TLT_PORTABLE:=1

PKG_SOURCE_VERSION:=e4272c4e52917a5e9607a4c48e7c672dac40d9a2
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)
PKG_TLT_PORTABLE:=1

include $(INCLUDE_DIR)/package.mk

define Package/call_utilities
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=Call utilities
	DEPENDS:=+libgsm +libuci +libtlt_uci +libblobmsg-json +libubus +libusb-1.0
endef

define Package/sms_utilities
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=SMS utilities
	DEPENDS:=+libgsm +libtlt_uci +gsmd +libuci +libtlt-logger +libcgi \
		+libubox +IO_SUPPORT:iomand +libtlt_utils
endef

define Package/sms-post-get
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=SMS Send/Get using HTTP POST/GET requests
	DEPENDS:=+gsmd +libpthread +libgsm +libcgi
endef

define Package/call_utilities/conffiles
/etc/config/call_utils
endef

define Package/sms_utilities/conffiles
/etc/config/sms_utils
/etc/config/sms_gateway
endef


define Package/call_utilities/install
	$(INSTALL_DIR) $(1)/sbin/ $(1)/etc/config/ $(1)/etc/uci-defaults/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/call_utilities $(1)/sbin/call_utilities
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/call_utils.conf $(1)/etc/config/call_utils
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/99-call_utilities-config-migrate $(1)/etc/uci-defaults/
endef

define Package/sms_utilities/install
	$(INSTALL_DIR) $(1)/usr/sbin/ $(1)/etc/init.d/ $(1)/etc/config/ $(1)/etc/uci-defaults/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/sms_utils.init $(1)/etc/init.d/sms-utils
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/99-sms_utilities-config-migrate $(1)/etc/uci-defaults/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sms_utils $(1)/usr/sbin/sms_utils
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/sms_utils_add_action.sh $(1)/etc/uci-defaults/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/sms_utils_numbered_io_variables_migration.sh $(1)/etc/uci-defaults/98-sms_utils_numbered_io_variables

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/sms_gateway.init $(1)/etc/init.d/sms_gateway
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/sms_gateway.conf $(1)/etc/config/sms_gateway

	if [ $(TLT_PLATFORM_RUTX) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/sms_utils_rutx.conf $(1)/etc/config/sms_utils; \
	elif [ $(TLT_PLATFORM_TRB2) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/sms_utils_trb2.conf $(1)/etc/config/sms_utils; \
	elif [ $(TLT_PLATFORM_TRB1) ] || [ $(TLT_PLATFORM_TRB5) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/sms_utils_trb1.conf $(1)/etc/config/sms_utils; \
	elif [ $(TLT_PLATFORM_RUT36X) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/sms_utils_rut3.conf $(1)/etc/config/sms_utils; \
	elif [ $(TLT_PLATFORM_RUT30X) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/sms_utils_rut3.conf $(1)/etc/config/sms_utils; \
	elif [ $(TLT_PLATFORM_RUT9) ] || [ $(TLT_PLATFORM_RUT9M) ] || [ $(TLT_PLATFORM_RUT952) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/sms_utils_rut9.conf $(1)/etc/config/sms_utils; \
	elif [ $(TLT_PLATFORM_RUT2) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/sms_utils_rut2.conf $(1)/etc/config/sms_utils; \
	elif [ $(TLT_PLATFORM_TCR1) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/sms_utils_tcr1.conf $(1)/etc/config/sms_utils; \
	elif [ $(TLT_PLATFORM_RUT2M) ]; then \
		$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/sms_utils_rut2.conf $(1)/etc/config/sms_utils; \
	fi;
endef

define Package/sms-post-get/install
	$(INSTALL_DIR) $(1)/www $(1)/www/cgi-bin $(1)/etc/config/ $(1)/etc/uci-defaults/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sms_send $(1)/www/cgi-bin/sms_send
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sms_read $(1)/www/cgi-bin/sms_read
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sms_delete $(1)/www/cgi-bin/sms_delete
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sms_total $(1)/www/cgi-bin/sms_total
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sms_list $(1)/www/cgi-bin/sms_list
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mobile_data $(1)/www/cgi-bin/mobile_data
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/sms_post_get $(1)/etc/config/sms_post_get
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/sms_post_get_migration.sh $(1)/etc/uci-defaults/

endef

$(eval $(call BuildPackage,call_utilities))
$(eval $(call BuildPackage,sms_utilities))
$(eval $(call BuildPackage,sms-post-get))
