include $(TOPDIR)/rules.mk

PKG_NAME:=libcgi
PKG_VERSION:=1.3.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/rafaelsteil/libcgi.git
PKG_SOURCE_VERSION:=1d2928626fd42abab07440df24d62f101455e50c
PKG_MIRROR_HASH:=0781de5b640c35d84aa22006bd39687ab72455854f3bbb911221cba5dd25a481
CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libcgi
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=C library to build CGI applications
endef

define Package/libcgi/description
  LibCGI is a simple to use yet powerful library written from scratch to \
  assist in the making of CGI appliactions in C
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/libcgi
	$(CP) $(PKG_BUILD_DIR)/include/libcgi/* $(1)/usr/include/libcgi
	$(CP) $(PKG_BUILD_DIR)/src/libcgi.so* $(1)/usr/lib/
endef

define Package/libcgi/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/libcgi.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libcgi))