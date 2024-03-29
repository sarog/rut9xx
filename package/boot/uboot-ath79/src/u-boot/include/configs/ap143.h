/*
 * Copyright (C) 2016 Piotr Dymacz <piotr@dymacz.pl>
 *
 * This file contains the configuration parameters
 * for Qualcomm Atheros QCA953x based devices
 *
 * Reference designs: AP143
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef _AP143_H
#define _AP143_H

#include <config.h>
#include <configs/qca9k_common.h>
#include <soc/soc_common.h>

/*
 * ==================
 * GPIO configuration
 * ==================
 */
#if defined(CONFIG_FOR_ALFA_NETWORK_R36A)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO4  | GPIO12 | GPIO14 |\
						GPIO15 | GPIO16
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO17
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO3
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_L	GPIO0 | GPIO1

#elif defined(CONFIG_FOR_COMFAST_CF_E314N)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO4  | GPIO11 | GPIO14 |\
						GPIO15 | GPIO16
	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO0 | GPIO2 | GPIO3

#elif defined(CONFIG_FOR_COMFAST_CF_E320N_V2)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO0 | GPIO2 | GPIO3
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO11 | GPIO12 | GPIO14 |\
						GPIO16

#elif defined(CONFIG_FOR_COMFAST_CF_E520N) ||\
      defined(CONFIG_FOR_COMFAST_CF_E530N)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO11

#elif defined(CONFIG_FOR_GAINSTRONG_OOLITE_V5_2_DEV)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO13

#elif defined(CONFIG_FOR_GLINET_GL_AR300M_LITE)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO13 | GPIO14
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO0 | GPIO1 | GPIO16 | GPIO17
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_L	GPIO12

#elif defined(CONFIG_FOR_GLINET_GL_AR750)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO13 | GPIO14
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO0 | GPIO16 | GPIO17
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO2
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_L	GPIO12

#elif defined(CONFIG_FOR_P2W_CPE505N)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO4  | GPIO11 | GPIO12 |\
						GPIO14 | GPIO15

#elif defined(CONFIG_FOR_P2W_R602N)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO4  | GPIO11 | GPIO12 |\
						GPIO14 | GPIO15 | GPIO16

#elif defined(CONFIG_FOR_TPLINK_MR22U_V1)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO13
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO14 | GPIO16
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO11

#elif defined(CONFIG_FOR_TPLINK_MR3420_V3)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO1  | GPIO3  | GPIO4  |\
						GPIO11 | GPIO13 | GPIO14 |\
						GPIO15 | GPIO16
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO17

#elif defined(CONFIG_FOR_TPLINK_MR6400_V1V2)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO0  | GPIO1 | GPIO3 |\
						GPIO11 | GPIO16
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO14
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO4 | GPIO13

#elif defined(CONFIG_FOR_TPLINK_WA850RE_V2)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO0  | GPIO1 | GPIO2  |\
						GPIO3  | GPIO4 | GPIO12 |\
						GPIO13 | GPIO14
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO16
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO15

#elif defined(CONFIG_FOR_TPLINK_WR802N_V1) ||\
      defined(CONFIG_FOR_TPLINK_WR820N_V1_CN)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO13

#elif defined(CONFIG_FOR_TPLINK_WR810N_V1) ||\
      defined(CONFIG_FOR_TPLINK_WR810N_V2)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO13
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO0 | GPIO1

	#if defined(CONFIG_FOR_TPLINK_WR810N_V1)
		#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO11
	#endif

#elif defined(CONFIG_FOR_TPLINK_WR841N_V10) ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V9)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO3  | GPIO4  | GPIO11 |\
						GPIO13 | GPIO14 | GPIO15 |\
						GPIO16
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO17

#elif defined(CONFIG_FOR_TELTONIKA_TRB24XX)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H 	GPIO11

	#define CONFIG_QCA_GPIO_MASK_IN 	GPIO3 | GPIO4 | GPIO12

	#define CONFIG_SR_LED_ALL_OFF_MASK	0x00
	#define CONFIG_SR_LED_ALL_ON_MASK	0xFF

	// LED order is important!
	#define CONFIG_SR_LED_ANIMATION_MASK 	0x01, 0x02, 0x04, \
						0x08, 0x10, 0x20

	/* GPIOs for 74X164 shift register (used as a gpio expander) */
	#define GPIO_SR_74X164_SER	GPIO2	/* Serial input */
	#define GPIO_SR_74X164_SRCLK	GPIO0 	/* Shift register clock */
	#define GPIO_SR_74X164_RCLK	GPIO1 	/* Storage register clock */

	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_L GPIO_SR_74X164_SER |\
						GPIO_SR_74X164_SRCLK |\
						GPIO_SR_74X164_RCLK

	#define CONFIG_GPIO_MASK_DIGITAL_IN 	GPIO3

	#define OFFSET_MNF_INFO 		0x20000
	#define OFFSET_SERIAL_NUMBER 		0x00030
	#define SERIAL_NUMBER_LENGTH 		0x0000A

	#define DEVICE_MODEL 			"TRB2XX" // used for u-boot validation
	#define DEVICE_MODEL_MANIFEST 		"trb2xx" // used for firmware validation
	#define DEVICE_MODEL_NAME		"TRB2"	 // used for mnf info validation

#elif defined(CONFIG_FOR_TELTONIKA_RUT300)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H 	GPIO0 | GPIO1 | GPIO2 |\
						GPIO3 | GPIO4

	#define CONFIG_QCA_GPIO_MASK_IN 	GPIO11 | GPIO12

	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_L GPIO13 | GPIO14

	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H GPIO15

	#define CONFIG_GPIO_MASK_DIGITAL_IN 	GPIO11

	#define OFFSET_MNF_INFO 		0x20000
	#define OFFSET_SERIAL_NUMBER 		0x00030
	#define SERIAL_NUMBER_LENGTH 		0x0000A

	#define DEVICE_MODEL 			"RUT300" // used for u-boot validation
	#define DEVICE_MODEL_MANIFEST 		"rut30x" // used for firmware validation
	#define DEVICE_MODEL_NAME		"RUT30"	 // used for mnf info validation

#elif defined(CONFIG_FOR_TELTONIKA_RUT360)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H 	GPIO4 | GPIO13

	#define CONFIG_QCA_GPIO_MASK_IN 	GPIO3 | GPIO11 | GPIO12

	#define CONFIG_SR_LED_ALL_OFF_MASK	0x00 // skip power led
	#define CONFIG_SR_LED_ALL_ON_MASK	0xFE // skip power led

	// LED order is important!
	#define CONFIG_SR_LED_ANIMATION_MASK 	0x02, 0x04, 0x08, \
						0x10, 0x20

	/* GPIOs for 74X164 shift register (used as a gpio expander) */
	#define GPIO_SR_74X164_SER	GPIO2	/* Serial input */
	#define GPIO_SR_74X164_SRCLK	GPIO0 	/* Shift register clock */
	#define GPIO_SR_74X164_RCLK	GPIO1 	/* Storage register clock */

	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_L GPIO_SR_74X164_SER |\
						GPIO_SR_74X164_SRCLK |\
						GPIO_SR_74X164_RCLK

	#define CONFIG_GPIO_MASK_DIGITAL_IN 	GPIO11

	#define OFFSET_MNF_INFO 		0x20000
	#define OFFSET_SERIAL_NUMBER 		0x00030
	#define SERIAL_NUMBER_LENGTH 		0x0000A

	#define DEVICE_MODEL 			"RUT360" // used for u-boot validation
	#define DEVICE_MODEL_MANIFEST 		"rut36x" // used for firmware validation
	#define DEVICE_MODEL_NAME		"RUT36"	 // used for mnf info validation

#elif defined(CONFIG_FOR_TELTONIKA_TCR1XX)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H 	GPIO4 | GPIO13

	#define CONFIG_QCA_GPIO_MASK_IN 	GPIO3 | GPIO11 | GPIO12

	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H GPIO16

	#define CONFIG_SR_LED_ALL_OFF_MASK	0x00
	#define CONFIG_SR_LED_ALL_ON_MASK	0x3F

	// LED order is important!
	#define CONFIG_SR_LED_ANIMATION_MASK 	0x02, 0x04, 0x08, \
						0x10, 0x20

	/* GPIOs for 74X164 shift register (used as a gpio expander) */
	#define GPIO_SR_74X164_SER	GPIO2	/* Serial input */
	#define GPIO_SR_74X164_SRCLK	GPIO0 	/* Shift register clock */
	#define GPIO_SR_74X164_RCLK	GPIO1 	/* Storage register clock */

	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_L GPIO15 |\
						GPIO_SR_74X164_SER |\
						GPIO_SR_74X164_SRCLK |\
						GPIO_SR_74X164_RCLK

	#define OFFSET_MNF_INFO 		0x20000
	#define OFFSET_SERIAL_NUMBER 		0x00030
	#define SERIAL_NUMBER_LENGTH 		0x0000A

	#define DEVICE_MODEL	     		"TCR1XX" // used for u-boot validation
	#define DEVICE_MODEL_MANIFEST		"tcr1xx" // used for firmware validation
	#define DEVICE_MODEL_NAME		"TCR1"	 // used for mnf info validation

#elif defined(CONFIG_FOR_TELTONIKA_OTD1XX)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H 	GPIO4 | GPIO13

	#define CONFIG_QCA_GPIO_MASK_IN 	GPIO3 | GPIO11

	#define CONFIG_SR_LED_ALL_OFF_MASK	0x00 // skip 1&2 pins
	#define CONFIG_SR_LED_ALL_ON_MASK	0xFC // skip 1&2 pins

	// LED order is important!
	#define CONFIG_SR_LED_ANIMATION_MASK 	0x40, 0x10, 0x04, \
						0x80, 0x20, 0x08

	/* GPIOs for 74X164 shift register (used as a gpio expander) */
	#define GPIO_SR_74X164_SER	GPIO2	/* Serial input */
	#define GPIO_SR_74X164_SRCLK	GPIO0 	/* Shift register clock */
	#define GPIO_SR_74X164_RCLK	GPIO1 	/* Storage register clock */

	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_L GPIO_SR_74X164_SER |\
						GPIO_SR_74X164_SRCLK |\
						GPIO_SR_74X164_RCLK

	#define OFFSET_MNF_INFO 		0x20000
	#define OFFSET_SERIAL_NUMBER 		0x00030
	#define SERIAL_NUMBER_LENGTH 		0x0000A

	#define DEVICE_MODEL 			"OTD1XX" // used for u-boot validation
	#define DEVICE_MODEL_MANIFEST 		"otd1xx" // used for firmware validation
	#define DEVICE_MODEL_NAME		"ODT1"	 // used for mnf info validation

#elif defined(CONFIG_FOR_TPLINK_WR841N_V11)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO1  | GPIO2  | GPIO3  |\
						GPIO4  | GPIO11 | GPIO13 |\
						GPIO14 | GPIO15 | GPIO16
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO17

#elif defined(CONFIG_FOR_TPLINK_WR842N_V3)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO2  | GPIO3  | GPIO4  |\
						GPIO11 | GPIO12 | GPIO13 |\
						GPIO14 | GPIO15 | GPIO16 |\
						GPIO17
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO0

#elif defined(CONFIG_FOR_TPLINK_WR902AC_V1)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_H	GPIO4 | GPIO15
	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO0 | GPIO11 | GPIO12
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO2 | GPIO14 | GPIO17
	#define CONFIG_QCA_GPIO_MASK_OUT_INIT_H	GPIO1 | GPIO13

#elif defined(CONFIG_FOR_WALLYS_DR531)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO11 | GPIO12 | GPIO13 |\
						GPIO14 | GPIO15 | GPIO16

#elif defined(CONFIG_FOR_WHQX_E600G_V2)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO4 | GPIO13 | GPIO15 | GPIO16

#elif defined(CONFIG_FOR_WHQX_E600GAC_V2)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO4  | GPIO11 | GPIO12 |\
						GPIO13 | GPIO14 | GPIO15 |\
						GPIO16
	#define CONFIG_QCA_GPIO_MASK_IN		GPIO1

#elif defined(CONFIG_FOR_YUNCORE_AP90Q)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO4 | GPIO12 | GPIO16

#elif defined(CONFIG_FOR_YUNCORE_CPE830)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO0 | GPIO1 | GPIO2  |\
						GPIO3 | GPIO4 | GPIO12 |\
						GPIO16

#elif defined(CONFIG_FOR_YUNCORE_T830)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO4  | GPIO11 | GPIO12 |\
						GPIO13 | GPIO14 | GPIO15 |\
						GPIO16

#elif defined(CONFIG_FOR_ZBTLINK_ZBT_WE1526)

	#define CONFIG_QCA_GPIO_MASK_LED_ACT_L	GPIO4  | GPIO11 | GPIO12 |\
						GPIO13 | GPIO14 | GPIO15 |\
						GPIO16

#endif

/*
 * ================
 * Default bootargs
 * ================
 */
#if defined(CONFIG_FOR_ALFA_NETWORK_R36A)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:03 "\
				"rootfstype=jffs2,squashfs init=/sbin/init "\
				"mtdparts=ath-nor0:448k(u-boot),64k(art),1280k(kernel),14528k(rootfs),64k(config)"

#elif defined(CONFIG_FOR_COMFAST_CF_E314N) ||\
      defined(CONFIG_FOR_COMFAST_CF_E320N_V2)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:03 "\
				"rootfstype=jffs2 init=/sbin/init "\
				"mtdparts=ath-nor0:64k(u-boot),64k(art),1536k(uImage),14656k(rootfs),64k(mib0)"

#elif defined(CONFIG_FOR_COMFAST_CF_E520N) ||\
      defined(CONFIG_FOR_COMFAST_CF_E530N)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:03 "\
				"rootfstype=jffs2 init=/sbin/init "\
				"mtdparts=ath-nor0:64k(u-boot),64k(art),1536k(uImage),6464k(rootfs),64k(mib0)"

#elif defined(CONFIG_FOR_GAINSTRONG_OOLITE_V5_2) ||\
      defined(CONFIG_FOR_GAINSTRONG_OOLITE_V5_2_DEV)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:01 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=ath-nor0:128k(u-boot),16192k@0x20000(firmware),64k@0xff0000(art)"

#elif defined(CONFIG_FOR_GLINET_GL_AR300M_LITE)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:02 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env),16000k(firmware),64k(art)ro"

#elif defined(CONFIG_FOR_GLINET_GL_AR750)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:03 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=spi0.0:256k(u-boot)ro,64k(u-boot-env),64k(art)ro,16000k(firmware)"

#elif defined(CONFIG_FOR_P2W_CPE505N)    ||\
      defined(CONFIG_FOR_P2W_R602N)      ||\
      defined(CONFIG_FOR_YUNCORE_AP90Q)  ||\
      defined(CONFIG_FOR_YUNCORE_CPE830) ||\
      defined(CONFIG_FOR_ZBTLINK_ZBT_WE1526)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:02 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=spi0.0:256k(u-boot),64k(u-boot-env),14528k(rootfs),1472k(kernel),64k(art),16000k(firmware)"

#elif defined(CONFIG_FOR_TPLINK_MR22U_V1)    ||\
      defined(CONFIG_FOR_TPLINK_MR6400_V1V2) ||\
      defined(CONFIG_FOR_TPLINK_WR810N_V1)   ||\
      defined(CONFIG_FOR_TPLINK_WR810N_V2)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:02 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=ath-nor0:128k(u-boot),1024k(kernel),6912k(rootfs),64k(config),64k(art)"

#elif defined(CONFIG_FOR_TPLINK_MR3420_V3)  ||\
      defined(CONFIG_FOR_TPLINK_WA850RE_V2) ||\
      defined(CONFIG_FOR_TPLINK_WR802N_V1)  ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V10) ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V11) ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V9)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:02 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=ath-nor0:128k(u-boot),1024k(kernel),2816k(rootfs),64k(config),64k(art)"

#elif defined(CONFIG_FOR_TELTONIKA_TRB24XX) ||\
      defined(CONFIG_FOR_TELTONIKA_RUT300)  ||\
      defined(CONFIG_FOR_TELTONIKA_RUT360)  ||\
      defined(CONFIG_FOR_TELTONIKA_OTD1XX)  ||\
      defined(CONFIG_FOR_TELTONIKA_TCR1XX)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200"

#elif defined(CONFIG_FOR_TPLINK_WR820N_V1_CN)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:02 "\
				"rootfstype=squashfs init=/sbin/init "\
				"mtdparts=ath-nor0:32k(u-boot1),32k(u-boot2),3008k(rootfs),896k(uImage),64k(mib0),64k(art)"

#elif defined(CONFIG_FOR_TPLINK_WR842N_V3) ||\
      defined(CONFIG_FOR_TPLINK_WR902AC_V1)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:02 "\
				"rootfstype=jffs2 init=/sbin/init "\
				"mtdparts=ath-nor0:32k(u-boot1),32k(u-boot2),3008k(rootfs),896k(uImage),64k(mib0),64k(ART)"

#elif defined(CONFIG_FOR_WALLYS_DR531)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:02 "\
				"rootfstype=jffs2 init=/sbin/init "\
				"mtdparts=ath-nor0:256k(u-boot),64k(u-boot-env),6336k(rootfs),1408k(uImage),64k(mib0),64k(ART)"

#elif defined(CONFIG_FOR_WHQX_E600G_V2)   ||\
      defined(CONFIG_FOR_WHQX_E600GAC_V2) ||\
      defined(CONFIG_FOR_YUNCORE_T830)

	#define CONFIG_BOOTARGS	"console=ttyS0,115200 root=31:02 "\
				"rootfstype=jffs2 init=/sbin/init "\
				"mtdparts=ath-nor0:256k(u-boot),64k(u-boot-env),14528k(rootfs),1408k(uImage),64k(mib0),64k(ART)"

#endif

/*
 * =============================
 * Load address and boot command
 * =============================
 */
#if defined(CONFIG_FOR_ALFA_NETWORK_R36A)

	#define CFG_LOAD_ADDR	0x9F080000

#elif defined(CONFIG_FOR_COMFAST_CF_E314N)           ||\
      defined(CONFIG_FOR_COMFAST_CF_E320N_V2)        ||\
      defined(CONFIG_FOR_COMFAST_CF_E520N)           ||\
      defined(CONFIG_FOR_COMFAST_CF_E530N)           ||\
      defined(CONFIG_FOR_GAINSTRONG_OOLITE_V5_2)     ||\
      defined(CONFIG_FOR_GAINSTRONG_OOLITE_V5_2_DEV) ||\
      defined(CONFIG_FOR_TPLINK_MR22U_V1)            ||\
      defined(CONFIG_FOR_TPLINK_MR3420_V3)           ||\
      defined(CONFIG_FOR_TPLINK_MR6400_V1V2)         ||\
      defined(CONFIG_FOR_TPLINK_WA850RE_V2)          ||\
      defined(CONFIG_FOR_TPLINK_WR802N_V1)           ||\
      defined(CONFIG_FOR_TPLINK_WR810N_V1)           ||\
      defined(CONFIG_FOR_TPLINK_WR810N_V2)           ||\
      defined(CONFIG_FOR_TPLINK_WR820N_V1_CN)        ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V10)          ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V11)          ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V9)           ||\
      defined(CONFIG_FOR_TPLINK_WR842N_V3)           ||\
      defined(CONFIG_FOR_TPLINK_WR902AC_V1)

	#define CFG_LOAD_ADDR	0x9F020000

#elif defined(CONFIG_FOR_TELTONIKA_TRB24XX) ||\
      defined(CONFIG_FOR_TELTONIKA_RUT300)  ||\
      defined(CONFIG_FOR_TELTONIKA_RUT360)  ||\
      defined(CONFIG_FOR_TELTONIKA_OTD1XX)  ||\
      defined(CONFIG_FOR_TELTONIKA_TCR1XX)

	#define CFG_LOAD_ADDR	0x9F040000

#elif defined(CONFIG_FOR_GLINET_GL_AR300M_LITE) ||\
      defined(CONFIG_FOR_P2W_CPE505N)           ||\
      defined(CONFIG_FOR_P2W_R602N)             ||\
      defined(CONFIG_FOR_WALLYS_DR531)          ||\
      defined(CONFIG_FOR_YUNCORE_AP90Q)         ||\
      defined(CONFIG_FOR_YUNCORE_CPE830)        ||\
      defined(CONFIG_FOR_YUNCORE_T830)          ||\
      defined(CONFIG_FOR_ZBTLINK_ZBT_WE1526)

	#define CFG_LOAD_ADDR	0x9F050000

#elif defined(CONFIG_FOR_GLINET_GL_AR750) ||\
      defined(CONFIG_FOR_WHQX_E600G_V2)   ||\
      defined(CONFIG_FOR_WHQX_E600GAC_V2)

	#define CFG_LOAD_ADDR	0x9F070000

#endif

#if defined(CONFIG_FOR_P2W_CPE505N)    ||\
    defined(CONFIG_FOR_P2W_R602N)      ||\
    defined(CONFIG_FOR_YUNCORE_AP90Q)  ||\
    defined(CONFIG_FOR_YUNCORE_CPE830) ||\
    defined(CONFIG_FOR_YUNCORE_T830)   ||\
    defined(CONFIG_FOR_ZBTLINK_ZBT_WE1526)

	#define CONFIG_BOOTCOMMAND	"bootm 0x9F050000 || bootm 0x9FE80000"

#else

	#define CONFIG_BOOTCOMMAND	"bootm " MK_STR(CFG_LOAD_ADDR)

#endif

/*
 * =========================
 * Environment configuration
 * =========================
 */
#if defined(CONFIG_FOR_ALFA_NETWORK_R36A)

	#define CFG_ENV_ADDR		0x9F060000
	#define CFG_ENV_SIZE		0x10000

#elif defined(CONFIG_FOR_COMFAST_CF_E314N)    ||\
      defined(CONFIG_FOR_COMFAST_CF_E320N_V2) ||\
      defined(CONFIG_FOR_COMFAST_CF_E520N)    ||\
      defined(CONFIG_FOR_COMFAST_CF_E530N)

	#define CFG_ENV_ADDR		0x9F018000
	#define CFG_ENV_SIZE		0x7C00
	#define CFG_ENV_SECT_SIZE	0x10000

#elif defined(CONFIG_FOR_GAINSTRONG_OOLITE_V5_2)     ||\
      defined(CONFIG_FOR_GAINSTRONG_OOLITE_V5_2_DEV) ||\
      defined(CONFIG_FOR_TELTONIKA_TRB24XX)          ||\
      defined(CONFIG_FOR_TELTONIKA_RUT300)           ||\
      defined(CONFIG_FOR_TELTONIKA_RUT360)           ||\
      defined(CONFIG_FOR_TELTONIKA_OTD1XX)           ||\
      defined(CONFIG_FOR_TELTONIKA_TCR1XX)           ||\
      defined(CONFIG_FOR_TPLINK_MR22U_V1)            ||\
      defined(CONFIG_FOR_TPLINK_MR3420_V3)           ||\
      defined(CONFIG_FOR_TPLINK_MR6400_V1V2)         ||\
      defined(CONFIG_FOR_TPLINK_WA850RE_V2)          ||\
      defined(CONFIG_FOR_TPLINK_WR802N_V1)           ||\
      defined(CONFIG_FOR_TPLINK_WR810N_V1)           ||\
      defined(CONFIG_FOR_TPLINK_WR810N_V2)           ||\
      defined(CONFIG_FOR_TPLINK_WR820N_V1_CN)        ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V10)          ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V11)          ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V9)           ||\
      defined(CONFIG_FOR_TPLINK_WR842N_V3)           ||\
      defined(CONFIG_FOR_TPLINK_WR902AC_V1)

	#define CFG_ENV_ADDR		0x9F01EC00
	#define CFG_ENV_SIZE		0x1000
	#define CFG_ENV_SECT_SIZE	0x10000

#elif defined(CONFIG_FOR_GLINET_GL_AR300M_LITE) ||\
      defined(CONFIG_FOR_GLINET_GL_AR750)       ||\
      defined(CONFIG_FOR_WHQX_E600G_V2)         ||\
      defined(CONFIG_FOR_WHQX_E600GAC_V2)

	#define CFG_ENV_ADDR		0x9F040000
	#define CFG_ENV_SIZE		0x10000
	#define CFG_ENV_SECT_SIZE	0x10000

#elif defined(CONFIG_FOR_P2W_CPE505N)    ||\
      defined(CONFIG_FOR_P2W_R602N)      ||\
      defined(CONFIG_FOR_YUNCORE_AP90Q)  ||\
      defined(CONFIG_FOR_YUNCORE_CPE830) ||\
      defined(CONFIG_FOR_YUNCORE_T830)   ||\
      defined(CONFIG_FOR_ZBTLINK_ZBT_WE1526)

	#define CFG_ENV_ADDR		0x9F040000
	#define CFG_ENV_SIZE		0xFC00
	#define CFG_ENV_SECT_SIZE	0x10000

#elif defined(CONFIG_FOR_WALLYS_DR531)

	#define CFG_ENV_ADDR		0x9F030000
	#define CFG_ENV_SIZE		0xF800
	#define CFG_ENV_SECT_SIZE	0x10000

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
#if defined(CONFIG_FOR_ALFA_NETWORK_R36A)

	#define OFFSET_MAC_DATA_BLOCK		0x70000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x10000
	#define OFFSET_MAC_ADDRESS		0x00000

#elif defined(CONFIG_FOR_COMFAST_CF_E314N)    ||\
      defined(CONFIG_FOR_COMFAST_CF_E320N_V2) ||\
      defined(CONFIG_FOR_COMFAST_CF_E520N)    ||\
      defined(CONFIG_FOR_COMFAST_CF_E530N)

	#define OFFSET_MAC_DATA_BLOCK		0x10000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x10000
	#define OFFSET_MAC_ADDRESS		0x00000

#elif defined(CONFIG_FOR_GAINSTRONG_OOLITE_V5_2)     ||\
      defined(CONFIG_FOR_GAINSTRONG_OOLITE_V5_2_DEV) ||\
      defined(CONFIG_FOR_GLINET_GL_AR300M_LITE)      ||\
      defined(CONFIG_FOR_P2W_CPE505N)                ||\
      defined(CONFIG_FOR_P2W_R602N)                  ||\
      defined(CONFIG_FOR_YUNCORE_AP90Q)              ||\
      defined(CONFIG_FOR_YUNCORE_CPE830)             ||\
      defined(CONFIG_FOR_YUNCORE_T830)               ||\
      defined(CONFIG_FOR_ZBTLINK_ZBT_WE1526)

	#define OFFSET_MAC_DATA_BLOCK		0xFF0000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x010000
	#define OFFSET_MAC_ADDRESS		0x000000

#elif defined(CONFIG_FOR_GLINET_GL_AR750)

	#define OFFSET_MAC_DATA_BLOCK		0x50000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x10000
	#define OFFSET_MAC_ADDRESS		0x00000

#elif defined(CONFIG_FOR_TPLINK_MR22U_V1)     ||\
      defined(CONFIG_FOR_TPLINK_MR3420_V3)    ||\
      defined(CONFIG_FOR_TPLINK_MR6400_V1V2)  ||\
      defined(CONFIG_FOR_TPLINK_WR802N_V1)    ||\
      defined(CONFIG_FOR_TPLINK_WR810N_V1)    ||\
      defined(CONFIG_FOR_TPLINK_WR810N_V2)    ||\
      defined(CONFIG_FOR_TPLINK_WR820N_V1_CN) ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V10)   ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V11)   ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V9)    ||\
      defined(CONFIG_FOR_TPLINK_WR842N_V3)

	#define OFFSET_MAC_DATA_BLOCK		0x010000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x010000
	#define OFFSET_MAC_ADDRESS		0x00FC00
	#define OFFSET_ROUTER_MODEL		0x00FD00
	#define OFFSET_PIN_NUMBER		0x00FE00

#elif defined(CONFIG_FOR_TELTONIKA_TRB24XX) ||\
      defined(CONFIG_FOR_TELTONIKA_RUT300)  ||\
      defined(CONFIG_FOR_TELTONIKA_RUT360)  ||\
      defined(CONFIG_FOR_TELTONIKA_OTD1XX)  ||\
      defined(CONFIG_FOR_TELTONIKA_TCR1XX)

	#define OFFSET_MAC_DATA_BLOCK		0x020000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x010000
	#define OFFSET_MAC_ADDRESS		0x000000

#elif defined(CONFIG_FOR_TPLINK_WA850RE_V2)

	#define OFFSET_MAC_DATA_BLOCK		0x3c0000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x010000
	#define OFFSET_MAC_ADDRESS		0x000008

#elif defined(CONFIG_FOR_TPLINK_WR902AC_V1)

	#define OFFSET_MAC_DATA_BLOCK		0x750000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x010000
	#define OFFSET_MAC_ADDRESS		0x000008

#elif defined(CONFIG_FOR_WALLYS_DR531)

	#define OFFSET_MAC_DATA_BLOCK		0x030000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x010000
	#define OFFSET_MAC_ADDRESS		0x00F810

#elif defined(CONFIG_FOR_WHQX_E600G_V2) ||\
      defined(CONFIG_FOR_WHQX_E600GAC_V2)

	#define OFFSET_MAC_DATA_BLOCK		0x50000
	#define OFFSET_MAC_DATA_BLOCK_LENGTH	0x10000
	#define OFFSET_MAC_ADDRESS		0x00400

#endif

/*
 * =========================
 * Custom changes per device
 * =========================
 */

/*
 * Comfast CF-E520N and E320Nv2 are limited to 64 KB only,
 * disable some commands
 */
#if defined(CONFIG_FOR_COMFAST_CF_E314N)    ||\
    defined(CONFIG_FOR_COMFAST_CF_E320N_V2) ||\
    defined(CONFIG_FOR_COMFAST_CF_E520N)    ||\
    defined(CONFIG_FOR_COMFAST_CF_E530N)

	#undef CONFIG_CMD_DHCP
	#undef CONFIG_CMD_LOADB
	#undef CONFIG_CMD_SNTP
	#undef CONFIG_UPG_SCRIPTS_UBOOT

#endif

/*
 * ===========================
 * HTTP recovery configuration
 * ===========================
 */
#define WEBFAILSAFE_UPLOAD_KERNEL_ADDRESS	CFG_LOAD_ADDR

#if defined(CONFIG_FOR_ALFA_NETWORK_R36A)

	#define WEBFAILSAFE_UPLOAD_ART_ADDRESS	(CFG_FLASH_BASE + 0x70000)

#elif defined(CONFIG_FOR_COMFAST_CF_E314N)    ||\
      defined(CONFIG_FOR_COMFAST_CF_E320N_V2) ||\
      defined(CONFIG_FOR_COMFAST_CF_E520N)    ||\
      defined(CONFIG_FOR_COMFAST_CF_E530N)

	#define WEBFAILSAFE_UPLOAD_ART_ADDRESS	(CFG_FLASH_BASE + 0x10000)

#elif defined(CONFIG_FOR_GLINET_GL_AR750)

	#define WEBFAILSAFE_UPLOAD_ART_ADDRESS	(CFG_FLASH_BASE + 0x50000)

#elif defined(CONFIG_FOR_WHQX_E600G_V2) ||\
      defined(CONFIG_FOR_WHQX_E600GAC_V2)

	#define WEBFAILSAFE_UPLOAD_ART_ADDRESS	(CFG_FLASH_BASE + 0x60000)

#elif defined(CONFIG_FOR_TELTONIKA_TRB24XX) ||\
      defined(CONFIG_FOR_TELTONIKA_RUT300)  ||\
      defined(CONFIG_FOR_TELTONIKA_RUT360)  ||\
      defined(CONFIG_FOR_TELTONIKA_OTD1XX)  ||\
      defined(CONFIG_FOR_TELTONIKA_TCR1XX)

	#define WEBFAILSAFE_UPLOAD_ART_ADDRESS	(CFG_FLASH_BASE + 0x30000)

#endif

/* Firmware size limit */
#if defined(CONFIG_FOR_ALFA_NETWORK_R36A)

	#define WEBFAILSAFE_UPLOAD_LIMITED_AREA_IN_BYTES	(512 * 1024)

#elif defined(CONFIG_FOR_COMFAST_CF_E314N)           ||\
      defined(CONFIG_FOR_COMFAST_CF_E320N_V2)        ||\
      defined(CONFIG_FOR_COMFAST_CF_E520N)           ||\
      defined(CONFIG_FOR_COMFAST_CF_E530N)           ||\
      defined(CONFIG_FOR_GAINSTRONG_OOLITE_V5_2)     ||\
      defined(CONFIG_FOR_GAINSTRONG_OOLITE_V5_2_DEV) ||\
      defined(CONFIG_FOR_TPLINK_MR22U_V1)            ||\
      defined(CONFIG_FOR_TPLINK_MR3420_V3)           ||\
      defined(CONFIG_FOR_TPLINK_MR6400_V1V2)         ||\
      defined(CONFIG_FOR_TPLINK_WR802N_V1)           ||\
      defined(CONFIG_FOR_TPLINK_WR810N_V1)           ||\
      defined(CONFIG_FOR_TPLINK_WR810N_V2)           ||\
      defined(CONFIG_FOR_TPLINK_WR820N_V1_CN)        ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V10)          ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V11)          ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V9)           ||\
      defined(CONFIG_FOR_TPLINK_WR842N_V3)

	#define WEBFAILSAFE_UPLOAD_LIMITED_AREA_IN_BYTES	(192 * 1024)

#elif defined(CONFIG_FOR_GLINET_GL_AR300M_LITE) ||\
      defined(CONFIG_FOR_GLINET_GL_AR750)       ||\
      defined(CONFIG_FOR_P2W_CPE505N)           ||\
      defined(CONFIG_FOR_P2W_R602N)             ||\
      defined(CONFIG_FOR_WALLYS_DR531)          ||\
      defined(CONFIG_FOR_YUNCORE_AP90Q)         ||\
      defined(CONFIG_FOR_YUNCORE_CPE830)        ||\
      defined(CONFIG_FOR_YUNCORE_T830)          ||\
      defined(CONFIG_FOR_TELTONIKA_TRB24XX)     ||\
      defined(CONFIG_FOR_TELTONIKA_RUT300)      ||\
      defined(CONFIG_FOR_TELTONIKA_RUT360)      ||\
      defined(CONFIG_FOR_TELTONIKA_OTD1XX)      ||\
      defined(CONFIG_FOR_TELTONIKA_TCR1XX)      ||\
      defined(CONFIG_FOR_ZBTLINK_ZBT_WE1526)

	#define WEBFAILSAFE_UPLOAD_LIMITED_AREA_IN_BYTES	(384 * 1024)

#elif defined(CONFIG_FOR_TPLINK_WA850RE_V2) ||\
      defined(CONFIG_FOR_WHQX_E600G_V2)     ||\
      defined(CONFIG_FOR_WHQX_E600GAC_V2)

	#define WEBFAILSAFE_UPLOAD_LIMITED_AREA_IN_BYTES	(448 * 1024)

#elif defined(CONFIG_FOR_TPLINK_WR902AC_V1)

	#define WEBFAILSAFE_UPLOAD_LIMITED_AREA_IN_BYTES	(832 * 1024)

#endif

/*
 * ========================
 * PLL/Clocks configuration
 * ========================
 */
#if defined(CONFIG_FOR_TPLINK_MR22U_V1)     ||\
    defined(CONFIG_FOR_TPLINK_WA850RE_V2)   ||\
    defined(CONFIG_FOR_TPLINK_WR802N_V1)    ||\
    defined(CONFIG_FOR_TPLINK_WR820N_V1_CN) ||\
    defined(CONFIG_FOR_TPLINK_WR841N_V9)

	#define CONFIG_QCA_PLL	QCA_PLL_PRESET_550_400_200

#elif defined(CONFIG_FOR_TELTONIKA_TRB24XX) ||\
      defined(CONFIG_FOR_TELTONIKA_RUT300)  ||\
      defined(CONFIG_FOR_TELTONIKA_RUT360)  ||\
      defined(CONFIG_FOR_TELTONIKA_OTD1XX)  ||\
      defined(CONFIG_FOR_TELTONIKA_TCR1XX)

	#define CONFIG_QCA_PLL	QCA_PLL_PRESET_650_450_225

#else

	#define CONFIG_QCA_PLL	QCA_PLL_PRESET_650_400_200

#endif


#if defined(CONFIG_FOR_ALFA_NETWORK_R36A)

	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_OFFSET	0x70000
	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_SIZE	0x10000

#elif defined(CONFIG_FOR_GAINSTRONG_OOLITE_V5_2)     ||\
      defined(CONFIG_FOR_GAINSTRONG_OOLITE_V5_2_DEV) ||\
      defined(CONFIG_FOR_COMFAST_CF_E314N)           ||\
      defined(CONFIG_FOR_COMFAST_CF_E320N_V2)        ||\
      defined(CONFIG_FOR_COMFAST_CF_E520N)           ||\
      defined(CONFIG_FOR_COMFAST_CF_E530N)           ||\
      defined(CONFIG_FOR_TELTONIKA_TRB24XX)          ||\
      defined(CONFIG_FOR_TELTONIKA_RUT300)           ||\
      defined(CONFIG_FOR_TELTONIKA_RUT360)           ||\
      defined(CONFIG_FOR_TELTONIKA_OTD1XX)           ||\
      defined(CONFIG_FOR_TELTONIKA_TCR1XX)           ||\
      defined(CONFIG_FOR_TPLINK_MR22U_V1)            ||\
      defined(CONFIG_FOR_TPLINK_MR3420_V3)           ||\
      defined(CONFIG_FOR_TPLINK_MR6400_V1V2)         ||\
      defined(CONFIG_FOR_TPLINK_WA850RE_V2)          ||\
      defined(CONFIG_FOR_TPLINK_WR802N_V1)           ||\
      defined(CONFIG_FOR_TPLINK_WR810N_V1)           ||\
      defined(CONFIG_FOR_TPLINK_WR810N_V2)           ||\
      defined(CONFIG_FOR_TPLINK_WR820N_V1_CN)        ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V10)          ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V11)          ||\
      defined(CONFIG_FOR_TPLINK_WR841N_V9)           ||\
      defined(CONFIG_FOR_TPLINK_WR842N_V3)           ||\
      defined(CONFIG_FOR_TPLINK_WR902AC_V1)

	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_OFFSET	0x10000
	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_SIZE	0x10000

#elif defined(CONFIG_FOR_GLINET_GL_AR300M_LITE)

	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_OFFSET	0xFF0000
	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_SIZE	0x010000

#elif defined(CONFIG_FOR_GLINET_GL_AR750) ||\
      defined(CONFIG_FOR_WHQX_E600G_V2)   ||\
      defined(CONFIG_FOR_WHQX_E600GAC_V2)

	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_OFFSET	0x50000
	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_SIZE	0x10000

#elif defined(CONFIG_FOR_P2W_CPE505N)    ||\
      defined(CONFIG_FOR_P2W_R602N)      ||\
      defined(CONFIG_FOR_YUNCORE_AP90Q)  ||\
      defined(CONFIG_FOR_YUNCORE_CPE830) ||\
      defined(CONFIG_FOR_YUNCORE_T830)   ||\
      defined(CONFIG_FOR_ZBTLINK_ZBT_WE1526)

	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_OFFSET	0x40000
	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_SIZE	0x10000

#elif defined(CONFIG_FOR_WALLYS_DR531)

	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_OFFSET	0x30000
	#define CONFIG_QCA_PLL_IN_FLASH_BLOCK_SIZE	0x10000

#endif

/*
 * ==================================
 * For upgrade scripts in environment
 * ==================================
 */
#if !defined(CONFIG_FOR_ALFA_NETWORK_R36A)     &&\
    !defined(CONFIG_FOR_COMFAST_CF_E314N)      &&\
    !defined(CONFIG_FOR_COMFAST_CF_E320N_V2)   &&\
    !defined(CONFIG_FOR_COMFAST_CF_E520N)      &&\
    !defined(CONFIG_FOR_COMFAST_CF_E530N)      &&\
    !defined(CONFIG_FOR_GLINET_GL_AR300M_LITE) &&\
    !defined(CONFIG_FOR_GLINET_GL_AR750)       &&\
    !defined(CONFIG_FOR_P2W_CPE505N)           &&\
    !defined(CONFIG_FOR_P2W_R602N)             &&\
    !defined(CONFIG_FOR_WALLYS_DR531)          &&\
    !defined(CONFIG_FOR_WHQX_E600G_V2)         &&\
    !defined(CONFIG_FOR_WHQX_E600GAC_V2)       &&\
    !defined(CONFIG_FOR_YUNCORE_AP90Q)         &&\
    !defined(CONFIG_FOR_YUNCORE_CPE830)        &&\
    !defined(CONFIG_FOR_YUNCORE_T830)          &&\
    !defined(CONFIG_FOR_ZBTLINK_ZBT_WE1526)

	#define CONFIG_UPG_UBOOT_SIZE_BACKUP_HEX	0x20000

#endif

#if defined(CONFIG_FOR_P2W_CPE505N)    ||\
    defined(CONFIG_FOR_P2W_R602N)      ||\
    defined(CONFIG_FOR_YUNCORE_AP90Q)  ||\
    defined(CONFIG_FOR_YUNCORE_CPE830) ||\
    defined(CONFIG_FOR_YUNCORE_T830)   ||\
    defined(CONFIG_FOR_ZBTLINK_ZBT_WE1526)

	#define CONFIG_UPG_SCRIPTS_FW_ADDR_HEX	0x9F050000

#endif

/*
 * ===================
 * Other configuration
 * ===================
 */

/* Cache lock for stack */
#define CONFIG_INIT_SRAM_SP_OFFSET	0xbd001800

#endif /* _AP143_H */
