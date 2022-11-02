#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bluez
PKG_VERSION:=5.56
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/bluetooth/
PKG_HASH:=59c4dba9fc8aae2a6a5f8f12f19bc1b0c2dc27355c7ca3123eed3fe6bd7d0b9d

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=
PKG_CPE_ID:=cpe:/a:bluez:bluez

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/bluez/Default
  TITLE:=Bluetooth
  URL:=http://www.bluez.org/
endef

define Package/bluez-libs
$(call Package/bluez/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= library
  DEPENDS:=+libpthread +USB_SUPPORT:kmod-bluetooth +glib2 +dbus
endef

TARGET_CFLAGS += -D_GNU_SOURCE -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections

CONFIGURE_ARGS += \
	--enable-static \
	--enable-shared \
	--enable-library \
	--disable-client \
	--disable-datafiles \
	--disable-experimental \
	--disable-monitor \
	--disable-obex \
	--disable-threads \
	--disable-tools \
	--disable-android \
	--disable-cups \
	--disable-manpages \
	--disable-sixaxis \
	--disable-hid \
	--disable-network \
	--disable-hog \
	--disable-systemd \
	--disable-test \
	--disable-udev \
	--disable-deprecated

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/bluetooth $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libbluetooth.so* $(1)/usr/lib/
endef


define Package/bluez-libs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libbluetooth.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,bluez-libs))
