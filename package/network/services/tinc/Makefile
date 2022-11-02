#
# Copyright (C) 2007-2015 OpenWrt.org
#

# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tinc
PKG_VERSION:=1.0.36
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.tinc-vpn.org/packages
PKG_HASH:=40f73bb3facc480effe0e771442a706ff0488edea7a5f2505d4ccb2aa8163108

PKG_CPE_ID:=cpe:/a:tinc:tinc

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/tinc
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+liblzo +libopenssl +kmod-tun +zlib
  TITLE:=VPN tunneling daemon
  URL:=http://www.tinc-vpn.org/
  MAINTAINER:=Saverio Proto <zioproto@gmail.com>
  SUBMENU:=VPN
  PKG_ROUTER:=$(TLT_PLATFORM_NAME)
  PKG_TLT_NAME:=Tinc VPN
endef


define Package/tinc/description
  tinc is a Virtual Private Network (VPN) daemon that uses tunnelling and
  encryption to create a secure private network between hosts on the Internet.
endef

TARGET_CFLAGS += -std=gnu99

CONFIGURE_ARGS += \
	--with-kernel="$(LINUX_DIR)" \
	--with-zlib="$(STAGING_DIR)/usr" \
	--with-lzo-include="$(STAGING_DIR)/usr/include/lzo"

CONFIGURE_VARS += \
	ac_cv_have_decl_OpenSSL_add_all_algorithms=yes

define Package/tinc/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/tincd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) files/$(PKG_NAME).config $(1)/etc/config/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc/tinc
	$(INSTALL_DIR) $(1)/lib/upgrade/keep.d
	$(INSTALL_DATA) files/tinc.upgrade $(1)/lib/upgrade/keep.d/tinc
endef

define Package/tinc/preinst
	#!/bin/sh

	add_zone() {
		sec=$$(uci add firewall zone)
		uci set firewall.$$sec.name="tinc"
		uci set firewall.$$sec.src="wan"
		uci set firewall.$$sec.device="tinc+"
		uci set firewall.$$sec.input="ACCEPT"
		uci set firewall.$$sec.output="ACCEPT"
		uci set firewall.$$sec.forward="REJECT"
	}

	add_rule() {
		sec=$$(uci add firewall rule)
		uci set firewall.$$sec.dest_port="655"
		uci set firewall.$$sec.src="wan"
		uci set firewall.$$sec.name="Allow-tinc-traffic"
		uci set firewall.$$sec.target="ACCEPT"
		uci set firewall.$$sec.proto="tcp udp"
	}

	add_zone
	add_rule
	uci commit firewall
	/etc/init.d/firewall reload
endef

define Package/tinc/prerm
	#!/bin/sh
	. /lib/functions.sh

	clean_zone() {
		config_get name "$$1" name

		[ "$$name" = "tinc" ] && {
			uci_remove firewall "$$1"
		}
	}

	clean_rule() {
		config_get name "$$1" name

		[ "$$name" = "Allow-tinc-traffic" ] && {
			uci_remove firewall "$$1"
		}
	}

	config_load firewall
	config_foreach clean_zone zone
	config_foreach clean_rule rule
	uci commit firewall
	/etc/init.d/firewall reload
endef

define Package/tinc/conffiles
/etc/config/tinc
endef


$(eval $(call BuildPackage,tinc))
