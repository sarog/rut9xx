# 
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
curdir:=target

$(curdir)/subtargets:=install
$(curdir)/builddirs:=linux sdk imagebuilder toolchain
$(curdir)/builddirs-default:=linux
$(curdir)/builddirs-install:=linux $(if $(CONFIG_SDK),sdk) $(if $(CONFIG_IB),imagebuilder) $(if $(CONFIG_MAKE_TOOLCHAIN),toolchain)

$(curdir)/sdk/install:=$(curdir)/linux/install
$(curdir)/imagebuilder/install:=$(curdir)/linux/install

$(eval $(call stampfile,$(curdir),target,prereq,.config))
$(eval $(call stampfile,$(curdir),target,compile,$(TMP_DIR)/.build))
$(eval $(call stampfile,$(curdir),target,install,$(TMP_DIR)/.build))

$($(curdir)/stamp-install): $($(curdir)/stamp-compile) 

GPL_TARGET_DIR:=$(GPL_BUILD_DIR)/target

$(curdir)/gpl:
	$(RM) -rf "$(GPL_TARGET_DIR)"
	mkdir -p "$(GPL_TARGET_DIR)"
	find "target" -maxdepth 1 -type f -exec cp "{}" "$(GPL_TARGET_DIR)" \;
	find "target" -maxdepth 1 -type d ! -name "target" ! -name "gpl-prep" ! -name "linux" \
		-exec cp -r "{}" "$(GPL_TARGET_DIR)" \;
	mkdir -p "$(GPL_TARGET_DIR)/linux"; \
	$(MAKE) -C "target/linux" gpl 1="$(GPL_TARGET_DIR)/linux"; \

$(eval $(call subdir,$(curdir)))
