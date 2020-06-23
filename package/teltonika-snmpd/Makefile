#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=net-snmp
PKG_VERSION:=5.8
#PKG_RELEASE:=1

PKG_MAINTAINER:=Stijn Tintel <stijn@linux-ipv6.be>
PKG_LICENSE:=MIT BSD-3-Clause-Clear
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_ROUTER:=RUT9
PKG_FIRMWARE:=06.05.0
PKG_TLT_NAME:=SNMP
PKG_VERSION_PM:=5.8.2

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/net-snmp/Default
  SECTION:=net
  CATEGORY:=Network
  URL:=http://www.net-snmp.org/
endef

define Package/net-snmp/Default/description
 Simple Network Management Protocol (SNMP) is a widely used protocol for
 monitoring the health and welfare of network equipment (eg. routers),
 computer equipment and even devices like UPSs. Net-SNMP is a suite of
 applications used to implement SNMP v1, SNMP v2c and SNMP v3 using both
 IPv4 and IPv6.
endef


define Package/libnetsnmp
$(call Package/net-snmp/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libnl-tiny +libopenssl
  TITLE:=Open source SNMP implementation (libraries)
endef

define Package/libnetsnmp/description
$(call Package/net-snmp/Default/description)
 .
 This package contains shared libraries, needed by other programs.
endef


define Package/snmp-mibs
$(call Package/net-snmp/Default)
  TITLE:=Open source SNMP implementation (MIB-files)
endef

define Package/snmp-mibs/description
$(call Package/net-snmp/Default/description)
 .
 This package contains SNMP MIB-Files.
endef


define Package/snmp-utils
$(call Package/net-snmp/Default)
  DEPENDS:=+libnetsnmp
  TITLE:=Open source SNMP implementation (utilities)
endef

define Package/snmp-utils/description
$(call Package/net-snmp/Default/description)
 .
 This package contains SNMP client utilities:
   - snmpget
   - snmpset
   - snmpstatus
   - snmptest
   - snmptrap
   - snmpwalk
endef


define Package/tlt_custom_pkg_snmpd
$(call Package/net-snmp/Default)
  DEPENDS:=+libnetsnmp
  TITLE:=Open source SNMP implementation (daemon)
endef

define Package/tlt_custom_pkg_snmpd/description
$(call Package/net-snmp/Default/description)
 .
 This package contains the SNMP agent, dynamically linked.
endef


define Package/snmpd-static
$(call Package/net-snmp/Default)
  DEPENDS:=+snmpd
  TITLE:=Open source SNMP implementation (daemon)
endef


define Package/snmptrapd
$(call Package/net-snmp/Default)
  DEPENDS:=+libnetsnmp
  TITLE:=Open source SNMP implementation (notification receiver)
endef

define Package/snmptrapd/description
$(call Package/net-snmp/Default/description)
 .
 This package contains the SNMP notification receiver.
endef


SNMP_MIB_MODULES_INCLUDED = \
	agent/extend \
	agentx \
	host/hr_device \
	host/hr_disk \
	host/hr_filesys \
	host/hr_network \
	host/hr_partition \
	host/hr_proc \
	host/hr_storage \
	host/hr_system \
	if-mib/ifXTable \
	ip-mib/inetNetToMediaTable \
	ieee802dot11 \
	mibII/at \
	mibII/icmp \
	mibII/ifTable \
	mibII/ip \
	mibII/snmp_mib \
	mibII/sysORTable \
	mibII/system_mib \
	mibII/tcp \
	mibII/udp \
	mibII/vacm_context \
	mibII/vacm_vars \
	snmpv3/snmpEngine \
	snmpv3/snmpMPDStats \
	snmpv3/usmConf \
	snmpv3/usmStats \
	snmpv3/usmUser \
	ucd-snmp/disk \
	ucd-snmp/dlmod \
	ucd-snmp/extensible \
	ucd-snmp/loadave \
	ucd-snmp/memory \
	ucd-snmp/pass \
	ucd-snmp/pass_persist \
	ucd-snmp/proc \
	ucd-snmp/vmstat \
	util_funcs \
	utilities/execute \
	teltonika \
	examples/notification\

SNMP_MIB_MODULES_EXCLUDED = \
	agent_mibs \
	disman/event \
	disman/schedule \
	hardware \
	host \
	if-mib \
	ip-mib \
	mibII \
	notification \
	notification-log-mib \
	snmpv3mibs \
	target \
	tcp-mib \
	ucd_snmp \
	udp-mib \
	utilities \

SNMP_TRANSPORTS_INCLUDED = Callback UDP Unix

SNMP_TRANSPORTS_EXCLUDED = TCP TCPIPv6

TARGET_CFLAGS += $(FPIC)
TARGET_CPPFLAGS += -I$(STAGING_DIR)/usr/include/libnl-tiny

CONFIGURE_ARGS += \
	--enable-mfd-rewrites \
	--enable-shared \
	--enable-static \
	--with-endianness=$(if $(CONFIG_BIG_ENDIAN),big,little) \
	--with-logfile=/var/log/snmpd.log \
	--with-persistent-directory=/usr/lib/snmp/ \
	--with-default-snmp-version=1 \
	--with-sys-contact=root@localhost \
	--with-sys-location=Unknown \
	--enable-applications \
	--disable-manuals \
	--disable-scripts \
	--with-out-mib-modules="$(SNMP_MIB_MODULES_EXCLUDED)" \
	--with-mib-modules="$(SNMP_MIB_MODULES_INCLUDED)" \
	--with-out-transports="$(SNMP_TRANSPORTS_EXCLUDED)" \
	--with-transports="$(SNMP_TRANSPORTS_INCLUDED)" \
	--without-libwrap \
	--without-mysql \
	--without-rpm \
	--without-zlib \
	--with-nl \
	 $(call autoconf_bool,CONFIG_IPV6,ipv6) \
	--disable-perl-cc-checks \
	--disable-embedded-perl \
	--without-perl-modules \

CONFIGURE_VARS += \
	ac_cv_header_netlink_netlink_h=yes \
	ac_cv_header_pcre_h=no \
	netsnmp_cv_func_nl_connect_LIBS=-lnl-tiny \

ifeq ($(CONFIG_IPV6),y)
SNMP_TRANSPORTS_INCLUDED+= UDPIPv6
endif

TARGET_LDFLAGS += -L$(TOOLCHAIN_DIR)/usr/lib

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(call Build/Prepare/Default)
endef

define Build/Configure
	$(call Build/Configure/Default)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		LDFLAGS="$(TARGET_LDFLAGS) -lm -lc" \
		all install
	# we will use statically linked daemon
	$(MAKE) -C $(PKG_BUILD_DIR)/agent \
		LDFLAGS="$(TARGET_LDFLAGS) -lm -static" \
		EXEEXT="-static" \
		snmpd-static
endef

define Build/InstallDev
	$(INSTALL_DIR) $(2)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/net-snmp-config $(2)/bin/
	$(SED) 's,=/usr,=$(STAGING_DIR)/usr,g' $(2)/bin/net-snmp-config
	$(INSTALL_DIR) $(STAGING_DIR)/usr/bin
	$(LN) $(STAGING_DIR)/host/bin/net-snmp-config $(STAGING_DIR)/usr/bin/

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/net-snmp $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnetsnmp{,agent,helpers,mibs}.{a,so*} $(1)/usr/lib/
endef

define Package/libnetsnmp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnetsnmp{,agent,helpers,mibs}.so.* $(1)/usr/lib/
endef

define Package/snmp-mibs/install
	$(INSTALL_DIR) $(1)/usr/share/snmp/mibs
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/snmp/mibs/* $(1)/usr/share/snmp/mibs/
endef

define Package/snmp-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/snmp{get,set,status,test,trap,walk} $(1)/usr/bin/
endef

define Package/tlt_custom_pkg_snmpd/conffiles
/etc/default/snmpd
/etc/config/snmpd
endef

define Package/tlt_custom_pkg_snmpd/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/snmpd.conf $(1)/etc/config/snmpd
	$(INSTALL_DIR) $(1)/etc/snmp
	$(LN) /var/run/snmpd.conf $(1)/etc/snmp/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/snmpd.init $(1)/etc/init.d/snmpd
	$(INSTALL_DIR) $(1)/usr/opt/snmp/mibs
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/mibs/TLT-MIB.txt $(1)/usr/opt/snmp/mibs/TLT-MIB.txt
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/snmpd $(1)/usr/sbin/snmpd
	$(INSTALL_BIN) ./files/hotspot_user_info.lua $(1)/usr/sbin/hotspot_user_info.lua
	$(INSTALL_BIN) ./files/analog_calc.lua $(1)/usr/sbin/analog_calc
	$(INSTALL_BIN) ./files/sysget $(1)/usr/sbin/sysget

	#for package manager
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/tlt-snmp
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/tlt-snmp
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-tlt-snmp/luasrc/controller/tlt-snmp.lua $(1)/usr/lib/lua/luci/controller/tlt-snmp.lua
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-tlt-snmp/luasrc/model/cbi/tlt-snmp/tlt-snmp.lua $(1)/usr/lib/lua/luci/model/cbi/tlt-snmp/tlt-snmp.lua
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-tlt-snmp/luasrc/model/cbi/tlt-snmp/tlt-trap.lua $(1)/usr/lib/lua/luci/model/cbi/tlt-snmp/tlt-trap.lua
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-tlt-snmp/luasrc/model/cbi/tlt-snmp/tlt-trap-details.lua $(1)/usr/lib/lua/luci/model/cbi/tlt-snmp/tlt-trap-details.lua
	$(INSTALL_BIN) $(TOPDIR)/custom_feeds/teltonika_luci/applications/luci-tlt-snmp/luasrc/view/tlt-snmp/add_trap_rule.htm $(1)/usr/lib/lua/luci/view/tlt-snmp/add_trap_rule.htm

	#libnetsnmp
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnetsnmp{,agent,helpers,mibs}.so.* $(1)/usr/lib/
endef

define Package/snmptrapd/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/snmptrapd.init $(1)/etc/init.d/snmptrapd
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnetsnmptrapd.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/snmptrapd $(1)/usr/sbin/
endef

define Package/tlt_custom_pkg_snmpd/postinst
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@snmpd[-1]
add ucitrack snmpd
set ucitrack.@snmpd[-1].init='snmpd'
commit ucitrack
endef

define Package/tlt_custom_pkg_snmpd/postrm
#!/bin/sh
uci -q batch <<-EOF >/dev/null
delete ucitrack.@snmpd[-1]
commit ucitrack
endef

$(eval $(call BuildPackage,libnetsnmp))
$(eval $(call BuildPackage,snmp-mibs))
$(eval $(call BuildPackage,snmp-utils))
$(eval $(call BuildPackage,tlt_custom_pkg_snmpd))
$(eval $(call BuildPackage,snmpd-static))
$(eval $(call BuildPackage,snmptrapd))
