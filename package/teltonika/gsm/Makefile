#
# Copyright (C) 2022 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=gsm
PKG_VERSION:=2022-09-02
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=1b890b06c3a6e9b7f07bf28c8b498dd4e32d2b54

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

GPL_INCLUDE_HEADERS:=1

include $(INCLUDE_DIR)/package.mk

define Package/gsmd
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Simple server to route AT commands to modem
	DEPENDS:=+libusb-1.0 +libubus +libubox +libblobmsg-json +modems +libtlt_uci +libgsm +libuci +liblog +libjson-c +IO_SUPPORT:iomand +libmnfinfo
endef

define Package/gsmctl
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Sending AT commands by ubus to gsmd deamon app
	DEPENDS:=+libgsm +libubus +libubox +libblobmsg-json +gsmd +libjson-c +IO_SUPPORT:iomand +libmnfinfo
endef

define Package/libgsm
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=A library for manage gsm API. Read, list, delete, etc. For parse AT commands
	DEPENDS:=+libtlt_uci +liblog +libblobmsg-json +libubus +libmnfinfo +libuci +libubox +libjson-c +rpcd-mod-smslimit +rpcd-mod-simd +IO_SUPPORT:iomand
endef

define Package/modem-sync
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Automated time synchronization with operator station
	DEPENDS:=+libgsm +libubus +libubox +libblobmsg-json +gsmd +libjson-c
endef

define Package/gsmd/description
	Simple server to route GSM modem AT commands. Created by Teltonika
endef

define Package/gsmctl/description
	Simple executable application to execute and read GSM modem AT commands. Created by Teltonika
endef

define Package/modem-sync/description
	Simple daemon meant for time synchronization with operator station. Created by Teltonika
endef

define Package/libgsm/description
	A simple lib with AT commands parser. Created by Teltonika
endef

ifeq ($(CONFIG_MOBILE_SUPPORT),y)
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/*.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/include/*.h $(1)/usr/include/
endef
endif


define Package/libgsm/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libgsm.so $(1)/usr/lib/
endef

define Package/gsmd/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/config/ $(1)/etc/init.d/ $(1)/etc/hotplug.d/usb/ $(1)/etc/hotplug.d/iface/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gsmd $(1)/usr/sbin/gsmd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/unhandler $(1)/usr/sbin/unhandler
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/gsmd.init $(1)/etc/init.d/gsmd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/unhandler.init $(1)/etc/init.d/unhandler
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/gsmd.hotplug $(1)/etc/hotplug.d/usb/05_gsmd.sh
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/gsm_event.hotplug $(1)/etc/hotplug.d/iface/89-gsm-event
	$(INSTALL_CONF) /dev/null $(1)/etc/config/simcard
endef

define Package/gsmctl/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gsmctl $(1)/usr/sbin/gsmctl
endef

define Package/modem-sync/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/modem_sync $(1)/usr/sbin/modem_sync
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/modem_sync.init $(1)/etc/init.d/modem_sync
endef

$(eval $(call BuildPackage,libgsm))
$(eval $(call BuildPackage,gsmd))
$(eval $(call BuildPackage,gsmctl))
$(eval $(call BuildPackage,modem-sync))
