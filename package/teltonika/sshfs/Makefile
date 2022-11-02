#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sshfs
PKG_VERSION:=2.10

PKG_LICENSE:=GPL-2.0
PKG_MAINTAINER:=Zoltan HERPAI <wigyori@uid0.hu>

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/libfuse/sshfs/releases/download/$(PKG_NAME)-$(PKG_VERSION)
PKG_HASH:=70845dde2d70606aa207db5edfe878e266f9c193f1956dd10ba1b7e9a3c8d101

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

GPL_INCLUDE_SRC:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/sshfs
	TITLE:=SSHFS service for Teltonika package manager
	DEPENDS:=+libfuse +fuse-utils +glib2 +libpthread +vuci-app-sshfs
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Filesystem
	URL:=http://fuse.sourceforge.net/
	PKG_TLT_NAME:=SSHFS
	PKG_ROUTER:=$(TLT_PLATFORM_NAME)
endef

define Package/sshfs/description
	Simple application to mount remote system over SFTP.
endef

define Package/sshfs/conffiles
/etc/config/sshfs
endef

CONFIGURE_VARS += \
	SSHFS_CFLAGS=" \
		-D_FILE_OFFSET_BITS=64 \
		-I$(STAGING_DIR)/usr/include/glib-2.0 \
		-I$(STAGING_DIR)/usr/lib/glib-2.0/include \
		-I$(STAGING_DIR)/usr/include/fuse" \
	SSHFS_LIBS=" \
		-lglib-2.0 -liconv $(if $(INTL_FULL),-lintl) -lfuse -pthread -lgthread-2.0 \
		-L$(STAGING_DIR)/usr/lib"

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		DESTDIR="$(PKG_BUILD_DIR)" \
		all install
endef

define Package/sshfs/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/sbin $(1)/etc/config $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/usr/bin/sshfs $(1)/usr/bin/
	$(INSTALL_BIN) ./files/sshfs.config $(1)/etc/config/sshfs
	$(INSTALL_BIN) ./files/sshfs.init $(1)/etc/init.d/sshfs
	$(INSTALL_BIN) ./files/run_sshfs_application.sh $(1)/usr/sbin/run_sshfs_application.sh
endef

$(eval $(call BuildPackage,sshfs))
