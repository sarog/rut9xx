#
# Copyright (C) 2021 Teltonika
#

define GPL/Build/Default
	cp -rf . "$(1)/"
endef

GPL/Build=$(call GPL/Build/Default,$(1))

gpl:
	$(call GPL/Build,$(1))
