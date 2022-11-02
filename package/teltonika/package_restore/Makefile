#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=package_restore
PKG_VERSION:=2022-05-16
PKG_RELEASE:=2

PKG_SOURCE_VERSION:=1b01c1ed1681211082ad96193c7bff407681f3d9
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/package_restore
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=Auto package restore
endef



define Package/package_restore/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/package_restore.sh $(1)/sbin/package_restore.sh

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/package_restore.init $(1)/etc/init.d/package_restore

	$(INSTALL_DIR) $(1)/etc/uci-defaults
	if [ $(TLT_PLATFORM_RUTX) ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rutx_restore_list.sh $(1)/etc/uci-defaults/w-package-manager-restore; \
	elif [ $(TLT_PLATFORM_TRB2) ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/trb2_restore_list.sh $(1)/etc/uci-defaults/w-package-manager-restore; \
	elif [ $(TLT_PLATFORM_TRB1) ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/trb1_restore_list.sh $(1)/etc/uci-defaults/w-package-manager-restore; \
	elif [ $(TLT_PLATFORM_RUT9) ] || [ $(TLT_PLATFORM_RUT9M) ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rut9_restore_list.sh $(1)/etc/uci-defaults/w-package-manager-restore; \
	elif [ $(TLT_PLATFORM_RUT36X) ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rut36_restore_list.sh $(1)/etc/uci-defaults/w-package-manager-restore; \
	elif [ $(TLT_PLATFORM_RUT30X) ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rut30_restore_list.sh $(1)/etc/uci-defaults/w-package-manager-restore; \
	elif [ $(TLT_PLATFORM_RUT2) ]; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/rut2_restore_list.sh $(1)/etc/uci-defaults/w-package-manager-restore; \
	fi;

endef

$(eval $(call BuildPackage,package_restore))
