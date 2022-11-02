include $(TOPDIR)/rules.mk

PKG_NAME:=troubleshoot
PKG_VERSION:=1.0

include $(INCLUDE_DIR)/package.mk

define Package/troubleshoot
	SECTION:=base
	CATEGORY:=Base system
	DEPENDS:=
	TITLE:=Troubleshoot generator by Teltonika
endef

define Package/troubleshoot/description
	Troubleshoot generator by Teltonika
endef



define Package/troubleshoot/install
	$(INSTALL_DIR) $(1)/sbin $(1)/lib/functions $(1)/lib/troubleshoot
	$(INSTALL_BIN) ./files/troubleshoot.sh $(1)/sbin/
	$(CP) ./files/libtroubleshoot.sh $(1)/lib/functions/
	$(CP) ./files/troubleshoot/* $(1)/lib/troubleshoot/
endef

$(eval $(call BuildPackage,troubleshoot))
