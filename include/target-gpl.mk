#
# Copyright (C) 2021 Teltonika
#

define GPL/Target/Build/Default
	cp -rf . "$(1)/"
endef

GPL/Target/Build=$(call GPL/Target/Build/Default,$(1))

$(BOARD)/gpl:
	$(call GPL/Target/Build,$(1))
