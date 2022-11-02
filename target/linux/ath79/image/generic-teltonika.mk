DEVICE_VARS += TPLINK_HWID TPLINK_HWREV TPLINK_FLASHLAYOUT TPLINK_HEADER_VERSION
DEVICE_VARS += UBOOT_NAME UBOOT_SIZE
DEVICE_VARS += CONFIG_SIZE
DEVICE_VARS += ART_SIZE NO_ART
DEVICE_VARS += MASTER_IMAGE_SIZE
DEVICE_VARS += TLT_LEGACY_MODEL
DEVICE_VARS += SIGNATURE_SIZE
DEVICE_VARS += SYSUPGRADE_OFFSET

define rootfs_align
$(patsubst %-256k,0x40000,$(patsubst %-128k,0x20000,$(patsubst %-64k,0x10000,$(patsubst squashfs%,0x4,$(patsubst root.%,%,$(1))))))
endef

define Build/append-tlt-uboot
	dd if="$(BIN_DIR)/u-boot_$(UBOOT_NAME)$(word 1,$(1)).bin" >> $@
endef

define Build/append-tlt-config
	dd if=$(TOPDIR)/target/linux/ath79/image/bin/tlt-factory-config.bin >> $@
endef

define Build/append-tlt-art
	if [ $(NO_ART) == 1 ]; then \
		dd if=/dev/zero bs=$(ART_SIZE) count=1 >> $@; \
	elif [ "$(DEVICE_MODEL)" == "RUT2XX" ]; then \
		dd if=$(TOPDIR)/target/linux/ath79/image/bin/tlt-factory-art-RUT2XX.bin >> $@; \
	elif [ "$(DEVICE_MODEL)" == "TCR1XX" ]; then \
		dd if=$(TOPDIR)/target/linux/ath79/image/bin/tlt-factory-art-TCR1XX.bin >> $@; \
	else \
		dd if=$(TOPDIR)/target/linux/ath79/image/bin/tlt-factory-art.bin >> $@; \
	fi
endef

define Build/mktltfw
	# If building master image, temporarily remove everything up to sysupgrade image start
	$(if $(findstring master,$(word 2,$(1))), \
	cp $@ $@.start; dd if=$@ of=$@.fw skip=1 bs=$(SYSUPGRADE_OFFSET) && mv $@.fw $@)

	-$(STAGING_DIR_HOST)/bin/mktplinkfw \
		-H $(TPLINK_HWID) -W $(TPLINK_HWREV) -F $(TPLINK_FLASHLAYOUT) -N Teltonika \
		-V $(TLT_LEGACY_MODEL)xx \
		-m $(TPLINK_HEADER_VERSION) \
		-k $(IMAGE_KERNEL) \
		-r $@ \
		-o $@.new \
		-j -X 0x40000 \
		-a $(call rootfs_align,$(FILESYSTEM)) \
		$(wordlist 2,$(words $(1)),$(1)) \
		$(if $(findstring sysupgrade,$(word 1,$(1))),-s) && mv $@.new $@ || rm -f $@

	# If building master image, add processed image to master firmware image
	$(if $(findstring master,$(word 2,$(1))), \
	dd if=$@ of=$@.start seek=1 bs=$(SYSUPGRADE_OFFSET) && mv $@.start $@)
endef

define Device/DefaultTeltonika
	DEVICE_VENDOR := TELTONIKA
	SOC := qca9531
	UBOOT_NAME :=
	UBOOT_SIZE := 131072
	CONFIG_SIZE := 65536
	ART_SIZE := 65536
	NO_ART := 0
	IMAGE_SIZE := 15552k
	MASTER_IMAGE_SIZE := 16384k
	SIGNATURE_SIZE := 256
	SYSUPGRADE_OFFSET := 262144
	IMAGES += $(if $(CONFIG_BUILD_FACTORY_IMAGE),master_fw.bin)
	IMAGE/sysupgrade.bin = append-kernel | pad-to $$$$(BLOCKSIZE) | \
			append-rootfs | pad-rootfs | append-metadata | \
			check-size $$$$(IMAGE_SIZE) | finalize-tlt-webui

	IMAGE/master_fw.bin = append-tlt-uboot | pad-to $$$$(UBOOT_SIZE) | \
			append-tlt-config | pad-to $$$$(CONFIG_SIZE) | \
			append-tlt-art | pad-to $$$$(ART_SIZE) | \
			append-kernel | pad-to $$$$(BLOCKSIZE) | \
			append-rootfs | pad-rootfs | \
			check-size $$$$(MASTER_IMAGE_SIZE) | \
			finalize-tlt-master-stendui
endef

define Device/teltonika_trb2xx
	$(Device/DefaultTeltonika)
	DEVICE_MODEL := TRB2XX
	UBOOT_NAME := tlt-trb24x
	NO_ART := 1
	# Default common packages for TRB2XX series
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# Essential must-have:
	DEVICE_PACKAGES := kmod-spi-gpio

	# USB related:
	DEVICE_PACKAGES += kmod-usb-core kmod-usb2 kmod-usb-serial kmod-cypress-serial \
	                   kmod-usb-serial-pl2303 kmod-usb-serial-ftdi
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
endef
TARGET_DEVICES += teltonika_trb2xx

define Device/teltonika_otd1xx
	$(Device/DefaultTeltonika)
	DEVICE_MODEL := OTD1XX
	UBOOT_NAME := tlt-otd1xx
	# Default common packages for OTD1XX series
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# Essential must-have:
	DEVICE_PACKAGES := kmod-spi-gpio

	# USB related:
	DEVICE_PACKAGES += kmod-usb-core kmod-usb2 kmod-usb-serial

	# Wireless related:
	DEVICE_PACKAGES += kmod-ath9k kmod-ath10k ath10k-firmware-qca9887
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
endef
TARGET_DEVICES += teltonika_otd1xx

define Device/teltonika_rut2xx
	$(Device/DefaultTeltonika)
	DEVICE_MODEL := RUT2XX
	DEVICE_FEATURES := small_flash
	SOC := ar9330
	TPLINK_HWID := 0x32200002
	TPLINK_FLASHLAYOUT := 16Mlzma
	TPLINK_HWREV := 0x1
	TPLINK_HEADER_VERSION := 1
	LOADER_TYPE := gz
	KERNEL := kernel-bin | append-dtb | lzma
	KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | tplink-v1-header | \
			append-signature
	TLT_LEGACY_MODEL := RUT2
	UBOOT_NAME := tlt_rut200
	IMAGES = sysupgrade.bin
	IMAGES += $(if $(CONFIG_BUILD_FACTORY_IMAGE),master_fw.bin)
	IMAGES += $(if $(and $(CONFIG_BUILD_FACTORY_IMAGE),$(CONFIG_BUILD_VERIFIED_BOOT_IMAGE)) \
			,master_fw_vboot.bin)
	UBOOT_NAME := tlt-rut2xx

	IMAGE/sysupgrade.bin := append-rootfs | pad-extra $$$$(SIGNATURE_SIZE) | \
			mktltfw sysupgrade | append-signature | \
			append-metadata | finalize-tlt-webui
	IMAGE/master_fw.bin = append-tlt-uboot | pad-to $$$$(UBOOT_SIZE) | \
			append-tlt-config | pad-to $$$$(CONFIG_SIZE) | \
			append-tlt-art | pad-to $$$$(ART_SIZE) | \
			append-rootfs | pad-extra $$$$(SIGNATURE_SIZE) | \
			mktltfw sysupgrade master | append-signature master | \
			check-size $$$$(MASTER_IMAGE_SIZE) | \
			finalize-tlt-master-stendui
	IMAGE/master_fw_vboot.bin = append-tlt-uboot -vboot | pad-to $$$$(UBOOT_SIZE) | \
			append-tlt-config | pad-to $$$$(CONFIG_SIZE) | \
			append-tlt-art | pad-to $$$$(ART_SIZE) | \
			append-rootfs | pad-extra $$$$(SIGNATURE_SIZE) | \
			mktltfw sysupgrade master | append-signature master | \
			check-size $$$$(MASTER_IMAGE_SIZE) | \
			finalize-tlt-master-stendui -vboot

	# Default common packages for RUT2XX series
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# USB related:
	DEVICE_PACKAGES += kmod-usb-core kmod-usb-chipidea2 kmod-usb-serial

	# Wireless related:
	DEVICE_PACKAGES += kmod-ath9k
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
endef
TARGET_DEVICES += teltonika_rut2xx

define Device/teltonika_rut30x
	$(Device/DefaultTeltonika)
	DEVICE_MODEL := RUT30X
	UBOOT_NAME := tlt-rut300
	NO_ART := 1
	# Default common packages for RUT30X series
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# USB related:
	DEVICE_PACKAGES += kmod-usb-core kmod-usb2 kmod-usb-serial kmod-usb-acm \
		kmod-usb-serial-ch341 kmod-usb-serial-pl2303 kmod-usb-serial-ark3116 \
		kmod-usb-serial-belkin kmod-usb-serial-cp210x kmod-usb-serial-cypress-m8 \
		kmod-usb-serial-ftdi
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
endef
TARGET_DEVICES += teltonika_rut30x

define Device/teltonika_rut36x
	$(Device/DefaultTeltonika)
	DEVICE_MODEL := RUT36X
	UBOOT_NAME := tlt-rut360
	# Default common packages for RUT36X series
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# Essential must-have:
	DEVICE_PACKAGES := kmod-spi-gpio

	# USB related:
	DEVICE_PACKAGES += kmod-usb-core kmod-usb2 kmod-usb-serial

	# Wireless related:
	DEVICE_PACKAGES += kmod-ath9k
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
endef
TARGET_DEVICES += teltonika_rut36x

define Device/teltonika_rut9xx
	$(Device/DefaultTeltonika)
	DEVICE_MODEL := RUT9XX
	DEVICE_FEATURES := small_flash
	SOC := ar9344
	TPLINK_HWID := 0x35000001
	TPLINK_FLASHLAYOUT := 16Mlzma
	TPLINK_HWREV := 0x1
	TPLINK_HEADER_VERSION := 1
	LOADER_TYPE := gz
	KERNEL := kernel-bin | append-dtb | lzma
	KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | tplink-v1-header | \
				append-signature
	TLT_LEGACY_MODEL := RUT9
	IMAGES = sysupgrade.bin
	IMAGES += $(if $(CONFIG_BUILD_FACTORY_IMAGE),master_fw.bin)
	IMAGES += $(if $(and $(CONFIG_BUILD_FACTORY_IMAGE),$(CONFIG_BUILD_VERIFIED_BOOT_IMAGE)) \
				,master_fw_vboot.bin)
	IMAGE/sysupgrade.bin = append-rootfs | pad-extra $$$$(SIGNATURE_SIZE) | \
				mktltfw sysupgrade | append-signature | \
				append-metadata | finalize-tlt-webui
	UBOOT_NAME := tlt-rut9xx
	IMAGE/master_fw.bin = append-tlt-uboot | pad-to $$$$(UBOOT_SIZE) | \
			append-tlt-config | pad-to $$$$(CONFIG_SIZE) | \
			append-tlt-art | pad-to $$$$(ART_SIZE) | \
			append-rootfs | pad-extra $$$$(SIGNATURE_SIZE) | \
			mktltfw sysupgrade master | append-signature master | \
			check-size $$$$(MASTER_IMAGE_SIZE) | \
			finalize-tlt-master-stendui
	IMAGE/master_fw_vboot.bin = append-tlt-uboot -vboot | pad-to $$$$(UBOOT_SIZE) | \
			append-tlt-config | pad-to $$$$(CONFIG_SIZE) | \
			append-tlt-art | pad-to $$$$(ART_SIZE) | \
			append-rootfs | pad-extra $$$$(SIGNATURE_SIZE) | \
			mktltfw sysupgrade master | append-signature master | \
			check-size $$$$(MASTER_IMAGE_SIZE) | \
			finalize-tlt-master-stendui -vboot
	# Default common packages for RUT9XX series
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# Essential must-have:
	DEVICE_PACKAGES := kmod-spi-gpio kmod-mmc-spi kmod-i2c-gpio kmod-hwmon-core \
			kmod-hwmon-mcp3021 

	# USB related:
	DEVICE_PACKAGES += kmod-usb-core kmod-usb2 kmod-usb-serial kmod-usb-acm \
			kmod-usb-serial-ch341 kmod-usb-serial-pl2303 kmod-usb-serial-ark3116 \
			kmod-usb-serial-belkin kmod-usb-serial-cp210x kmod-usb-serial-cypress-m8 \
			kmod-usb-serial-ftdi

	# Wireless related:
	DEVICE_PACKAGES += kmod-ath9k
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
endef
TARGET_DEVICES += teltonika_rut9xx

define Device/teltonika_rut952
	$(Device/DefaultTeltonika)
	DEVICE_MODEL := RUT952
	DEVICE_FEATURES := small_flash
	SOC := ar9344
	UBOOT_NAME := tlt-rut952
	# Default common packages for RUT952
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# Essential must-have:
	DEVICE_PACKAGES := kmod-spi-gpio kmod-mmc-spi kmod-i2c-gpio kmod-hwmon-core \
			kmod-hwmon-mcp3021 meig_flasher kmod-sfp kmod-phylink kmod-i2c

	# USB related:
	DEVICE_PACKAGES += kmod-usb-core kmod-usb2 kmod-usb-serial

	# Wireless related:
	DEVICE_PACKAGES += kmod-ath9k
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
endef
TARGET_DEVICES += teltonika_rut952

define Device/teltonika_tcr1xx
	$(Device/DefaultTeltonika)
	DEVICE_MODEL := TCR1XX
	UBOOT_NAME := tlt-tcr1xx
	# Default common packages for TCR1XX series
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# Essential must-have:
	DEVICE_PACKAGES := pepe2k_uboot-ath79 kmod-spi-gpio

	# USB related:
	DEVICE_PACKAGES += kmod-usb-core kmod-usb2 kmod-usb-serial

	# Wireless related:
	DEVICE_PACKAGES += kmod-ath9k kmod-ath10k ath10k-firmware-qca9887
	# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
endef
TARGET_DEVICES += teltonika_tcr1xx


