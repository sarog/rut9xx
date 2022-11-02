#
# Copyright (C) 2021 Teltonika
#

PKG_VUCI_BUILD_DIR := $(if $(findstring feeds/vuci,$(shell pwd)),$(GPL_BUILD_DIR)/package/feeds/vuci/$(shell basename $$(pwd)),)
PKG_NORMAL_BUILD_DIR := $(GPL_BUILD_DIR)/$(subst $(TOPDIR)/,,$(shell pwd))
PKG_GPL_BUILD_DIR ?= $(if $(PKG_VUCI_BUILD_DIR),$(PKG_VUCI_BUILD_DIR),$(PKG_NORMAL_BUILD_DIR))

define gpl_clear_install
	[ -e "$(1)/Makefile" ] && \
		sed -i '/^define Build\/InstallGPL/,/^endef/d' "$(1)/Makefile" || true
endef

define gpl_clear_vars
	( \
		sed -i "/cmake.mk/d" "$(1)/Makefile"; \
		sed -i '/define Build\/Prepare/,/endef/d' "$(1)/Makefile"; \
		sed -i '/define Build\/Compile/,/endef/d' "$(1)/Makefile"; \
		sed -i "/^PKG_MIRROR_HASH:=/d" "$(1)/Makefile"; \
		sed -i "/^PKG_SOURCE_PROTO:=/d" "$(1)/Makefile"; \
		sed -i "/^PKG_SOURCE_URL:=/d" "$(1)/Makefile"; \
		sed -i "/^PKG_MIRROR_HASH:=/d" "$(1)/Makefile"; \
	)
endef

define gpl_install_mixed
	(\
		mkdir -p "$(1)"; \
		case "$(PKG_SOURCE_URL)" in \
		"$(TLT_GIT)"*) \
			cp -rf . "$(1)/"; \
			mkdir -p "$(1)/src"; \
			tar xf "$(TOPDIR)/dl/$(PKG_SOURCE)" --strip-components=1 -C "$(1)/src"; \
			sed -i "/^PKG_SOURCE_PROTO:=/d" "$(1)/Makefile"; \
			sed -i "/^PKG_SOURCE_URL:=/d" "$(1)/Makefile"; \
			sed -i "/^PKG_MIRROR_HASH:=/d" "$(1)/Makefile"; \
			;; \
		*) cp -rf . "$(1)/" ;; \
		esac; \
	)
endef

define gpl_install_feeds
	(\
		name=$$(basename "$(1)"); \
		rm -rf "$(1)"; \
		TOPDIR="$(GPL_BUILD_DIR)" "$(GPL_BUILD_DIR)/scripts/feeds" install "$${name}"; \
	)
endef

define gpl_install_deps
	(\
		dir=$$(find "$(TOPDIR)/package" -iname "$(1)" -print -quit | \
			sed 's@$(TOPDIR)/@@g'); \
		[ -n "$${dir}" ] && $(MAKE) -C "$(TOPDIR)/$${dir}" gpl-install || true; \
	)
endef

define gpl_scan_deps
	$(foreach dep,$(PKG_BUILD_DEPENDS), \
		$(if $(findstring :,$(dep)), \
			$(if $(findstring $(PKG_NAME),$(dep)),, \
				$(call gpl_install_deps,$(lastword $(subst :, ,$(dep)))); \
			) \
		, \
			$(if $(findstring /,$(dep)), \
				$(if $(findstring $(PKG_NAME),$(dep)),, \
					$(call gpl_install_deps,$(firstword $(subst /, ,$(dep)))); \
				) \
			, \
				$(call gpl_install_deps,$(dep)); \
			) \
		) \
	)

	$(foreach dep,$(HOST_BUILD_DEPENDS), \
		$(if $(findstring :,$(dep)), \
			$(if $(findstring $(PKG_NAME),$(dep)),, \
				$(call gpl_install_deps,$(lastword $(subst :, ,$(dep)))); \
			) \
		, \
			$(if $(findstring /,$(dep)), \
				$(if $(findstring $(PKG_NAME),$(dep)),, \
					$(call gpl_install_deps,$(firstword $(subst /, ,$(dep)))); \
				) \
			, \
				$(call gpl_install_deps,$(dep)); \
			) \
		) \
	)
endef

define gpl_install_def
	$(if $(findstring $(GPL_INCLUDE_SRC),1), \
		$(call gpl_install_mixed,$(1)) \
	, \
		$(error Build/InstallGPL section is not defined! Please fix the package) \
	)
endef

define gpl_install_closed
	rm -rf "$(PKG_GPL_BUILD_DIR)"; \
	mkdir -p "$(PKG_GPL_BUILD_DIR)/bin"; \
	cp Makefile "$(PKG_GPL_BUILD_DIR)"; \
	[ -e Config.in ] && cp Config.in "$(PKG_GPL_BUILD_DIR)" || true; \
	$(call gpl_clear_vars,$(PKG_GPL_BUILD_DIR)); \
	$(call Build/InstallGPL,$(PKG_GPL_BUILD_DIR)/bin)
endef

define Build/InstallGPL/Default
	$(eval current_dir:=$(shell pwd))
	$(if $(findstring package/teltonika,$(current_dir)), \
		$(call gpl_install_def,$(1)) \
	, \
		$(if $(findstring feeds/vuci,$(current_dir)), \
			$(if $(CONFIG_GPL_INCLUDE_WEB_SOURCES), \
				$(call gpl_install_mixed,$(1)) \
			, \
				$(call gpl_install_def,$(1)) \
			) \
		, \
			$(if $(findstring feeds,$(current_dir)), \
				$(call gpl_install_feeds,$(1)) \
			, \
				$(call gpl_install_mixed,$(1)) \
			) \
		) \
	)
endef

Build/InstallGPL=$(call Build/InstallGPL/Default,$(1))

$(GPL_BUILD_DIR)/package:
	rm -rf "$@"
	mkdir -p "$@"
	find "$(TOPDIR)/package" -maxdepth 1 -type f -exec cp "{}" "$@" \;
	cp -rf "$(TOPDIR)/feeds" "$(GPL_BUILD_DIR)"

gpl-install: $(GPL_BUILD_DIR)/package
	$(if $(findstring $(GPL_INCLUDE_SRC),1), \
		$(call Build/InstallGPL,$(PKG_GPL_BUILD_DIR)) \
	, \
		$(if $(findstring package/teltonika,$(shell pwd)), \
			$(call gpl_install_closed,$(PKG_GPL_BUILD_DIR)) \
		, \
			$(if $(findstring feeds/vuci,$(shell pwd)), \
				$(if $(CONFIG_GPL_INCLUDE_WEB_SOURCES), \
					$(call Build/InstallGPL,$(PKG_GPL_BUILD_DIR)) \
				, \
					$(call gpl_install_closed,$(PKG_GPL_BUILD_DIR)) \
				) \
			, \
				$(call Build/InstallGPL,$(PKG_GPL_BUILD_DIR)) \
			) \
		) \
	)
	$(call gpl_scan_deps)
	$(call gpl_clear_install,$(PKG_GPL_BUILD_DIR))