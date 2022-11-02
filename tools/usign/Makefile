include $(TOPDIR)/rules.mk

PKG_NAME:=usign
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/usign.git
PKG_SOURCE_DATE:=2020-05-23
PKG_SOURCE_VERSION:=f1f65026a94137c91b5466b149ef3ea3f20091e9
PKG_MIRROR_HASH:=3f6569a5e63fdfd032976ac0f79d736d3935101ac1b97fb370514b013c5e6bb6

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk
include $(INCLUDE_DIR)/gpl.mk

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/usign $(STAGING_DIR_HOST)/bin/
endef

$(eval $(call HostBuild))
