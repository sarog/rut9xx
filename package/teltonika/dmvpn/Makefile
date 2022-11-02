#
# Copyright (C) 2021 Teltonika
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=DMVPN
PKG_VERSION:=2021-02-11
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/dmvpn
	SUBMENU:=VPN
  	SECTION:=net
  	CATEGORY:=Network
	TITLE:=Dynamic multipoint VPN using NHRP GRE and IPsec
	DEPENDS:=+kmod-gre +IPV6:kmod-gre6 +resolveip +libpthread +ip \
		+kmod-crypto-authenc +kmod-ipsec +kmod-ipsec4 +IPV6:kmod-ipsec6 \
		+kmod-ipt-ipsec +iptables-mod-ipsec +frr +frr-libfrr +libcares \
		+frr-vtysh +frr-watchfrr +frr-zebra +vuci-app-dmvpn +frr-nhrpd
	ifeq (m, $(CONFIG_PACKAGE_dmvpn))
		PKG_TLT_NAME:=DMVPN
		PKG_ROUTER:=$(TLT_PLATFORM_NAME)
	endif
endef



define Package/dmvpn/conffiles
/etc/config/dmvpn
endef

define Package/dmvpn/install
	$(INSTALL_DIR) \
		$(1)/etc/config
	$(INSTALL_CONF) files/etc/config/dmvpn \
		$(1)/etc/config/dmvpn
endef

define Package/dmvpn/prerm
	#!/bin/sh
	. /lib/functions.sh
	clean_config() {
		uci_remove network "$${1}"
		uci_remove network "$${1}_static"
		uci_remove ipsec "$${1}_dmvpn_c"
		uci_remove ipsec "$${1}_dmvpn"
		uci_remove ipsec "$${1}_dmvpn_ph1"
		uci_remove ipsec "$${1}_dmvpn_ph2"
		uci_remove frr "$${1}_dmvpn"
	}
	config_load 'dmvpn'
	config_foreach clean_config dmvpn
	uci_commit network
	uci_commit ipsec
	uci_commit frr
endef

$(eval $(call BuildPackage,dmvpn))
