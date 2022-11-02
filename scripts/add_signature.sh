#!/usr/bin/env bash
#
# Copyright (c) 2021, Teltonika Networks
#
# This is free software, licensed under the GNU General Public License v2.

TOPDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." >/dev/null && pwd)"
STAGING_DIR_HOST="$TOPDIR/staging_dir/host"
PRIV_KEY="key-build-authenta.pem"
FW_BIN="$1"
ROOTFS_FW_BIN="$2"
TMP_DIR="$(dirname "$FW_BIN")"
SIGNATURE="signature.sha256"
SIGNATURE_SIZE="256"
FW_SIZE="$(stat --format=%s "$FW_BIN")"
IS_MASTER="$3"
# SYSUPGR_OFFSET = 0x40000 in hex
SYSUPGR_OFFSET=262144

echo "Passed param 1: $1, signature will be added to $ROOTFS_FW_BIN"

if [ ! -f "$TOPDIR/$PRIV_KEY" ]; then
	echo "Generating SSL key"
	openssl genrsa -out "$TOPDIR/$PRIV_KEY" 2048
fi

if [ ! -s "$TOPDIR/$PRIV_KEY" ]; then
	echo "NO KEY, FAIL!"
	cp "$FW_BIN" "$ROOTFS_FW_BIN"
	exit 0
fi

if [ -n "$IS_MASTER" ]; then
	mv "$FW_BIN" "$FW_BIN.fw"
	# Get firmware without uboot+config+art, only from tplinkfw header
	dd if="$FW_BIN.fw" of="$FW_BIN" skip=1 bs="$SYSUPGR_OFFSET"
fi

# Get firmware image information from tplinkfw header
MKTPLINKFW_OUT="$("$STAGING_DIR_HOST"/bin/mktplinkfw -i "$1")"


# Get squashfs rootfs offset and length
ROOTFS_OFFSET="$(echo "$MKTPLINKFW_OUT" | grep "Rootfs data offset" \
	| awk '{print $7}')"
ROOTFS_SIZE="$(echo "$MKTPLINKFW_OUT" | grep "Rootfs data length" \
	| awk '{print $7}')"

# If rootfs offset or/and size is zero, we are checking initramfs image
if [ "$ROOTFS_OFFSET" -eq 0 ] || [ "$ROOTFS_SIZE" -eq 0 ]; then
	# Get kernel + initramfs offset and length
	KERNEL_OFFSET="$(echo "$MKTPLINKFW_OUT" | grep "Kernel data offset" \
		| awk '{print $7}')"
	KERNEL_SIZE="$(echo "$MKTPLINKFW_OUT" | grep "Kernel data length" \
		| awk '{print $7}')"
	# Signature is put right after kernel + initramfs
	SIGNATURE_OFFSET="$((KERNEL_OFFSET + KERNEL_SIZE))"

	cp "$FW_BIN" "$ROOTFS_FW_BIN"
else
	# Signature is put right after squashfs rootfs
	SIGNATURE_OFFSET="$((ROOTFS_OFFSET + ROOTFS_SIZE))"

	# Get binary image part only of data which needs to be signed.
	# Data to be signed = tplinkfw header + kernel + squashfs rooths
	dd if="$FW_BIN" of="$ROOTFS_FW_BIN" bs="$((SIGNATURE_OFFSET - SIGNATURE_SIZE))" count=1
fi

echo "SIGNATURE OFFSET: $SIGNATURE_OFFSET"

# Sign data
openssl dgst -sha256 -sign "$TOPDIR/$PRIV_KEY" -out "$TMP_DIR/$SIGNATURE" "$ROOTFS_FW_BIN"

echo "SIGNATURE:"
echo "$(hexdump $TMP_DIR/$SIGNATURE)"

if [ "$ROOTFS_OFFSET" -eq 0 ] || [ "$ROOTFS_SIZE" -eq 0 ]; then
	# Make final signed binary image
	# Signed image structure:
	# tplinkheader + kernel + initramfs + signature(tplinkheader + kernel + initramfs)
	cat "$TMP_DIR/$SIGNATURE" >> "$ROOTFS_FW_BIN"
else
	# Get binary image part only of data after signature (jffs2 padding)
	dd if="$FW_BIN" of="$FW_BIN.tmp2" bs=1 skip="$SIGNATURE_OFFSET" count="$((FW_SIZE - SIGNATURE_OFFSET))"

	# Make final signed binary image
	# Signed image structure:
	# tplinkheader + kernel + squashfs rootfs + signature(tplinkheader + kernel + squashfs rootfs) + jffs2 padding
	cat "$TMP_DIR/$SIGNATURE" "$FW_BIN.tmp2" >> "$ROOTFS_FW_BIN"

	if [ -n "$IS_MASTER" ]; then
		# Get uboot+config+art bin part, only up to tplinkfw header
		dd if="$FW_BIN.fw" of="$FW_BIN.boot" count=1 bs="$SYSUPGR_OFFSET"
		# Add signed image to uboot+config+art bin part
		cat "$ROOTFS_FW_BIN" >> "$FW_BIN.boot"
		mv "$FW_BIN.boot" "$ROOTFS_FW_BIN"
		rm "$FW_BIN.fw"
	fi

	rm "$FW_BIN.tmp2"
fi
