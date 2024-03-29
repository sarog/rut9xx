#
# Copyright (C) 2016 Piotr Dymacz <piotr@dymacz.pl>
#
# SPDX-License-Identifier: GPL-2.0
#

SHELL = bash

HOSTARCH := $(shell uname -m |        \
	      sed -e s/i.86/x86_32/   \
		  -e s/sun4u/sparc64/ \
		  -e s/arm.*/arm/     \
		  -e s/sa110/arm/     \
		  -e s/powerpc/ppc/   \
		  -e s/macppc/ppc/)

HOSTOS := $(shell uname -s | tr '[:upper:]' '[:lower:]' | \
	    sed -e 's/\(cygwin\).*/cygwin/')

ifneq ($(HOSTOS), darwin)
  ifneq ($(HOSTOS), linux)
    $(error Error! Unsupported host operating system/arch: "$(HOSTOS)-$(HOSTARCH)")
  endif
endif

ifndef BUILD_TOPDIR
	BUILD_TOPDIR = $(PWD)
endif
export BUILD_TOPDIR

export HOSTOS
export HOSTARCH
export STAGING_DIR  = $(BUILD_TOPDIR)/tmp
export SOURCE_DIR   = $(BUILD_TOPDIR)/u-boot
export BIN_DIR      = $(BUILD_TOPDIR)/bin
export SUB_MAKE_CMD = $(MAKE) --silent --no-print-directory \
                      ARCH=mips V=1 SHELL=$(SHELL)

export VERSION_PATH=$(BUILD_TOPDIR)/version
export RESERVE_BYTES=10
export RESERVE_BYTES_NAME=12

# ==========================================================================
# You can override some default configuration options below or pass them on
# command line, for example:
# make ... IMG_SIZE=256 IMG_LZMA=0 CROSS_COMPILE=...

# Set to 1 if you want to build RAM-loadable image, without low level
# initialization of the hardware (PLL/clocks, DRAM):
# IMG_RAM =

# You can change limit of the image size in KB with below option (image will
# be filled up to the selected size with 0xFF):
# IMG_SIZE =

# If you don't want LZMA compressed image, set below option to 1 (by default
# images for all targets are LZMA compressed):
# IMG_LZMA =

# Define _absolute_ path to your toolchain directory, for example:
# export TOOLCHAIN_DIR:=/home/user/toolchain-mips_24kc_gcc-5.4.0_musl-1.1.15
# export PATH:=$(TOOLCHAIN_DIR)/bin:$(PATH)

ifndef CROSS_COMPILE
  CROSS_COMPILE = mips-openwrt-linux-musl-
endif
export CROSS_COMPILE

# By default, optimization for size (-Os) is enabled, set below option
# to n or remove it if you want only basic optimization (-O/-O1)
# BUILD_OPTIMIZED = n

ifneq ($(BUILD_OPTIMIZED), n)
  BUILD_OPTIMIZED = y
endif
export BUILD_OPTIMIZED

# ==========================================================================

# =======================
# CUSTOM HELPER FUNCTIONS
# =======================

define echo_green
  echo -e "\e[92m$(1)\e[0m"
endef

define echo_red
  echo -e "\e[91m$(1)\e[0m"
endef

define echo_blue
  echo -e "\e[96m$(1)\e[0m"
endef

# $(1): size
define img_size
$(if $(IMG_SIZE),$(strip $(IMG_SIZE)),$(strip $(1)))
endef

# $(1): value
define is_lzma
$(if $(IMG_LZMA),\
  $(if $(filter $(strip $(IMG_LZMA)),1),1,0),\
  $(if $(filter $(strip $(1)),1),1,0)\
)
endef

define git_describe
$(shell git describe HEAD 2>/dev/null || echo "unknown")
endef

# $(1): file extension
define img_name
u-boot_$@_$(call git_describe)$(if $(filter $(IMG_RAM),1), \
	_RAM-LOAD-ONLY)$(if $(1),.$(1))
endef

define md5_sum
  $(call echo_green,Calculating MD5 sum for the final image...)

  md5sum $(BIN_DIR)/$(call img_name,bin) | \
         awk '{print $$1}' | \
         tr -d '\n' > $(BIN_DIR)/$(call img_name).md5

  echo ' *'$(call img_name,bin) >> $(BIN_DIR)/$(call img_name,md5)
endef

# $(1): size
define padded_img
  $(call echo_green,Preparing $(1) KB image padded with 0xFF...)
  tr "\000" "\377" < /dev/zero | dd ibs=1k count=$(1) \
     of=$(BIN_DIR)/$(call img_name,bin) 2> /dev/null
endef

define final_img
  $(call echo_green,Preparing final image...)
  mv $(BIN_DIR)/temp.bin $(BIN_DIR)/$(call img_name,bin)
endef

# $(1): path to image
# $(2): size limit in KB
define size_chk
  $(call echo_green,Checking size of the image...)

  if [ `wc -c < $(1)` -gt $$(($(2) * 1024)) ]; then \
    echo; \
    $(call echo_red,  ======================); \
    $(call echo_red,  IMAGE SIZE IS TOO BIG!); \
    $(call echo_red,  ======================); \
    echo; \
    rm -f $(1); \
    exit 1; \
  fi;
endef

# $(1): filename of image to copy
# $(2): image size limit (check if set)
define copy_img
  echo;
  $(call echo_green,Copying compiled image...)

  cp $(SOURCE_DIR)/$(strip $(1)).bin $(BIN_DIR)/temp.bin
  $(if $(2),$(call size_chk,$(BIN_DIR)/temp.bin,$(2)))
endef

# $(1): size limit in KB
# $(2): if set to 1, use LZMA
# $(3): other parameters passed to subdir make
define build
  args="IMG_SIZE=$$((1024*$(call img_size,$(1)))) \
        IMG_LZMA=$(strip $(call is_lzma,$(2))) \
        $(strip $(3))"; \
  cd $(SOURCE_DIR) && \
     $(SUB_MAKE_CMD) $@ $$args && \
     $(SUB_MAKE_CMD) all $$args

  $(if $(filter $(IMG_RAM),1),\
    $(call copy_img,u-boot), \
    $(if $(filter $(strip $(call is_lzma,$(2))),1), \
      $(call copy_img,tuboot,$(call img_size,$(1))), \
      $(call copy_img,u-boot,$(call img_size,$(1))) \
    ) \
  )

  $(foreach v,$(subst =,:=,$(3)),$(eval $v))

  $(if $(filter $(IMG_RAM),1),,$(call padded_img,$(1)))
  $(call final_img)

  @printf "%-$(RESERVE_BYTES_NAME)s" "$(DEVICE_MODEL)" | dd of=$(BUILD_TOPDIR)/bin/$(call img_name).bin bs=1 count=$(RESERVE_BYTES_NAME) conv=notrunc seek=$(shell expr $(call img_size,$(1)) \* 1024 - $(RESERVE_BYTES) - $(RESERVE_BYTES_NAME));
  @printf "%-$(RESERVE_BYTES)s" "$(shell cat '$(VERSION_PATH)')$(VERIFIED_BOOT_STR)" | dd of=$(BUILD_TOPDIR)/bin/$(call img_name).bin bs=1 count=$(RESERVE_BYTES) conv=notrunc seek=$(shell expr $(call img_size,$(1)) \* 1024 - $(RESERVE_BYTES));

  @cd $(BUILD_TOPDIR)/crc32/ && $(MAKE) all
  @cp $(BUILD_TOPDIR)/bin/$(call img_name,bin) $(BUILD_TOPDIR)/bin/$(call img_name)_webui.bin
  @$(BUILD_TOPDIR)/crc32/crc32 $(BUILD_TOPDIR)/bin/$(call img_name,bin) | awk '{ print $$1 }' >> $(BUILD_TOPDIR)/bin/$(call img_name)_webui.bin

  $(call md5_sum)
  echo;
  $(call echo_green,DONE!)
  $(call echo_green,Image 'bin/$(call img_name,bin)' is ready!)

  if [ "x$$IMG_RAM" = "x1" ]; then \
    echo; \
    $(call echo_blue,  ================================); \
    $(call echo_blue,  THIS IMAGE IS ONLY FOR RAM LOAD!); \
    $(call echo_blue,  DO NOT WRITE IT INTO FLASH CHIP!); \
    $(call echo_blue,  ================================); \
    echo; \
  fi;
endef

# ===========================================
# TARGETS IN ALPHABETICAL ORDER, SHARED FIRST
# ===========================================

COMMON_AR933X_TARGETS = \
	gainstrong_oolite_v1_dev \
	gl-inet_6416 \
	hak5_lan-turtle \
	hak5_packet-squirrel \
	hak5_wifi-pineapple-nano \
	tp-link_tl-mr10u_v1 \
	tp-link_tl-mr13u_v1 \
	tp-link_tl-mr3020_v1 \
	tp-link_tl-mr3040_v1v2 \
	tp-link_tl-mr3220_v2 \
	tp-link_tl-wr703n_v1 \
	tp-link_tl-wr710n_v1 \
	tp-link_tl-wr720n_v3_CN \
	tp-link_tl-wr740n_v4

$(COMMON_AR933X_TARGETS):
	@$(call build,123,1)

COMMON_ETHS27_TARGETS = \
	gainstrong_oolite_v5.2 \
	gainstrong_oolite_v5.2_dev \
	tp-link_tl-mr22u_v1 \
	tp-link_tl-mr3420_v2 \
	tp-link_tl-mr3420_v3 \
	tp-link_tl-mr6400_v1v2 \
	tp-link_tl-wa801nd_v2 \
	tp-link_tl-wa830re_v2 \
	tp-link_tl-wa850re_v2 \
	tp-link_tl-wdr3500_v1 \
	tp-link_tl-wr802n_v1 \
	tp-link_tl-wr810n_v1 \
	tp-link_tl-wr810n_v2 \
	tp-link_tl-wr820n_v1_CN \
	tp-link_tl-wr841n_v10 \
	tp-link_tl-wr841n_v11 \
	tp-link_tl-wr841n_v8 \
	tp-link_tl-wr841n_v9 \
	tp-link_tl-wr842n_v3 \
	tp-link_tl-wr902ac_v1

$(COMMON_ETHS27_TARGETS):
	@$(call build,123,1,ETH_CONFIG=_s27)

tlt-trb24x:
	@$(call build,123,1,\
		ETH_CONFIG=_s27 \
		DEVICE_VENDOR=teltonika \
		DEVICE_MODEL=TRB2XX \
		CFG_74X164=1)

tlt-rut2xx:
	@$(call build,128,1,\
		DEVICE_VENDOR=teltonika \
		DEVICE_MODEL=RUT2xx)

tlt-rut2xx-vboot:
	@$(call build,128,1,\
		DEVICE_VENDOR=teltonika_VerifiedBoot \
		DEVICE_MODEL=RUT2xx \
		CFG_TLT_VERIFIED_BOOT=1 \
		VERIFIED_BOOT_STR=-vb)

tlt-rut2xx-vboot-64k:
	@$(call build,64,1,\
		DEVICE_VENDOR=teltonika_recovery \
		DEVICE_MODEL=RUT2xx \
		CFG_TLT_VERIFIED_BOOT=1 \
		VERIFIED_BOOT_STR=-vbr)

tlt-rut2m:
	@$(call build,128,0,\
		DEVICE_VENDOR=teltonika \
		DEVICE_MODEL=RUT2M)

tlt-rut300:
	@$(call build,123,1,\
		ETH_CONFIG=_s27 \
		DEVICE_VENDOR=teltonika \
		DEVICE_MODEL=RUT300)

tlt-rut360:
	@$(call build,123,1,\
		ETH_CONFIG=_s27 \
		DEVICE_VENDOR=teltonika \
		DEVICE_MODEL=RUT360 \
		CFG_74X164=1)

tlt-rut9xx:
	@$(call build,128,1,\
		ETH_CONFIG=_s27 \
		DEVICE_VENDOR=teltonika \
		DEVICE_MODEL=RUT9XX \
		CFG_74X164=1)

tlt-rut9xx-vboot:
	@$(call build,128,1,\
		ETH_CONFIG=_s27 \
		DEVICE_VENDOR=teltonika_VerifiedBoot \
		DEVICE_MODEL=RUT9XX \
		CFG_74X164=1 \
		CFG_TLT_VERIFIED_BOOT=1 \
		VERIFIED_BOOT_STR=-vb)

tlt-rut9xx-vboot-64k:
	@$(call build,64,1,\
		ETH_CONFIG=_s27 \
		DEVICE_VENDOR=teltonika_recovery \
		DEVICE_MODEL=RUT9XX \
		CFG_74X164=1 \
		CFG_TLT_VERIFIED_BOOT=1 \
		VERIFIED_BOOT_STR=-vbr)


tlt-rut952:
	@$(call build,128,1,\
		ETH_CONFIG=_s27 \
		DEVICE_VENDOR=teltonika \
		DEVICE_MODEL=RUT952 \
		CFG_74X164=1)

tlt-rut952-vboot:
	@$(call build,128,1,\
		ETH_CONFIG=_s27 \
		DEVICE_VENDOR=teltonika_VerifiedBoot \
		DEVICE_MODEL=RUT952 \
		CFG_74X164=1 \
		CFG_TLT_VERIFIED_BOOT=1 \
		VERIFIED_BOOT_STR=-vb)

tlt-rut952-vboot-64k:
	@$(call build,64,1,\
		ETH_CONFIG=_s27 \
		DEVICE_VENDOR=teltonika_recovery \
		DEVICE_MODEL=RUT952 \
		CFG_74X164=1 \
		CFG_TLT_VERIFIED_BOOT=1 \
		VERIFIED_BOOT_STR=-vbr)

tlt-rut9m:
	@$(call build,128,0,\
		DEVICE_VENDOR=teltonika \
		DEVICE_MODEL=RUT9M)

tlt-tcr1xx:
	@$(call build,123,1,\
		ETH_CONFIG=_s27 \
		DEVICE_VENDOR=teltonika \
		DEVICE_MODEL=TCR1XX \
		CFG_74X164=1)

tlt-otd1xx:
	@$(call build,123,1,\
		ETH_CONFIG=_s27 \
		DEVICE_VENDOR=teltonika \
		DEVICE_MODEL=OTD1XX \
		CFG_74X164=1)

8devices_carambola2 \
alfa-network_hornet-ub \
alfa-network_tube2h \
creatcomm-technology_d3321 \
gl-inet_gl-ar150 \
gl-inet_gl-usb150:
	@$(call build,256,1)

alfa-network_ap121f:
	@$(call build,192,1)

alfa-network_n5q \
alfa-network_r36a:
	@$(call build,384,1,ETH_CONFIG=_s27)

comfast_cf-e314n \
comfast_cf-e320n_v2 \
comfast_cf-e520n \
comfast_cf-e530n:
	@$(call build,64,1,ETH_CONFIG=_s27)

d-link_dir-505_a1:
	@$(call build,64,1)

dragino_ms14:
	@$(call build,192,1,DEVICE_VENDOR=dragino)

engenius_ens202ext \
gl-inet_gl-ar300 \
gl-inet_gl-ar300m-lite \
gl-inet_gl-ar750 \
p2w_cpe505n \
p2w_r602n \
yuncore_ap90q \
yuncore_cpe830 \
yuncore_t830 \
whqx_e600g_v2 \
whqx_e600gac_v2 \
zbtlink_zbt-we1526:
	@$(call build,256,1,ETH_CONFIG=_s27)

tp-link_tl-wdr3600_v1 \
tp-link_tl-wdr43x0_v1 \
tp-link_tl-wr1041n_v2:
	@$(call build,123,1,ETH_CONFIG=_s17)

unwireddevices_unwired-one:
	@$(call build,128,1,DEVICE_VENDOR=SE)

village-telco_mesh-potato_v2:
	@$(call build,192,1,DEVICE_VENDOR=villagetelco)

wallys_dr531:
	@$(call build,192,1,ETH_CONFIG=_s27)

yuncore_cpe870:
	@$(call build,64,1,ETH_CONFIG=_s27)

# =============
# CLEAN TARGETS
# =============

lzma_host_clean:
	@cd $(SOURCE_DIR) && $(SUB_MAKE_CMD) $@

rsa_pkey_tool_host_clean:
	@cd $(SOURCE_DIR) && $(SUB_MAKE_CMD) $@

clean:
	@cd $(SOURCE_DIR) && $(SUB_MAKE_CMD) distclean
	@rm -f $(SOURCE_DIR)/httpd/fsdata.c

clean_all: clean
	@$(call echo_green,Removing all binary images...)
	@rm -f $(BIN_DIR)/*.bin
	@rm -f $(BIN_DIR)/*.md5
