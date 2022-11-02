include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=dnp3
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=e3f058e1c747c39bd0143925e346d8803fbe2313
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/dnp3
	SECTION:=net
	CATEGORY:=Network
	TITLE:=dnp3 stack
	DEPENDS:=+libstdcpp +libuci +libubus +libopenssl +libsqlite3 +libgsm +GPS_SUPPORT:libgps \
		+libtlt_uci +libmdcollect +libjson-c +opendnp3 +vuci-app-dnp3
	PKG_TLT_NAME:=DNP3
	PKG_ROUTER:=$(TLT_PLATFORM_NAME)
endef

ifeq ($(CONFIG_GPS_SUPPORT),y)
CMAKE_OPTIONS=-DGPS_SUPPORT=TRUE
endif

define Package/dnp3/conffiles
/etc/config/dnp3_master
/etc/config/dnp3_outstation
endef

define Package/dnp3/description
	Teltonika dnp3 application stack
endef


define Package/dnp3/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dnp3_master $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dnp3_outstation $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/dnp3_master.conf $(1)/etc/config/dnp3_master
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/dnp3_outstation.conf $(1)/etc/config/dnp3_outstation

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/dnp3_outstation.init $(1)/etc/init.d/dnp3_outstation
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/dnp3_master.init $(1)/etc/init.d/dnp3_master
endef

$(eval $(call BuildPackage,dnp3))
