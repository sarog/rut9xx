/*
 * Copyright (C) 2021 Jokubas Maciulaitis <jokubas.maciulaitis@teltonika.lt>
 *
 * This file contains the configuration parameters
 * for MediaTek/Ralink RT2880 based devices
 *
 * Based on u-boot/include/configs/ap143.h
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef _RT2880_H
#define _RT2880_H

#include <config.h>
#include <configs/mtk_common.h>
#include <soc/soc_common.h>

#define LEAVE_LEDS_ALONE

/*
 * ==================
 * GPIO configuration
 * ==================
 */
#if defined(CONFIG_FOR_TELTONIKA_RUT2M)

	#define CONFIG_MTK_GPIO_MASK_LED_ACT_L 	(u64)(GPIO36 | GPIO43 | GPIO42 | GPIO45)
	#define CONFIG_MTK_GPIO_MASK_LED_ACT_H 	(u64)(GPIO11 | GPIO44 | GPIO1 |\
						      GPIO39 | GPIO40 | GPIO41)

	// LED order is important!
	#define CONFIG_MTK_LED_ANIMATION_MASK 	GPIO39, GPIO40, GPIO41, \
						GPIO0,  GPIO44, GPIO36, \
						GPIO1,  GPIO45

	#define CONFIG_MTK_GPIO_MASK_IN 	(u64)(GPIO0 | GPIO4 | GPIO37 |\
						      GPIO38)

	#define CONFIG_GPIO_MASK_DIGITAL_IN 	GPIO4

#elif defined(CONFIG_FOR_TELTONIKA_RUT9M)

	#define CONFIG_MTK_GPIO_MASK_LED_ACT_L 	(u64)(GPIO39 | GPIO41 | GPIO42 | GPIO43)

	#define CONFIG_SR_LED_ALL_ON_MASK    0b1111111
	#define CONFIG_SR_LED_ALL_OFF_MASK   0
	#define CONFIG_SR_LED_ANIMATION_MASK 0b1, 0b10, 0b100, 0b1000, 0b10000
	#define GPIO_SR_74X164_SRCLK	     GPIO3
	#define GPIO_SR_74X164_RCLK	     GPIO2
	#define GPIO_SR_74X164_SER	     GPIO1

	#define CONFIG_MTK_GPIO_MASK_IN 	(u64)(GPIO40)

	#define CONFIG_GPIO_MASK_DIGITAL_IN 	GPIO40

#endif

/*
 * =========================
 * Device info configuration
 * =========================
 */
#if defined(CONFIG_FOR_TELTONIKA_RUT2M) || defined(CONFIG_FOR_TELTONIKA_RUT9M)

	#define OFFSET_MNF_INFO 	0x20000
	#define OFFSET_SERIAL_NUMBER 	0x00030
	#define SERIAL_NUMBER_LENGTH 	0x0000A

#endif

#if defined(CONFIG_FOR_TELTONIKA_RUT2M)

	#define DEVICE_MODEL 		"RUT2M" // used for u-boot validation
	#define DEVICE_MODEL_MANIFEST 	"rut2m" // used for firmware validation
	#define DEVICE_MODEL_NAME	"RUT2"	 // used for mnf info validation

#elif defined(CONFIG_FOR_TELTONIKA_RUT9M)

	#define DEVICE_MODEL 		"RUT9M"
	#define DEVICE_MODEL_MANIFEST 	"rut9m"
	#define DEVICE_MODEL_NAME	"RUT9"	 // used for mnf info validation

#endif

/*
 * ================
 * Default bootargs
 * ================
 */
#if defined(CONFIG_FOR_TELTONIKA_RUT2M) || defined(CONFIG_FOR_TELTONIKA_RUT9M)

	#define CONFIG_BOOTARGS		"console=ttyS0,115200"

#endif

/*
 * =============================
 * Load address and boot command
 * =============================
 */
#if defined(CONFIG_FOR_TELTONIKA_RUT2M) || defined(CONFIG_FOR_TELTONIKA_RUT9M)

	#define	CFG_LOAD_ADDR 		0xbc060000

#endif

#if defined(CONFIG_FOR_TELTONIKA_RUT2M) || defined(CONFIG_FOR_TELTONIKA_RUT9M)

	#define CONFIG_BOOTCOMMAND 	"bootm " MK_STR(CFG_LOAD_ADDR)

#endif

/*
 * =========================
 * Environment configuration
 * =========================
 */
#if defined(CONFIG_FOR_TELTONIKA_RUT2M) || defined(CONFIG_FOR_TELTONIKA_RUT9M)

	#define CFG_ENV_ADDR		0xBC01F800
	#define CFG_ENV_SIZE		0x800
	#define CFG_ENV_SECT_SIZE	0x10000
	
	#if defined(CFG_MONITOR_LEN) 
		#undef CFG_MONITOR_LEN	
		#define CFG_MONITOR_LEN		((128 << 10)-CFG_ENV_SECT_SIZE)
	#endif

#endif

/*
 * ===========================
 * List of available baudrates
 * ===========================
 */
#define CFG_BAUDRATE_TABLE	\
		{ 600,    1200,   2400,    4800,    9600,    14400, \
		  19200,  28800,  38400,   56000,   57600,   115200 }

/*
 * ==================================================
 * MAC address/es, model and WPS pin offsets in FLASH
 * ==================================================
 */
#if defined(CONFIG_FOR_TELTONIKA_RUT2M) || defined(CONFIG_FOR_TELTONIKA_RUT9M)

	#define OFFSET_MAC_DATA_BLOCK		0x020000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x010000
	#define OFFSET_MAC_ADDRESS		0x000000

#endif

/*
 * ===========================
 * HTTP recovery configuration
 * ===========================
 */
#define WEBFAILSAFE_UPLOAD_KERNEL_ADDRESS	CFG_LOAD_ADDR

#if defined(CONFIG_FOR_TELTONIKA_RUT2M) || defined(CONFIG_FOR_TELTONIKA_RUT9M)

	#define WEBFAILSAFE_UPLOAD_ART_ADDRESS	(CFG_FLASH_BASE + 0x30000)

#endif

/* Firmware size limit */
#if defined(CONFIG_FOR_TELTONIKA_RUT2M) || defined(CONFIG_FOR_TELTONIKA_RUT9M)

	#define WEBFAILSAFE_UPLOAD_LIMITED_AREA_IN_BYTES	(384 * 1024)

#endif

/* MNFINFO command config */
#if defined(CONFIG_FOR_TELTONIKA_RUT2M) ||\
	defined(CONFIG_FOR_TELTONIKA_RUT9M) ||\
	defined(CONFIG_FOR_TELTONIKA_TAP100)

	#define CONFIG_MNFINFO_LITE

#endif

/*
 * ==================================
 * For upgrade scripts in environment
 * ==================================
 */
#define CONFIG_UPG_UBOOT_SIZE_BACKUP_HEX	0x20000

#endif	/* _RT2880_H */
