include $(TOPDIR)/rules.mk

PKG_NAME:=uqmi
PKG_RELEASE:=3

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/uqmi.git
PKG_SOURCE_DATE:=2020-11-22
PKG_SOURCE_VERSION:=0a19b5b77140465c29e2afa7d611fe93abc9672f
PKG_MIRROR_HASH:=0a6641f8e167efd21d464b0b2aeb1fec5f974dddcdb8822fbd5d7190d0b741b4
PKG_MAINTAINER:=Matti Laakso <malaakso@elisanet.fi>

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

# PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/uqmi
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=WWAN
  DEPENDS:=+libubox +libblobmsg-json +kmod-usb-net +kmod-usb-net-qmi-wwan +wwan
  TITLE:=Control utility for mobile broadband modems
endef

define Package/uqmi/description
  uqmi is a command line tool for controlling mobile broadband modems using
  the QMI-protocol.
endef

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include -ffunction-sections -fdata-sections

TARGET_LDFLAGS += -Wl,--gc-sections

CMAKE_OPTIONS += \
	-DDEBUG=1

define Package/uqmi/install
	$(INSTALL_DIR) $(1)/sbin $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uqmi $(1)/sbin/

	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/qmi.sh $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/qmux.sh $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/qmux_track.sh $(1)/usr/bin/qmuxtrack
	$(INSTALL_BIN) ./files/connm.sh $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/dhcp_mobile.script $(1)/lib/netifd/
	$(INSTALL_BIN) ./files/ncm.sh $(1)/lib/netifd/proto

	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/bridge_fix.sh $(1)/etc/uci-defaults/bridge_fix
endef

$(eval $(call BuildPackage,uqmi))
