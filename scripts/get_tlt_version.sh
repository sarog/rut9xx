#!/usr/bin/env bash
export LANG=C
export LC_ALL=C
TOPDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." >/dev/null && pwd)"

if [ -s "${TOPDIR}/gpl_version" ]; then
	read -r gpl_version < "${TOPDIR}/gpl_version"
	printf "%s" "${gpl_version}"
	exit 0
fi

exit 1