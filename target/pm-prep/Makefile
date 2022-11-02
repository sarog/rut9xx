#
# Copyright (C) 2021 Teltonika
#

include rules.mk

define PM/Build/Prepare
    mkdir -p $(OUTPUT_DIR)/packages/$(ARCH_PACKAGES)/pm_packages
    rm -f $(OUTPUT_DIR)/packages/$(ARCH_PACKAGES)/pm_packages/*
    mkdir -p $(OUTPUT_DIR)/packages/$(ARCH_PACKAGES)/zipped_packages
    rm -f $(OUTPUT_DIR)/packages/$(ARCH_PACKAGES)/zipped_packages/*

    packages="$(foreach m, $(shell jq "keys | .[]" --raw-output "$(TOPDIR)/ipk_packages.json"), \
        $(if $(filter m, $(CONFIG_PACKAGE_$(m))), $(m),) \
    )"; \
    $(TOPDIR)/scripts/prepare_tlt_pm_packages.sh "$${packages}" "$(TOPDIR)" "$(ARCH_PACKAGES)" "$(PACKAGE_DIR)"

    cd $(OUTPUT_DIR)/packages/$(ARCH_PACKAGES)/pm_packages && \
    $(SCRIPT_DIR)/ipkg-make-index.sh . 2>&1 > Packages.manifest && \
    grep -vE '^(Maintainer|LicenseFiles|Source|SourceName|Require)' Packages.manifest > Packages && \
    gzip -9nc Packages > Packages.gz && \
    $(STAGING_DIR_HOST)/bin/usign -S -m Packages -s $(BUILD_KEY)
endef
