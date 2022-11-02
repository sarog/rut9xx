#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=modems
PKG_VERSION:=2022-08-04
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=31245948d2fa2a77027dbab1093b4d2388416c59
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/modems
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=3G/4G modem list
endef



define Package/modems/install
	$(INSTALL_DIR) $(1)/lib/network/wwan/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/data/* $(1)/lib/network/wwan/
	#in order to keep the Lede GIT repo free of filenames with colons,
	#we name the files xxxx-yyyy
	# and rename here after copying to the build directory
	shopt -s nullglob ; \
	for filevar in $(1)/lib/network/wwan/*-* ; \
	do \
		FILENAME=$$$$(basename $$$$filevar) ; \
		NEWNAME=$$$${FILENAME//-/:} ; \
		mv "$(1)/lib/network/wwan/$$$$FILENAME" "$(1)/lib/network/wwan/$$$$NEWNAME" ; \
	done
endef

$(eval $(call BuildPackage,modems))
