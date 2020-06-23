#
# Copyright (C) 2019 Teltonika
#

define Develop/clean_prepare
	@# allow only from-git-pulled packages
	@if [ "$(PKG_SOURCE_PROTO)" != "git" ]; then \
		echo "ERROR: Package cannot be reset from development mode" \
		     "because it is not pulled from the git repository!"; \
		exit 1; \
	fi;

	@# if package dir does not contain git then it is not in develop mode
	@if [ ! -d "git" ]; then \
		echo "ERROR: Package cannot be reset from development mode" \
		     "because \`git\` directory does not exist!"; \
		exit 1; \
	fi;

	@# check for untracked/unpushed files
	@if [ ! -z "$$(cd git && git status -s && git cherry -v)" ]; then \
		echo "ERROR: Unpushed contents found!"; \
		exit 1; \
	fi;
endef

define Develop/clean
	$(RM) -rf git
endef

define Develop/prepare
	@# allow only from-git-pulled packages
	@if [ "$(PKG_SOURCE_PROTO)" != "git" ]; then \
		echo "ERROR: Package cannot be set as development mode" \
		     "because it is not pulled from the git repository!"; \
		exit 1; \
	fi;

	@# if package dir contains git then it is in mode already
	@if [ -d "git" ]; then \
		echo "ERROR: Package cannot be set as development mode" \
		     "because \`git\` directory exists!"; \
		exit 1; \
	fi;

	@# clone sources into git dir
	@mkdir git && git clone $(PKG_SOURCE_URL) git

	@# get main project's branch name
	@# checkout (or create) custom branch if it is not master/develop
	@# come back into original dir
	@BRANCH=$$(git rev-parse --abbrev-ref HEAD); \
	if [ "$$BRANCH" != "master" ] && [ "$$BRANCH" != "develop" ] && \
	   [ "$$BRANCH" == "$${BRANCH#release}" ]; then \
		BRANCH="$(PKG_NAME)_$$BRANCH"; \
	else \
		BRANCH="develop"; \
	fi; \
	cd git; \
	git checkout $$BRANCH; \
	if [ "$$?" -eq 0 ]; then \
		git branch --set-upstream-to="origin/$$BRANCH" && git pull; \
	else \
		git checkout -b $$BRANCH; \
	fi;

	@cd ..
endef

define Develop/unstage
	$(Develop/clean_prepare)
	$(Develop/clean)
endef

define Develop/stage
	$(Develop/clean_prepare)
	# grab and modify hash and update pkg version
	HASH=$$(cd git && git rev-parse HEAD); \
	if [ "$$HASH" != "$(PKG_SOURCE_VERSION)" ]; then \
		CURRENT_DAY=$$(date +'%Y-%m-%d'); \
		RELEASE=$(PKG_RELEASE); \
		if [ "$$CURRENT_DAY" == "$(PKG_VERSION)" ]; then \
			((RELEASE++)); \
		else \
			RELEASE=1; \
		fi; \
		sed -i "/PKG_SOURCE_VERSION:=/cPKG_SOURCE_VERSION:=$$HASH" Makefile; \
		sed -i "/PKG_VERSION:=/cPKG_VERSION:=$$(date +'%Y-%m-%d')" Makefile; \
		sed -i "/PKG_RELEASE:=/cPKG_RELEASE:=$$RELEASE" Makefile; \
	fi; \
	$(Develop/clean)
endef
