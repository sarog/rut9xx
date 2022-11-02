#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libpam
PKG_VERSION:=1.5.1
PKG_RELEASE:=1

PKG_SOURCE:=Linux-PAM-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/linux-pam/linux-pam/releases/download/v$(PKG_VERSION)
PKG_HASH:=201d40730b1135b1b3cdea09f2c28ac634d73181ccd0172ceddee3649c5792fc
PKG_BUILD_DIR:=$(BUILD_DIR)/Linux-PAM-$(PKG_VERSION)

PKG_MAINTAINER:=Nikos Mavrogiannopoulos <n.mavrogiannopoulos@gmail.com>
PKG_LICENSE:=BSD-3c GPL
PKG_LICENSE_FILES:=COPYING Copyright
PKG_CPE_ID:=cpe:/a:kernel:linux-pam

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libpam/Default
	SECTION:=libs
	CATEGORY:=Libraries
	URL:=http://www.kernel.org/pub/linux/libs/pam
endef

define Package/libpam
	$(call Package/libpam/Default)
 	 TITLE:=the Linux-PAM libraries and modules.
  	MENU:=1
endef

define Package/libpam/description
	The Linux-PAM Pluggable Authentication Modules.
endef

define Package/libpam-mod-unix
  $(call Package/libpam/Default)
  DEPENDS:=libpam
  TITLE:=Module for traditional password authentication
endef

define Package/libpam-mod-deny
  $(call Package/libpam/Default)
  DEPENDS:=libpam
  TITLE:=The locking-out PAM module
endef

define Package/libpam-mod-deny/description
	This module can be used to deny access. \
	It always indicates a failure to the application through the PAM framework
endef

define Package/libpam-mod-permit
  $(call Package/libpam/Default)
  DEPENDS:=libpam
  TITLE:=The promiscuous module
endef

define Package/libpam-mod-permit/description
	pam_permit is a PAM module that always permit access. It does nothing else.
endef

define Package/libpam-mod-umask
  $(call Package/libpam/Default)
  DEPENDS:=libpam
  TITLE:=PAM module to set the file mode creation mask
endef
define Package/libpam-mod-extra
  $(call Package/libpam/Default)
  DEPENDS:=libpam
  TITLE:=Extra PAM modules
endef

CONFIGURE_ARGS += \
	--enable-pamlocking \
	--enable-shared \
	--enable-static \
	--disable-audit \
	--disable-cracklib \
	--disable-db \
	--disable-debug \
	--disable-doc \
	--disable-econf \
	--disable-lckpwdf \
	--disable-nis \
	--disable-prelude \
	--disable-regenerate-docu \
	--disable-rpath \
	--disable-selinux \
	--disable-Werror \
	--with-gnu-ld \
	--without-mailspool \
	--without-xauth \
	--enable-vendordir=/tmp

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/libpam/install
	$(INSTALL_DIR) $(1)/etc $(1)/etc/pam.d
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/lib/security $(1)/usr/lib/security/pam_filter
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/etc/* $(1)/etc/
	$(CP) ./files/* $(1)/etc/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/security/pam_filter/* $(1)/usr/lib/security/pam_filter/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/* $(1)/usr/sbin/
endef

define BuildPlugin
  define Package/libpam-mod-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib/security
	for m in $(2); do \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/security/pam_$$$$$$$${m}.so* $$(1)/usr/lib/security/ ;\
	done
  endef
  $$(eval $$(call BuildPackage,libpam-mod-$(1)))
endef

PAM_EXTRA_MODULES:= \
	access debug deny echo env exec faildelay fil filter ftp group issue keyinit \
	limits listfile localuser loginuid mail mkhomedir motd namespace nologin permit \
	pwhistory rootok securetty shells stress succeed_if tally2 tally time timestamp \
	umask unix warn wheel xauth \

$(eval $(call BuildPackage,libpam))
$(eval $(call BuildPlugin,unix,unix))
$(eval $(call BuildPlugin,deny,deny))
$(eval $(call BuildPlugin,permit,permit))
$(eval $(call BuildPlugin,umask,umask))
$(eval $(call BuildPlugin,extra,$(PAM_EXTRA_MODULES)))
