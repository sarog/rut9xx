#!/bin/sh

BRANCH="$(git name-rev --name-only HEAD)"
[ -n "$CI_COMMIT_REF_NAME" ] && BRANCH="$CI_COMMIT_REF_NAME"
RELEASE_VER="${BRANCH##*/}"
RELEASE_MAJOR="$(printf %02d $(echo $RELEASE_VER | awk -F . '{ print $1 }'))"
RELEASE_MINOR="$(printf %02d $(echo $RELEASE_VER | awk -F . '{ print $2 }'))"
RELEASE_PATCH="$(echo $RELEASE_VER | awk -F . '{ print $3 }')"

[ -n "$RELEASE_MAJOR" -a -n "$RELEASE_MINOR" ] || {
	echo "Unable to detect release version!"
	exit 1
}

[ "${BRANCH#release}" = "${BRANCH}" -a "${BRANCH#hotfix}" = "${BRANCH}" ] && {
	echo "This script should be executed from release or hotfix branch!"
	exit 1
}

echo -n "\
CONFIG_TLT_VERSIONING_MANUAL_ENABLE=y
CONFIG_TLT_VERSIONING_RELEASE=\"R\"
CONFIG_TLT_VERSIONING_MAJOR=\"$RELEASE_MAJOR\"
CONFIG_TLT_VERSIONING_MINOR=\"$RELEASE_MINOR\"" >> .config

[ -n "$RELEASE_PATCH" ] && {
	echo -n "
CONFIG_TLT_VERSIONING_PATCH_ENABLE=y
CONFIG_TLT_VERSIONING_PATCH=\"$RELEASE_PATCH\"" >> .config
}

export FAKE_RELEASE_BUILD=1

make defconfig
