#!/bin/sh

out="$1"
version="$2"
page_size="$3"
rootfs="$4"
kernel="$5"
modem="$6"

rootfs_name=$(basename $rootfs)
kernel_name=$(basename $kernel)
rootfs_hash=$(sha1sum $rootfs | awk '{ print $1 }')
kernel_hash=$(sha1sum $kernel | awk '{ print $1 }')

[ ! -z "$modem" ] && {
    modem_name=$(basename $modem)
    modem_hash=$(sha1sum $modem | awk '{ print $1 }')
}

machine=""
region=""
branch=""
client=""
major=""
minor=""
patch=""

parse_version() {
    local IFS=_
    set $1

    machine=$(echo $1 | awk '{ str=substr($0, 1, 6); print str; }')
    region=$(echo $1 | awk '{ str=substr($0, 7, 1); print str; }')
    branch=$3
    split_version $4

    [ "$2" = "R" ] && {
        branch="$2"
        split_version $3
    }
}

split_version() {
    local IFS=.
    set $1

    client=$1
    major=$2
    minor=$3
    patch=$4
}

parse_version $version

if [ ! -z "$modem" ]; then
    cat <<EOF > "$1"
    {
        "version": "$version",
        "region": "$region",
        "machine": "$machine",
        "version_major": "$major",
        "version_minor": "$minor",
        "version_patch": "$patch",
        "branch": "$branch",
        "client": "$client",
        "page_size": "$page_size",
        "images": [
            {
                "version": "5",
                "type": "rootfs",
                "filename": "$rootfs_name",
                "sha1": "$rootfs_hash",
                "depends": [
                    {
                        "force": true,
                        "type": "kernel"
                    }
                ],
                "format": "ubi"
            },
            {
                "version": "7",
                "type": "kernel",
                "filename": "$kernel_name",
                "sha1": "$kernel_hash",
                "depends": [
                    {
                        "force": true,
                        "type": "rootfs"
                    }
                ],
                "format": "raw"
            },
            {
                "version": "10",
                "filename": "$modem_name",
                "sha1": "$modem_hash",
                "type": "modem",
                "format": "ubi"
            }
        ]
    }
EOF
else
    cat <<EOF > "$1"
    {
        "version": "$version",
        "region": "$region",
        "machine": "$machine",
        "version_major": "$major",
        "version_minor": "$minor",
        "version_patch": "$patch",
        "branch": "$branch",
        "client": "$client",
        "page_size": "$page_size",
        "images": [
            {
                "version": "5",
                "type": "rootfs",
                "filename": "$rootfs_name",
                "sha1": "$rootfs_hash",
                "depends": [
                    {
                        "force": true,
                        "type": "kernel"
                    }
                ],
                "format": "ubi"
            },
            {
                "version": "7",
                "type": "kernel",
                "filename": "$kernel_name",
                "sha1": "$kernel_hash",
                "depends": [
                    {
                        "force": true,
                        "type": "rootfs"
                    }
                ],
                "format": "raw"
            }
        ]
    }
EOF
fi
