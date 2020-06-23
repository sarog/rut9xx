#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libsmpp34
PKG_VERSION:=1.12.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/osmocom/libsmpp34/archive
PKG_MD5SUM:=6b989918aa2e5a7d81149669d0ef6585
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_FIXUP:=autoreconf

PKG_INSTALL:=1
PKG_LICENSE:=LGPLv2.1

include $(INCLUDE_DIR)/package.mk

define Package/libsmpp34
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=C library for SMPP 3.4
  DEPENDS:=+libpthread +librt
  URL:=http://git.osmocom.org/libsmpp34
#  MENU:=1
endef

define Package/libsmpp34/description
This package contains a C-language library implementing the SMPP Protocol version 3.4
as specified by the SMPP Developers Forum.
endef

#TARGET_CFLAGS += \
#	-ffunction-sections \
#	-fdata-sections

#CONFIGURE_VARS += \
#	ac_cv_linux_vers=$(LINUX_VERSION) \
#	ac_cv_header_libusb_1_0_libusb_h=no \
#	ac_cv_netfilter_can_compile=no


CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--with-build-cc="$(HOSTCC)"

#CONFIGURE_ARGS += \
#	--enable-shared \
#	--enable-static \
#	--disable-yydebug \
#	--with-build-cc="$(HOSTCC)" \
#	--with-pcap=linux \
#	--without-septel \
#	--without-dag \
#	--without-libnl \
#	--without-snf \
#	--disable-can \
#	--disable-canusb \
#	--disable-dbus \
#	--disable-bluetooth

#MAKE_FLAGS += \
#	CCOPT="$(TARGET_CFLAGS) -I$(BUILD_DIR)/linux/include"

#define Build/Configure
#	$(call Build/Configure/Default)
#	$(if $(CONFIG_PCAP_HAS_USB),,$(SED) '/^#define PCAP_SUPPORT_USB/D' $(PKG_BUILD_DIR)/config.h)
#	$(if $(CONFIG_PCAP_HAS_USB),,$(SED) 's/pcap-usb-linux.c *//' $(PKG_BUILD_DIR)/Makefile)
#	$(if $(CONFIG_PCAP_HAS_BT),,$(SED) '/^#define PCAP_SUPPORT_BT/D' $(PKG_BUILD_DIR)/config.h)
#	$(if $(CONFIG_PCAP_HAS_BT),,$(SED) 's/pcap-bt-linux.c *//' $(PKG_BUILD_DIR)/Makefile)
#endef

#define Build/InstallDev
#	$(INSTALL_DIR) $(1)/usr/include
#	$(CP) $(PKG_INSTALL_DIR)/usr/include/pcap* $(1)/usr/include/
#
#	$(INSTALL_DIR) $(1)/usr/lib
#	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsmpp34.{a,so*} $(1)/usr/lib/
#endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsmpp34.so* $(1)/usr/lib/
endef

define Package/libsmpp34/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsmpp34.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libsmpp34))
