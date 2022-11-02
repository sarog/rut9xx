/*
 * Copyright (C) 2021 Jokubas Maciulaitis <jokubas.maciulaitis@teltonika.lt>
 *
 * This file contains some of the common configs/defines
 * for boards based on MediaTek/Ralink RT2880 WiSoCs
 *
 * Based on u-boot/include/configs/qca9k_common.h
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef __MTK_COMMON_H
#define __MTK_COMMON_H

/* No debug */
#if defined(DEBUG)
	#undef DEBUG
#endif

/* Wait only 1 second by default */
#if !defined(CONFIG_BOOTDELAY)
	#define CONFIG_BOOTDELAY	1
#endif

/* Allow to disable console output, don't display console info */
#define CONFIG_SILENT_CONSOLE	1
#define CFG_CONSOLE_INFO_QUIET	1

/* Use 115200 as default baudrate */
#if !defined(CONFIG_BAUDRATE)
	#define CONFIG_BAUDRATE	115200
#endif

/* CLI parser, prompt */
//#define CFG_HUSH_PARSER		1
//#define CFG_LONGHELP		1
#define CFG_PROMPT		"u-boot> "
#define CFG_PROMPT_HUSH_PS2	"> "

#define CFG_CBSIZE		1024
#define CFG_PBSIZE		(CFG_CBSIZE + sizeof(CFG_PROMPT) + 16)
#define CFG_MAXARGS		16
#define CFG_MALLOC_LEN		(512 * 1024)
#define CFG_BOOTPARAMS_LEN	(512 * 1024)

/* RAM memory start address */
#define CFG_SDRAM_BASE		0x80000000

/* Ethernet related */
#define CFG_RX_ETH_BUFFER	60

/* Memory test */
#define CFG_MEMTEST_START	(CFG_SDRAM_BASE + (2 * 1024 * 1024))
#define CFG_MEMTEST_END		(CFG_SDRAM_BASE + bd->bi_memsize \
				 - (4 * 1024 * 1024) - 1)

/* FLASH */
#define CFG_MAX_FLASH_BANKS	1
#define CFG_MAX_FLASH_SECT	263
#define CFG_FLASH_BASE		0xBC000000

/* Boot related */
#define CFG_BOOTM_LEN		(16 << 20)

/* The following #defines are needed to get flash environment right */
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MONITOR_LEN		(192 << 10)

/* Default device and server IPs, net console */
#define CONFIG_IPADDR		192.168.1.1
#define CONFIG_SERVERIP		192.168.1.2

/* Other environment variables */
#define CONFIG_BOOTFILE	firmware.bin
#define CONFIG_LOADADDR	0x80800000
#define CONFIG_AUTOLOAD	no
#define CONFIG_BOOTP_RANDOM_DELAY	1

/* This is needed for time calculation */
#define CFG_HZ	(bd->bi_cfg_hz)

#define CFG_INIT_SP_OFFSET	0x400000
#define CFG_CACHELINE_SIZE	16
#define CFG_DCACHE_SIZE		(16 * 1024)
#define CFG_ICACHE_SIZE		(16 * 1024)

/* Environment related */
#define CFG_ENV_IS_IN_FLASH	1
#undef  CFG_ENV_IS_NOWHERE

/* Available commands */
#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_BUTTON
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_ENV
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_FLASH_PROBE
#define CONFIG_CMD_HTTPD
#define CONFIG_CMD_LED
#define CONFIG_CMD_MAC
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_RSTBYWDT
#define CONFIG_CMD_RUN
#define CONFIG_CMD_SLEEP
#define CONFIG_CMD_ITEST
#define CONFIG_CMD_SETEXPR

/* Recovery script with button */
#define CONFIG_BTN_RECOVERY_SCRIPT

/* For HTTP based recovery */
/* TODO: move it to httpd related header */
#define WEBFAILSAFE_PROGRESS_START		0
#define WEBFAILSAFE_PROGRESS_TIMEOUT		1
#define WEBFAILSAFE_PROGRESS_UPLOAD_READY	2
#define WEBFAILSAFE_PROGRESS_UPGRADE_READY	3
#define WEBFAILSAFE_PROGRESS_UPGRADE_FAILED	4

#define WEBFAILSAFE_UPGRADE_TYPE_FIRMWARE	0
#define WEBFAILSAFE_UPGRADE_TYPE_UBOOT		1
#define WEBFAILSAFE_UPGRADE_TYPE_ART		2

#define WEBFAILSAFE_UPLOAD_ART_SIZE_IN_BYTES	(64 * 1024)

#define HTTPD_TIMEOUT_MSEC 	 7000
#define HTTPD_TIMEOUT_DELAY_MSEC 10

/* Some helper macros */
/* TODO: move them out from here */
#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

#endif // __MTK_COMMON_H
