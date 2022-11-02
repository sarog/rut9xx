define Package/base-files/install-target
	if [ $(CONFIG_RUT9_MIGRATION) ]; then \
		mkdir -p $(1)/etc/uci-defaults/001_rut9_migration; \
		mkdir -p $(1)/etc/migrate.conf; \
		$(CP) $(PLATFORM_DIR)/teltonika/rut9_migration/uci-defaults/* $(1)/etc/uci-defaults/001_rut9_migration; \
		$(CP) $(PLATFORM_DIR)/teltonika/rut9_migration/migrate.conf/* $(1)/etc/migrate.conf; \
	fi;
	if [ $(CONFIG_RUT2_MIGRATION) ]; then \
		mkdir -p $(1)/etc/uci-defaults/001_rut2_migration; \
		mkdir -p $(1)/etc/migrate.conf; \
		$(CP) $(PLATFORM_DIR)/teltonika/rut2_migration/uci-defaults/* $(1)/etc/uci-defaults/001_rut2_migration; \
		$(CP) $(PLATFORM_DIR)/teltonika/rut2_migration/migrate.conf/* $(1)/etc/migrate.conf; \
	fi;
endef

