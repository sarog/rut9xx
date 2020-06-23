#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=load_balancing
PKG_VERSION:=1.0

include $(INCLUDE_DIR)/package.mk

define Package/load_balancing
   SECTION:=net
   CATEGORY:=Network
   SUBMENU:=Routing and Redirection
   DEPENDS:=+ip +iptables +iptables-mod-conntrack-extra +iptables-mod-ipopt
   TITLE:=Multiwan hotplug script with connection tracking support
   PKGARCH:=all
endef

define Package/load_balancing/description
Hotplug script which makes configuration of multiple WAN interfaces simple
and manageable. With loadbalancing/failover support for up to 250 wan
interfaces, connection tracking and an easy to manage traffic ruleset.
endef

define Package/load_balancing/conffiles
   /etc/config/load_balancing
endef

define Build/Compile
endef

define Package/load_balancing/install
   $(CP) ./files/* $(1)
endef

define Package/load_balancing/postinst
   [ -n "$${IPKG_INSTROOT}" ] || /etc/init.d/load_balancing enable
   exit 0
endef

$(eval $(call BuildPackage,load_balancing))
