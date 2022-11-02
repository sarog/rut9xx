include $(TOPDIR)/rules.mk

PKG_NAME:=wireguard

# taken from wireguard-tools
PKG_VERSION:=1.0.20210223
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/wireguard/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=VPN
  URL:=https://www.wireguard.com
  MAINTAINER:=Jason A. Donenfeld <Jason@zx2c4.com> \
              Kevin Darbyshire-Bryant <ldir@darbyshire-bryant.me.uk>
endef

define Package/wireguard
  $(call Package/wireguard/Default)
  TITLE:=WireGuard meta-package
  DEPENDS:=+vuci-app-wireguard +wireguard-tools +kmod-wireguard
endef

define Build/Prepare
endef

define Build/Compile
endef

#Unload WireGuard kernel module
define Package/wireguard/prerm
	#!/bin/sh

	. /lib/functions.sh

	clean_config() {
		local section_id="$$1"
		local proto=""
		local cfg=""
		config_get proto "$$section_id" proto ""

		if [[ "$$proto" == "wireguard" ]]; then
			uci_remove network "$$section_id"
		else
			cfg=$$(uci_get network "$$section_id" | grep wireguard_)
			if [[ ! -z "$$cfg" ]]; then
				uci_remove network "$$section_id"
			fi
		fi
	}

	config_load 'network'
	config_foreach clean_config
	uci_commit network
endef

define Package/wireguard/install
	true
endef

$(eval $(call BuildPackage,wireguard))
