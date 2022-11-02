include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR = 0x80060000

DEVICE_VARS += LOADER_FLASH_OFFS LOADER_TYPE
DEVICE_VARS += LOADER_FLASH_MAX LOADER_KERNEL_MAGIC

define Device/Default
  DEVICE_DTS_DIR := ../dts
  DEVICE_DTS = $$(SOC)_$(1)
  PROFILES = Default
  MTDPARTS :=
  BLOCKSIZE := 64k
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | uImage lzma
  LOADER_KERNEL_MAGIC :=
  LOADER_FLASH_MAX :=
  LOADER_FLASH_OFFS :=
  LOADER_TYPE :=
  COMPILE :=
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin = append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | append-metadata | check-size
  HW_MODS := mod1%ftdi_new
endef

include generic-teltonika.mk

$(eval $(call BuildImage))
