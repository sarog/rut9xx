
KDIR	:= $(PWD)/../../build_dir/target-mips_34kc_uClibc-0.9.33.2/linux-ar71xx_generic/linux-3.10.36
PWD	:= $(shell pwd)
ROOTDIR := $(PWD)/modules

# Default architecture is MIPS
ifndef $(KERNELARCH)
KERNELARCH=mips
endif

ARC = $(KERNELARCH)
CROSS_CC := $(PWD)/../../staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mips-openwrt-linux-uclibc-


ifeq ($(POWERPC),1)
ARC     :=mips
CROSS_CC :=powerpc-none-linux-gnuspe-
endif

export KDIR
export ROOTDIR
export CROSS_CC

all: default

# default:
# 	@echo Making generic AP art module build. This build target is used for 3rd party AP processor. 
# 	make ARCH=$(ARC) PB42=1 DEBUG=$(DEBUG) CROSS_COMPILE=$(CROSS_CC) -C $(KDIR) M=$(PWD)/modules modules

default:
	@echo Making art module for Teltonika RUT9xx design.
	make ARCH=$(ARC) AP83=1 WASP=1 DEBUG=$(DEBUG) CROSS_COMPILE=$(CROSS_CC) -C $(KDIR) M=$(PWD)/modules modules

#RUT9ART is based on 'wasp'
RUT9ART:
	@echo Making art module for Teltonika RUT9xx design.
	make ARCH=$(ARC) AP83=1 WASP=1 DEBUG=$(DEBUG) CROSS_COMPILE=$(CROSS_CC) -C $(KDIR) M=$(PWD)/modules modules

hydra:
	@echo Making art module for Atheros hydra based design.
	make ARCH=$(ARC) PB42=1 DEBUG=$(DEBUG) CROSS_COMPILE=$(CROSS_CC) -C $(KDIR) M=$(PWD)/modules modules

howl:
	@echo Making art module for Atheros howl based design.
	make ARCH=$(ARC) AP83=1 DEBUG=$(DEBUG) CROSS_COMPILE=$(CROSS_CC) -C $(KDIR) M=$(PWD)/modules modules

python:
	@echo Making art module for Atheros python based design.
	make ARCH=$(ARC) PYTHON_EMU=1 DEBUG=$(DEBUG) CROSS_COMPILE=$(CROSS_CC) -C $(KDIR) M=$(PWD)/modules modules

wasp:
	@echo Making art module for Atheros wasp based design.
	make ARCH=$(ARC) AP83=1 WASP=1 DEBUG=$(DEBUG) CROSS_COMPILE=$(CROSS_CC) -C $(KDIR) M=$(PWD)/modules modules
	
honeybee:
	@echo Making art module for Atheros wasp based design.
	make ARCH=$(ARC) AP83=1 WASP=1 DEBUG=$(DEBUG) CROSS_COMPILE=$(CROSS_CC) -C $(KDIR) M=$(PWD)/modules modules	

wasp_osprey:
	@echo Making art module for Atheros wasp-osprey DBSC based design.
	make ARCH=$(ARC) AP83=1 WASP=1 PYTHON_EMU=1 WASP_OSPREY=1 DEBUG=$(DEBUG) CROSS_COMPILE=$(CROSS_CC) -C $(KDIR) M=$(PWD)/modules modules

scorpion:
	@echo Making art module for Atheros wasp based design.
	make ARCH=$(ARC) AP83=1 WASP=1 DEBUG=$(DEBUG) CROSS_COMPILE=$(CROSS_CC) -C $(KDIR) M=$(PWD)/modules modules

scorpion_pci:
	@echo Making art module for Atheros Scorpion-peacock_pci slot 1 or 2 DBSC based design.
	make ARCH=$(ARC) AP83=1 WASP=1 PYTHON_EMU=1 WASP_OSPREY=1 DEBUG=$(DEBUG) CROSS_COMPILE=$(CROSS_CC) -C $(KDIR) M=$(PWD)/modules modules

p1020:
	@echo Making art module for Freescale powepc ref resigns
	make ARCH=$(ARC) PB42=1 P1020=1 DEBUG=$(DEBUG) CROSS_COMPILE=$(CROSS_CC) -C $(KDIR) M=$(PWD)/modules modules

clean:
	cd modules && make clean
