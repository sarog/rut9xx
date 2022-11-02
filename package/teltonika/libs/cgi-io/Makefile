include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/download.mk

PKG_NAME:=cgi-io
PKG_VERSION:=2022-08-08
PKG_RELEASE:=1

PKG_SOURCE_VERSION:=269259bef03feca441c3f683f3957b6a6448337e
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/cgi-io
	SECTION:=vuci
	CATEGORY:=VuCI
	SUBMENU:=vuci-rpc-core
	DEPENDS:=+libubox +libubus +libuci
	TITLE:=CGI utility for handling up/downloading of files
endef

define Package/cgi-io/description
	This package contains an cgi utility that is useful for up/downloading files
endef


define Package/cgi-io/install
	$(INSTALL_DIR) $(1)/usr/libexec $(1)/www/cgi-bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cgi-io $(1)/usr/libexec
	$(LN) ../../usr/libexec/cgi-io $(1)/www/cgi-bin/cgi-upload 
	$(LN) ../../usr/libexec/cgi-io $(1)/www/cgi-bin/cgi-backup 
	$(LN) ../../usr/libexec/cgi-io $(1)/www/cgi-bin/cgi-tcpdump 
	$(LN) ../../usr/libexec/cgi-io $(1)/www/cgi-bin/cgi-troubleshoot 
	$(LN) ../../usr/libexec/cgi-io $(1)/www/cgi-bin/cgi-cert
	$(LN) ../../usr/libexec/cgi-io $(1)/www/cgi-bin/cgi-mib
endef

$(eval $(call BuildPackage,cgi-io))
