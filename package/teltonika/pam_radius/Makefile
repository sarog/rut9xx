
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=pam_radius
PKG_VERSION:=2022-05-06
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=10dbbc850953211a4b4dc4a7a822aa9266928b10
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/pam_radius
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=Radius server PAM module
  DEPENDS:=+libpam
endef


define Package/pam_radius/install
	$(INSTALL_DIR) $(1)/usr/lib/security/ $(1)/etc/raddb/
	$(CP) $(PKG_BUILD_DIR)/pam_radius_auth.so $(1)/usr/lib/security/
	#$(CP) $(PKG_BUILD_DIR)/pam_radius_auth.conf $(1)/etc/raddb/server
endef

$(eval $(call BuildPackage,pam_radius))
