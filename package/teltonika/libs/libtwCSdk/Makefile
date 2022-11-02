#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=libtwCSdk
PKG_VERSION:=2020-12-22
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=e64bb5f42965d5666522dd7b76c76709131730ce
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

GPL_INCLUDE_SRC:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

# set up openssl include and lib dir for CMake
OPENSSL_DIR:=$(wildcard $(BUILD_DIR)/openssl*)
CMAKE_OPTIONS += -DTHINGWORX_CSDK_OPENSSL_INCLUDE_DIR=$(OPENSSL_DIR)/include
CMAKE_OPTIONS += -DTHINGWORX_CSDK_OPENSSL_LIB_DIR=$(OPENSSL_DIR)

# for some reason we need to manually set SDK version defines here
#  because CMake builder doesn't set them
#  and final library becomes `libtwCSdk.so...` instead of `libtwCSdk.so.1.0.0`
CMAKE_OPTIONS += -DTWCSDK_VERSION_MAJOR=1
CMAKE_OPTIONS += -DTWCSDK_VERSION_MINOR=0
CMAKE_OPTIONS += -DTWCSDK_VERSION_PATCH=0

#UNPACK_CMD=unzip -d $(PKG_BUILD_DIR)/ $(DL_DIR)/$(PKG_SOURCE)

define Package/libtwCSdk
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=ThingWorx C SDK Library.
	DEPENDS:=+libopenssl +libpthread
endef

define Package/libtwCSdk/description
	A simple library to enable ThingWorx on the device.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include/libtwCSdk
	$(CP) $(PKG_BUILD_DIR)/src/* $(1)/usr/include/libtwCSdk
	$(CP) $(PKG_BUILD_DIR)/src/libtwCSdk.so* $(1)/usr/lib
endef

define Package/libtwCSdk/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/libtwCSdk.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,libtwCSdk))
