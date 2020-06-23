#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tlt_custom_pkg_zerotier
PKG_VERSION:=1.1.14

PKG_LICENSE:=GPL-3.0

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/zerotier/ZeroTierOne
PKG_SOURCE_SUBDIR:=ZeroTierOne-$(PKG_VERSION)
PKG_SOURCE_VERSION:=ae491c277e6f35d1acbdcbf700e2b834957295ae
PKG_SOURCE:=zerotier-$(PKG_VERSION).tar.xz
#PKG_MIRROR_MD5SUM:=c8c3219c995a59161832d580a194f6280de7a4eef75cebece6f38400b64f003e
PKG_MIRROR_MD5SUM:=5063603a7cfc0f30923d33e040d6361a
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)
PKG_ROUTER:=RUT9
PKG_FIRMWARE:=06.05.0
PKG_TLT_NAME:=ZeroTier
PKG_VERSION_PM:=1.1.16

PKG_BUILD_DEPENDS:=uclibcxx
PKG_BUILD_PARALLEL:=1
include $(INCLUDE_DIR)/package.mk

define Package/tlt_custom_pkg_zerotier
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libpthread +kmod-tun +ip +ZEROTIER_ENABLE_NETWORK_CONTROLLER:libsqlite3 +ZEROTIER_USE_MINIUPNPC:libminiupnpc +ZEROTIER_USE_MINIUPNPC:libnatpmp
  TITLE:=Create flat virtual Ethernet networks of almost unlimited size
  URL:=https://www.zerotier.com
  SUBMENU:=VPN
  MAINTAINER:=Moritz Warning <moritzwarning@web.de>
endef

define Package/tlt_custom_pkg_zerotier/description
	ZeroTier creates a global provider-independent virtual private cloud network.
endef

define Package/tlt_custom_pkg_zerotier/config
	source "$(SOURCE)/Config.in"
endef

ifeq ($(CONFIG_ZEROTIER_ENABLE_NETWORK_CONTROLLER),y)
MAKE_FLAGS += ZT_ENABLE_NETWORK_CONTROLLER=1
MAKE_FLAGS += LDLIBS+=" -lsqlite3 "
endif

ifeq ($(CONFIG_ZEROTIER_USE_MINIUPNPC),y)
MAKE_FLAGS += ZT_USE_MINIUPNPC=1
endif

ifeq ($(CONFIG_ZEROTIER_ENABLE_DEBUG),y)
TARGET_CXXFLAGS += -DZT_TRACE -ggdb3
endif

MAKE_FLAGS += \
	DEFS="" \
	LDFLAGS+=" -L$(STAGING_DIR)/usr/lib/uClibc++ -pthread " \
	LDLIBS+=" -fno-builtin -nodefaultlibs -Wl,-Bstatic -luClibc++ -Wl,-Bdynamic -lpthread -lm -lc -lsupc++ -lc -lgcc -lgcc_eh -lgcc_s " \
	CXXFLAGS+=" -fno-builtin -nostdinc++ -I$(STAGING_DIR)/usr/include/uClibc++ -DGCC_HASCLASSVISIBILITY -Wall -fPIE -fvisibility=hidden "

define Build/Compile
	$(call Build/Compile/Default,one)
ifeq ($(CONFIG_ZEROTIER_ENABLE_SELFTEST),y)
	$(call Build/Compile/Default,selftest)
endif
endef

define Package/tlt_custom_pkg_zerotier/postinst
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@zerotier[-1]
add ucitrack zerotier
set ucitrack.@zerotier[-1].init='zerotier'
add_list ucitrack.@zerotier[-1].affects='firewall'
commit ucitrack
endef

define Package/tlt_custom_pkg_zerotier/postrm
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@zerotier[-1]
commit ucitrack
endef

define Package/tlt_custom_pkg_zerotier/conffiles
/etc/config/zerotier
endef

define Package/tlt_custom_pkg_zerotier/install
	$(INSTALL_DIR) $(1)/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/zerotier-one $(1)/sbin/
	$(LN) zerotier-one $(1)/sbin/zerotier-cli
	$(LN) zerotier-one $(1)/sbin/zerotier-idtool

ifeq ($(CONFIG_ZEROTIER_ENABLE_SELFTEST),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/zerotier-selftest $(1)/sbin/
endif

	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) files/zerotier.init $(1)/etc/init.d/zerotier
	$(INSTALL_BIN) files/zerotier.sh $(1)/sbin/zerotier.sh
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) files/zerotier.config $(1)/etc/config/zerotier

	#for package manager
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-zerotier/luasrc/controller/zerotier.lua $(1)/usr/lib/lua/luci/controller/zerotier.lua
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-zerotier/luasrc/model/cbi/zerotier.lua $(1)/usr/lib/lua/luci/model/cbi/zerotier.lua
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-zerotier/luasrc/model/cbi/zerotiervpn.lua $(1)/usr/lib/lua/luci/model/cbi/zerotiervpn.lua
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-zerotier/luasrc/view/zeromap.htm $(1)/usr/lib/lua/luci/view/zeromap.htm

endef

$(eval $(call BuildPackage,tlt_custom_pkg_zerotier))

