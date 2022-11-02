#!/usr/bin/env bash

[ "$#" -ne 1 ] && {
	echo "Usage upload_opkg.sh <platform>"
	exit 1
}

if [ -n "${CI_COMMIT_REF_NAME}" ]; then
	BRANCH="${CI_COMMIT_REF_NAME}"
else
	BRANCH="$(git rev-parse --abbrev-ref HEAD)"
fi

if [ "${BRANCH}" = "master" ]; then
	SSH_PRIVATE_KEY="${PRODUCTION_SSH_PRIVATE_KEY}"
	SSH_HOST_KEY="${PRODUCTION_SSH_HOST_KEY}"
	SSH_USER_HOST="${PRODUCTION_SSH_USER_HOST}"
	SSH_PORT="${PRODUCTION_SSH_PORT}"
else
	SSH_PRIVATE_KEY="${TEST_SSH_PRIVATE_KEY}"
	SSH_HOST_KEY="${TEST_SSH_HOST_KEY}"
	SSH_USER_HOST="${TEST_SSH_USER_HOST}"
	SSH_PORT="${TEST_SSH_PORT}"
fi

PLATFORM="$1"
TOPDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." >/dev/null && pwd)
ARCH=$(ls "${TOPDIR}/bin/packages/")
PACKAGEDIR="${TOPDIR}/bin/packages/${ARCH}/pm_packages"
ZIPPEDDIR="${TOPDIR}/bin/packages/${ARCH}/zipped_packages"
TAG=$(git describe --abbrev=0)
HASH=$(echo -n "00/${TAG}/${PLATFORM}" | sha256sum | awk '{print $1}')
FOLDER="/home/admin/opkg_packages/${HASH}"
LINK="/home/admin/opkg_packages/packages/00/${TAG}"

eval "$(ssh-agent -s)"
ssh-add <(echo "${SSH_PRIVATE_KEY}")
mkdir -p ~/.ssh
echo "${SSH_HOST_KEY}" > ~/.ssh/known_hosts

echo "UPLOADING TO ${SSH_USER_HOST#*@}:"
ssh -p "${SSH_PORT}" "${SSH_USER_HOST}" "mkdir -p ${FOLDER}/wiki ${LINK} && ln -fs ${FOLDER} ${LINK}/${PLATFORM}" || exit 1
scp -P "${SSH_PORT}" "${PACKAGEDIR}/"* "${SSH_USER_HOST}:/${FOLDER}/"
scp -P "${SSH_PORT}" "${ZIPPEDDIR}/"* "${SSH_USER_HOST}:/${FOLDER}/wiki/"

trap "ssh-agent -k" exit
exit 0
