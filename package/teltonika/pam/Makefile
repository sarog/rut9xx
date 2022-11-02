
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=pamd
PKG_VERSION:=2022-01-03
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=886167d6b1d6f593202730ea060522f102906093
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/pamd
	SECTION:=utils
	CATEGORY:=Base system
	DEPENDS:=+rpcd +libubus +libubox +libpam
	TITLE:=Teltonika PAM support
	ifeq (m, $(CONFIG_PACKAGE_pamd))
  		PKG_TLT_NAME:=PAM
  		PKG_ROUTER:=$(TLT_PLATFORM_NAME)
  		PKG_FIRMWARE:=7.2
  	endif
endef

define Package/pamd/description
	This application provides PAM methods through ubus
endef


define Package/pamd/install
	$(INSTALL_DIR) $(1)/etc $(1)/usr/sbin/
	$(CP) ./files/* $(1)/etc/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pam $(1)/usr/sbin/pamd
endef

$(eval $(call BuildPackage,pamd))
