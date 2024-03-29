/*
 * Copyright (C) 2016 Piotr Dymacz <piotr@dymacz.pl>
 *
 * This file contains the configuration parameters
 * for Qualcomm Atheros AR933x based devices
 *
 * Reference designs: AP121
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef _AP121_H
#define _AP121_H

#include <config.h>
#include <configs/qca9k_common.h>
#include <soc/soc_common.h>

/*
 * ==================
 * GPIO configuration
 * ==================
 */
#if defined(CONFIG_FOR_8DEVICES_CARAMBOLA2)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO13 | GPIO14
	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO0

#elif defined(CONFIG_FOR_ALFA_NETWORK_AP121F)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO0
	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO17
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO21
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO26
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_L	GPIO27

#elif defined(CONFIG_FOR_ALFA_NETWORK_HORNET_UB)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO0  | GPIO1 | GPIO13
	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO17 | GPIO27
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO11
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO26 | GPIO28

#elif defined(CONFIG_FOR_ALFA_NETWORK_TUBE2H)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO0  | GPIO1 | GPIO13
	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO17 | GPIO27
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO28
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_L	GPIO18 | GPIO22

#elif defined(CONFIG_FOR_CREATCOMM_D3321)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO0  | GPIO13 | GPIO14 |\
						GPIO15 | GPIO16
	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO17 | GPIO27

#elif defined(CONFIG_FOR_DLINK_DIR505_A1)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO26 | GPIO27

#elif defined(CONFIG_FOR_DRAGINO_MS14) ||\
      defined(CONFIG_FOR_VILLAGE_TELCO_MP2)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO0 | GPIO28
	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO13 | GPIO17

#elif defined(CONFIG_FOR_GAINSTRONG_OOLITE_V1_DEV)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO13 | GPIO15 | GPIO17 |\
						GPIO27

#elif defined(CONFIG_FOR_GLINET_6416)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO0 | GPIO13

#elif defined(CONFIG_FOR_GLINET_GL_AR150)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO0 | GPIO13 | GPIO15
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO1  | GPIO7  | GPIO8 |\
						GPIO14 | GPIO16 | GPIO17
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO6

#elif defined(CONFIG_FOR_GLINET_GL_USB150)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO0
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO13
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_L	GPIO7

#elif defined(CONFIG_FOR_HAK5_LAN_TURTLE)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO13

#elif defined(CONFIG_FOR_HAK5_PACKET_SQUIRREL)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO19 | GPIO22 | GPIO23
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO18 | GPIO20 | GPIO21 |\
						GPIO24

#elif defined(CONFIG_FOR_HAK5_WIFI_PINEAPPLE_NANO)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO18
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO19 | GPIO20
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_L	GPIO23

#elif defined(CONFIG_FOR_TPLINK_MR10U_V1)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO27
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO18

#elif defined(CONFIG_FOR_TPLINK_MR13U_V1)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO27
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO6 | GPIO7
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO18

#elif defined(CONFIG_FOR_TPLINK_MR3020_V1)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO0
	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO17 | GPIO26 | GPIO27
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO18 | GPIO20
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO8

#elif defined(CONFIG_FOR_TPLINK_MR3040_V1V2)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO17 | GPIO26 | GPIO27
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO18

#elif defined(CONFIG_FOR_TPLINK_MR3220_V2)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO0  | GPIO1  | GPIO13 |\
						GPIO14 | GPIO15 | GPIO16 |\
						GPIO26
	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO17 | GPIO27
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO8

#elif defined(CONFIG_FOR_TPLINK_WR703N_V1) ||\
      defined(CONFIG_FOR_TPLINK_WR710N_V1)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO27
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO8

#elif defined(CONFIG_FOR_TPLINK_WR720N_V3)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO27
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO18 | GPIO20
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO8

#elif defined(CONFIG_FOR_TPLINK_WR740N_V4)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO0  | GPIO1  | GPIO13 |\
						GPIO14 | GPIO15 | GPIO16
	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO17 | GPIO27
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO26

#elif defined(CONFIG_FOR_UNWIRED_DEVICES_UNWIRED_ONE)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO27

#elif defined(CONFIG_FOR_TELTONIKA_RUT2XX)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L 	GPIO14 | GPIO17
	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H 	GPIO8  | GPIO24 | GPIO21 |\
						GPIO23 | GPIO7  | GPIO6  |\
						GPIO26 | GPIO27
	#define CONFIG_QCA_GPIO_MASK_IN 	GPIO0  | GPIO16
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_L GPIO12

	// LED order is important!
	#define CONFIG_QCA_LED_ANIMATION_MASK 	GPIO8,  GPIO24, GPIO21, \
						GPIO23, GPIO7,  GPIO6,  \
						GPIO26, GPIO27

	#define CONFIG_GPIO_MASK_DIGITAL_IN 	GPIO16

	#define OFFSET_MNF_INFO 		0x20000
	#define OFFSET_SERIAL_NUMBER 		0x00030
	#define SERIAL_NUMBER_LENGTH 		0x0000A

	#define DEVICE_MODEL	     		"RUT2xx" // used for u-boot validation
	#define DEVICE_MODEL_MANIFEST		"rut2xx" // used for firmware validation
	#define DEVICE_MODEL_NAME		"RUT2"	 // used for mnf info validation

#endif

/*
 * ================
 * Default bootargs
 * ================
 */
#if defined(CONFIG_FOR_8DEVICES_CARAMBOLA2)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:02 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=ar7240-nor0:256k(u-boot),64k(u-boot-env),16000k(firmware),64k(art)"

#elif defined(CONFIG_FOR_ALFA_NETWORK_AP121F)

	#define CONFIG_BOOTARGS	"board=AP121F console=ttyATH0,115200 "\
				"rootfstype=squashfs,jffs2 noinitrd "\
				"mtdparts=spi0.0:192k(u-boot)ro,64k(u-boot-env),64k(art)ro,-(firmware)"

#elif defined(CONFIG_FOR_ALFA_NETWORK_HORNET_UB) ||\
      defined(CONFIG_FOR_ALFA_NETWORK_TUBE2H)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:02 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=ar7240-nor0:256k(u-boot),64k(u-boot-env),6144k(rootfs),1600k(uImage),64k(NVRAM),64k(ART)"

#elif defined(CONFIG_FOR_CREATCOMM_D3321)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:03 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=ar7240-nor0:256k(u-boot),64k(u-boot-env),1216k(uImage),5952k(rootfs),256k(config),384k(customer),64k(ART) mem=32M"

#elif defined(CONFIG_FOR_DLINK_DIR505_A1)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:06 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=ar7240-nor0:64k(u-boot),64k(art),64k(mac),64k(nvram),256k(language),1024k(uImage),6656k(rootfs)"

#elif defined(CONFIG_FOR_DRAGINO_MS14) ||\
      defined(CONFIG_FOR_VILLAGE_TELCO_MP2)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:02 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=ar7240-nor0:192k(u-boot),64k(u-boot-env),16064k(firmware),64k(art)"

#elif defined(CONFIG_FOR_GAINSTRONG_OOLITE_V1_DEV)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:02 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=ar7240-nor0:128k(u-boot),1024k(kernel),2816k(rootfs),64k(config),64k(art)"

#elif defined(CONFIG_FOR_GLINET_6416)              ||\
      defined(CONFIG_FOR_HAK5_LAN_TURTLE)          ||\
      defined(CONFIG_FOR_HAK5_PACKET_SQUIRREL)     ||\
      defined(CONFIG_FOR_HAK5_WIFI_PINEAPPLE_NANO) ||\
      defined(CONFIG_FOR_TPLINK_MR10U_V1)          ||\
      defined(CONFIG_FOR_TPLINK_MR13U_V1)          ||\
      defined(CONFIG_FOR_TPLINK_MR3020_V1)         ||\
      defined(CONFIG_FOR_TPLINK_MR3040_V1V2)       ||\
      defined(CONFIG_FOR_TPLINK_MR3220_V2)         ||\
      defined(CONFIG_FOR_TPLINK_WR703N_V1)         ||\
      defined(CONFIG_FOR_TPLINK_WR720N_V3)         ||\
      defined(CONFIG_FOR_TPLINK_WR740N_V4)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:02 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=ar7240-nor0:128k(u-boot),1024k(kernel),2816k(rootfs),64k(config),64k(art)"

#elif defined(CONFIG_FOR_GLINET_GL_AR150) ||\
      defined(CONFIG_FOR_GLINET_GL_USB150)

	#define CONFIG_BOOTARGS	"console=ttyATH0,115200 board=domino root=31:03 "\
				"rootfstype=squashfs,jffs2 noinitrd "\
				"mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env)ro,1280k(kernel),14656k(rootfs),64k(nvram),64k(art)ro,15936k@0x50000(firmware)"

#elif defined(CONFIG_FOR_TPLINK_WR710N_V1)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:02 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=ar7240-nor0:128k(u-boot),1024k(kernel),6912k(rootfs),64k(config),64k(art)"

#elif defined(CONFIG_FOR_UNWIRED_DEVICES_UNWIRED_ONE)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:02 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=ar7240-nor0:128k(u-boot),64k(u-boot-env),16128k(firmware),64k(art)"

#elif defined(CONFIG_FOR_TELTONIKA_RUT2XX)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200"

#endif

/*
 * =============================
 * Load address and boot command
 * =============================
 */
#if defined(CONFIG_FOR_8DEVICES_CARAMBOLA2)    ||\
    defined(CONFIG_FOR_ALFA_NETWORK_AP121F)    ||\
    defined(CONFIG_FOR_ALFA_NETWORK_HORNET_UB) ||\
    defined(CONFIG_FOR_ALFA_NETWORK_TUBE2H)    ||\
    defined(CONFIG_FOR_CREATCOMM_D3321)        ||\
    defined(CONFIG_FOR_GLINET_GL_AR150)        ||\
    defined(CONFIG_FOR_GLINET_GL_USB150)

	#define CFG_LOAD_ADDR	0x9F050000

#elif defined(CONFIG_FOR_DLINK_DIR505_A1)

	#define CFG_LOAD_ADDR	0x9F080000

#elif defined(CONFIG_FOR_DRAGINO_MS14)      ||\
      defined(CONFIG_FOR_VILLAGE_TELCO_MP2) ||\
      defined(CONFIG_FOR_TELTONIKA_RUT2XX)

	#define CFG_LOAD_ADDR	0x9F040000

#elif defined(CONFIG_FOR_UNWIRED_DEVICES_UNWIRED_ONE)

	#define CFG_LOAD_ADDR	0x9F030000

#else

	#define CFG_LOAD_ADDR	0x9F020000

#endif

#if defined(CONFIG_FOR_ALFA_NETWORK_HORNET_UB) ||\
    defined(CONFIG_FOR_ALFA_NETWORK_TUBE2H)

	#define CONFIG_BOOTCOMMAND	"bootm 0x9F050000 || bootm 0x9F650000 || bootm 0x9FE50000"

#else

	#define CONFIG_BOOTCOMMAND	"bootm " MK_STR(CFG_LOAD_ADDR)

#endif

/*
 * =========================
 * Environment configuration
 * =========================
 */
#if defined(CONFIG_FOR_8DEVICES_CARAMBOLA2)    ||\
    defined(CONFIG_FOR_ALFA_NETWORK_HORNET_UB) ||\
    defined(CONFIG_FOR_ALFA_NETWORK_TUBE2H)    ||\
    defined(CONFIG_FOR_CREATCOMM_D3321)        ||\
    defined(CONFIG_FOR_GLINET_GL_AR150)        ||\
    defined(CONFIG_FOR_GLINET_GL_USB150)

	#define CFG_ENV_ADDR		0x9F040000
	#define CFG_ENV_SIZE		0x8000
	#define CFG_ENV_SECT_SIZE	0x10000

#elif defined(CONFIG_FOR_ALFA_NETWORK_AP121F)

	#define CFG_ENV_ADDR		0x9F030000
	#define CFG_ENV_SIZE		0x10000

#elif defined(CONFIG_FOR_DLINK_DIR505_A1)

	#define CFG_ENV_ADDR		0x9F028000
	#define CFG_ENV_SIZE		0x7C00
	#define CFG_ENV_SECT_SIZE	0x10000

#elif defined(CONFIG_FOR_DRAGINO_MS14) ||\
      defined(CONFIG_FOR_VILLAGE_TELCO_MP2)

	#define CFG_ENV_ADDR		0x9F030000
	#define CFG_ENV_SIZE		0x8000
	#define CFG_ENV_SECT_SIZE	0x10000

#elif defined(CONFIG_FOR_UNWIRED_DEVICES_UNWIRED_ONE)

	#define CFG_ENV_ADDR		0x9F020000
	#define CFG_ENV_SIZE		0x8000
	#define CFG_ENV_SECT_SIZE	0x10000

#else

	#define CFG_ENV_ADDR		0x9F01EC00
	#define CFG_ENV_SIZE		0x1000
	#define CFG_ENV_SECT_SIZE	0x10000

#endif

/*
 * ===========================
 * List of available baudrates
 * ===========================
 */
#define CFG_BAUDRATE_TABLE	\
		{ 600,    1200,   2400,    4800,    9600,    14400,  \
		  19200,  28800,  38400,   56000,   57600,   115200, \
		  128000, 153600, 230400,  250000,  256000,  460800, \
		  576000, 921600, 1000000, 1152000, 1500000, 2000000 }

/*
 * ==================================================
 * MAC address/es, model and WPS pin offsets in FLASH
 * ==================================================
 */
#if defined(CONFIG_FOR_8DEVICES_CARAMBOLA2)    ||\
    defined(CONFIG_FOR_ALFA_NETWORK_HORNET_UB) ||\
    defined(CONFIG_FOR_ALFA_NETWORK_TUBE2H)    ||\
    defined(CONFIG_FOR_CREATCOMM_D3321)        ||\
    defined(CONFIG_FOR_DRAGINO_MS14)           ||\
    defined(CONFIG_FOR_VILLAGE_TELCO_MP2)

	#define OFFSET_MAC_DATA_BLOCK		0xFF0000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x010000
	#define OFFSET_MAC_ADDRESS		0x000000
	#define OFFSET_MAC_ADDRESS2		0x000006

#elif defined(CONFIG_FOR_ALFA_NETWORK_AP121F)

	#define OFFSET_MAC_DATA_BLOCK		0x40000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x10000
	#define OFFSET_MAC_ADDRESS		0x00000

#elif defined(CONFIG_FOR_DLINK_DIR505_A1)

	/*
	 * DIR-505 has two MAC addresses inside dedicated MAC partition
	 * They are stored in plain text...
	 * TODO: read/write MAC stored as plain text
	 * #define OFFSET_MAC_DATA_BLOCK	0x02000
	 * #define OFFSET_MAC_DATA_BLOCK_LENGTH	0x010000
	 * #define OFFSET_MAC_ADDRESS		0x000004
	 * #define OFFSET_MAC_ADDRESS2		0x000016
	 */

#elif defined(CONFIG_FOR_GAINSTRONG_OOLITE_V1_DEV)

	#define OFFSET_MAC_DATA_BLOCK		0x010000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x010000
	#define OFFSET_MAC_ADDRESS		0x00FC00

#elif defined(CONFIG_FOR_GLINET_GL_AR150)  ||\
      defined(CONFIG_FOR_GLINET_GL_USB150) ||\
      defined(CONFIG_FOR_UNWIRED_DEVICES_UNWIRED_ONE)

	#define OFFSET_MAC_DATA_BLOCK		0xFF0000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x010000
	#define OFFSET_MAC_ADDRESS		0x000000

#elif defined(CONFIG_FOR_HAK5_WIFI_PINEAPPLE_NANO)

	#define OFFSET_MAC_DATA_BLOCK		0xFF0000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x010000
	#define OFFSET_MAC_ADDRESS		0x000006

#elif defined(CONFIG_FOR_TELTONIKA_RUT2XX)

	#define OFFSET_MAC_DATA_BLOCK		0x020000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x010000
	#define OFFSET_MAC_ADDRESS		0x000000

#else

	#define OFFSET_MAC_DATA_BLOCK		0x010000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x010000
	#define OFFSET_MAC_ADDRESS		0x00FC00

#endif

#if !defined(CONFIG_FOR_8DEVICES_CARAMBOLA2)         &&\
    !defined(CONFIG_FOR_ALFA_NETWORK_AP121F)         &&\
    !defined(CONFIG_FOR_ALFA_NETWORK_HORNET_UB)      &&\
    !defined(CONFIG_FOR_ALFA_NETWORK_TUBE2H)         &&\
    !defined(CONFIG_FOR_CREATCOMM_D3321)             &&\
    !defined(CONFIG_FOR_DLINK_DIR505_A1)             &&\
    !defined(CONFIG_FOR_DRAGINO_MS14)                &&\
    !defined(CONFIG_FOR_GAINSTRONG_OOLITE_V1_DEV)    &&\
    !defined(CONFIG_FOR_GLINET_6416)                 &&\
    !defined(CONFIG_FOR_GLINET_GL_AR150)             &&\
    !defined(CONFIG_FOR_GLINET_GL_USB150)            &&\
    !defined(CONFIG_FOR_HAK5_LAN_TURTLE)             &&\
    !defined(CONFIG_FOR_HAK5_PACKET_SQUIRREL)        &&\
    !defined(CONFIG_FOR_HAK5_WIFI_PINEAPPLE_NANO)    &&\
    !defined(CONFIG_FOR_UNWIRED_DEVICES_UNWIRED_ONE) &&\
    !defined(CONFIG_FOR_VILLAGE_TELCO_MP2)           &&\
    !defined(CONFIG_FOR_TELTONIKA_RUT2XX)

	#define OFFSET_ROUTER_MODEL	0xFD00

#endif

#if defined(CONFIG_FOR_TPLINK_MR3020_V1) ||\
    defined(CONFIG_FOR_TPLINK_MR3220_V2) ||\
    defined(CONFIG_FOR_TPLINK_WR710N_V1) ||\
    defined(CONFIG_FOR_TPLINK_WR740N_V4)

	#define OFFSET_PIN_NUMBER	0xFE00

#endif

/*
 * =========================
 * Custom changes per device
 * =========================
 */

/* Dragino MS14 uses different IP addresses */
#if defined(CONFIG_FOR_DRAGINO_MS14)

	#undef  CONFIG_IPADDR
	#define CONFIG_IPADDR	192.168.255.1

	#undef  CONFIG_SERVERIP
	#define CONFIG_SERVERIP	192.168.255.2

#endif

/* Dragino MS14 and Unwired One boards use different prompts */
#if defined(CONFIG_FOR_UNWIRED_DEVICES_UNWIRED_ONE)

	#undef  CFG_PROMPT
	#define CFG_PROMPT	"BSB> "

#elif defined(CONFIG_FOR_DRAGINO_MS14) ||\
      defined(CONFIG_FOR_VILLAGE_TELCO_MP2)

	#undef  CFG_PROMPT
	#define CFG_PROMPT	"dr_boot> "

#endif

/* D-Link DIR-505 is limited to 64 KB only and doesn't use env */
#if defined(CONFIG_FOR_DLINK_DIR505_A1)

	#undef CONFIG_CMD_DHCP
	#undef CONFIG_CMD_LOADB

#endif

/*
 * ===========================
 * HTTP recovery configuration
 * ===========================
 */
#define WEBFAILSAFE_UPLOAD_KERNEL_ADDRESS	CFG_LOAD_ADDR

#if defined(CONFIG_FOR_ALFA_NETWORK_AP121F)

	#define WEBFAILSAFE_UPLOAD_ART_ADDRESS	(CFG_FLASH_BASE + 0x40000)

#elif defined(CONFIG_FOR_DLINK_DIR505_A1)

	#define WEBFAILSAFE_UPLOAD_ART_ADDRESS	(CFG_FLASH_BASE + 0x10000)

#elif defined(CONFIG_FOR_TELTONIKA_RUT2XX)

	#define WEBFAILSAFE_UPLOAD_ART_ADDRESS	(CFG_FLASH_BASE + 0x30000)

#endif

/* Firmware size limit */
#if defined(CONFIG_FOR_8DEVICES_CARAMBOLA2) ||\
    defined(CONFIG_FOR_GLINET_GL_AR150)     ||\
    defined(CONFIG_FOR_GLINET_GL_USB150)    ||\
    defined(CONFIG_FOR_TELTONIKA_RUT2XX)

	#define WEBFAILSAFE_UPLOAD_LIMITED_AREA_IN_BYTES	(384 * 1024)

#elif defined(CONFIG_FOR_ALFA_NETWORK_AP121F) ||\
      defined(CONFIG_FOR_DRAGINO_MS14)        ||\
      defined(CONFIG_FOR_VILLAGE_TELCO_MP2)

	#define WEBFAILSAFE_UPLOAD_LIMITED_AREA_IN_BYTES	(320 * 1024)

#elif defined(CONFIG_FOR_ALFA_NETWORK_HORNET_UB) ||\
      defined(CONFIG_FOR_ALFA_NETWORK_TUBE2H)

	#define WEBFAILSAFE_UPLOAD_LIMITED_AREA_IN_BYTES	(448 * 1024)

#elif defined(CONFIG_FOR_CREATCOMM_D3321)

	#define WEBFAILSAFE_UPLOAD_LIMITED_AREA_IN_BYTES	(1856 * 1024)

#elif defined(CONFIG_FOR_DLINK_DIR505_A1)

	#define WEBFAILSAFE_UPLOAD_LIMITED_AREA_IN_BYTES	(512 * 1024)

#elif defined(CONFIG_FOR_UNWIRED_DEVICES_UNWIRED_ONE)

	#define WEBFAILSAFE_UPLOAD_LIMITED_AREA_IN_BYTES	(256 * 1024)

#else

	#define WEBFAILSAFE_UPLOAD_LIMITED_AREA_IN_BYTES	(192 * 1024)

#endif

/*
 * ========================
 * PLL/Clocks configuration
 * ========================
 */
#define CONFIG_QCA_PLL	QCA_PLL_PRESET_400_400_200

#if defined(CONFIG_FOR_8DEVICES_CARAMBOLA2)    ||\
    defined(CONFIG_FOR_ALFA_NETWORK_AP121F)    ||\
    defined(CONFIG_FOR_ALFA_NETWORK_HORNET_UB) ||\
    defined(CONFIG_FOR_ALFA_NETWORK_TUBE2H)    ||\
    defined(CONFIG_FOR_CREATCOMM_D3321)        ||\
    defined(CONFIG_FOR_GLINET_GL_AR150)        ||\
    defined(CONFIG_FOR_GLINET_GL_USB150)

	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_OFFSET	0x40000
	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_SIZE	0x10000

#elif defined(CONFIG_FOR_DLINK_DIR505_A1) ||\
      defined(CONFIG_FOR_UNWIRED_DEVICES_UNWIRED_ONE)

	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_OFFSET	0x20000
	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_SIZE	0x10000

#elif defined(CONFIG_FOR_DRAGINO_MS14) ||\
      defined(CONFIG_FOR_VILLAGE_TELCO_MP2)

	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_OFFSET	0x30000
	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_SIZE	0x10000

#else

	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_OFFSET	0x10000
	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_SIZE	0x10000

#endif

/*
 * ==================================
 * For upgrade scripts in environment
 * ==================================
 */
#if !defined(CONFIG_FOR_8DEVICES_CARAMBOLA2)         &&\
    !defined(CONFIG_FOR_ALFA_NETWORK_AP121F)         &&\
    !defined(CONFIG_FOR_ALFA_NETWORK_HORNET_UB)      &&\
    !defined(CONFIG_FOR_ALFA_NETWORK_TUBE2H)         &&\
    !defined(CONFIG_FOR_CREATCOMM_D3321)             &&\
    !defined(CONFIG_FOR_DLINK_DIR505_A1)             &&\
    !defined(CONFIG_FOR_DRAGINO_MS14)                &&\
    !defined(CONFIG_FOR_GLINET_GL_AR150)             &&\
    !defined(CONFIG_FOR_GLINET_GL_USB150)            &&\
    !defined(CONFIG_FOR_HAK5_WIFI_PINEAPPLE_NANO)    &&\
    !defined(CONFIG_FOR_UNWIRED_DEVICES_UNWIRED_ONE) &&\
    !defined(CONFIG_FOR_VILLAGE_TELCO_MP2)

	#define CONFIG_UPG_SCRIPTS_UBOOT_SIZE_BCKP_HEX	0x20000

#endif

#endif /* _AP121_H */
