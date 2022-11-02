#!/bin/sh

export TOPDIR="$(pwd)"
export INCLUDE_DIR="${TOPDIR}/include"

MAKE_CMD="$(which make) -j$(nproc) -C tools/tar"

# launched too early, because it does not exist (yet), skip for now
[ -f "staging_dir/host/bin/tar" ] || exit 0

v1="$(grep -Po 'PKG_VERSION:=\K[^ ]+' tools/tar/Makefile)"
v2="$(./staging_dir/host/bin/tar --version | grep -Po 'tar \(GNU tar\) \K[^ ]+')"

# different TAR versions... rebuild
[ "$v1" != "$v2" ] && {
        $MAKE_CMD clean &>/dev/null; $MAKE_CMD compile &>/dev/null
        exit 1
}

exit 0
