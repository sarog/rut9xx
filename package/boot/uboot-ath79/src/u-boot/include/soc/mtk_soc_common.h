/*
 * MediaTek/Ralink Wireless SOC common registers definitions
 *
 * Copyright (C) 2014 Piotr Dymacz <piotr@dymacz.pl>
 * 		 2021 Jokubas Maciulaitis <jokubas.maciulaitis@teltonika.lt>
 *
 * Based on u-boot/include/soc/qca_soc_common.h
 *
 * SPDX-License-Identifier:GPL-2.0
 */

#ifndef _MTK_SOC_COMMON_H_
#define _MTK_SOC_COMMON_H_

#include <soc/soc_common.h>

/*
 * CPU clocks
 */
#define MTK_CPU_CLK_PLL_480 			(480 * 1000 * 1000)
#define MTK_CPU_CLK_PLL_580 			(580 * 1000 * 1000)

/*
 * SDRAM sizes
 */
#define MTK_SDRAM_MIN_SIZE 			(  8 * 1024 * 1024) //   8 MiB
#define MTK_SDRAM_MAX_SIZE 			(256 * 1024 * 1024) // 256 MiB

/*
 * Address map
 */
#define MTK_SYSCTL_BASE 			0xB0000000
#define MTK_MEM_BASE 				MTK_SYSCTL_BASE + 0x000300
#define MTK_RBUS_BASE 				MTK_SYSCTL_BASE + 0x000400
#define MTK_GPIO_BASE 				MTK_SYSCTL_BASE + 0x000600
#define MTK_SPICTL_BASE 			MTK_SYSCTL_BASE + 0x000B00
#define MTK_FRAME_EN_BASE 			MTK_SYSCTL_BASE + 0x100000
#define MTK_SWITCH_BASE				MTK_SYSCTL_BASE + 0x110000

/*
 * Sysctl base egisters
 */
#define MTK_SYSCTL_CHIP_ID_0 			MTK_SYSCTL_BASE + 0x00
#define MTK_SYSCTL_CHIP_ID_1 			MTK_SYSCTL_BASE + 0x04
#define MTK_SYSCTL_EE_CFG 			MTK_SYSCTL_BASE + 0x08
#define MTK_SYSCTL_CHIP_REV_ID			MTK_SYSCTL_BASE + 0x0C
#define MTK_SYSCTL_SYSCFG_0			MTK_SYSCTL_BASE + 0x10
#define MTK_SYSCTL_CLKCFG_0			MTK_SYSCTL_BASE + 0x2C
#define MTK_SYSCTL_RSTCTL 			MTK_SYSCTL_BASE + 0x34
#define MTK_SYSCTL_RSTSTAT			MTK_SYSCTL_BASE + 0x38
#define MTK_SYSCTL_AGPIO_CFG 			MTK_SYSCTL_BASE + 0x3C
#define MTK_SYSCTL_GPIO1_MODE 			MTK_SYSCTL_BASE + 0x60
#define MTK_SYSCTL_GPIO2_MODE 			MTK_SYSCTL_BASE + 0x64

/* SYSCTL_CHIP_REV_ID */
#define MTK_SYSCTL_CHIP_REV_ID_ECO_SHIFT	0
#define MTK_SYSCTL_CHIP_REV_ID_ECO_MASK		BITS(MTK_SYSCTL_CHIP_REV_ID_ECO_SHIFT, 4)
#define MTK_SYSCTL_CHIP_REV_ID_VER_SHIFT	8
#define MTK_SYSCTL_CHIP_REV_ID_VER_MASK		BITS(MTK_SYSCTL_CHIP_REV_ID_VER_SHIFT, 4)
#define MTK_SYSCTL_CHIP_REV_ID_PKG_SHIFT	16
#define MTK_SYSCTL_CHIP_REV_ID_PKG_MASK		BIT(MTK_SYSCTL_CHIP_REV_ID_PKG_SHIFT)

/* SYSCTL_SYSCFG0 */
#define MTK_SYSCTL_SYSCFG0_DRAM_TYPE_SHIFT	0
#define MTK_SYSCTL_SYSCFG0_DRAM_TYPE_MASK	BIT(MTK_SYSCTL_SYSCFG0_DRAM_TYPE_SHIFT)
#define MTK_SYSCTL_SYSCFG0_XTAL_FREQ_SEL_SHIFT  6
#define MTK_SYSCTL_SYSCFG0_XTAL_FREQ_SEL_MASK   BIT(MTK_SYSCTL_SYSCFG0_XTAL_FREQ_SEL_SHIFT)

/* SYSCTL_CLKCFG0 */
#define MTK_SYSCTL_CLKCFG0_CPU_FRM_XTAL_SHIFT   0
#define MTK_SYSCTL_CLKCFG0_CPU_FRM_XTAL_MASK 	BIT(MTK_SYSCTL_CLKCFG0_CPU_FRM_XTAL_SHIFT)
#define MTK_SYSCTL_CLKCFG0_FRM_BBP_SHIFT 	1
#define MTK_SYSCTL_CLKCFG0_FRM_BBP_MASK 	BIT(MTK_SYSCTL_CLKCFG0_FRM_BBP_SHIFT)

/* SYSCTL_RSTCTL */
#define MTK_SYSCTL_RSTCTL_SYS_RST_SHIFT 	0
#define MTK_SYSCTL_RSTCTL_SYS_RST_MASK 		BIT(MTK_SYSCTL_RSTCTL_SYS_RST_SHIFT)

/* SYSCTL_RSTSTAT */
#define MTK_SYSCTL_RSTSTAT_WDRST_SHIFT 		1
#define MTK_SYSCTL_RSTSTAT_WDRST_MASK 		BIT(MTK_SYSCTL_RSTSTAT_WDRST_SHIFT)
#define MTK_SYSCTL_RSTSTAT_UART0_SHIFT 		12
#define MTK_SYSCTL_RSTSTAT_UART0_MASK 		BIT(MTK_SYSCTL_RSTSTAT_UART0_SHIFT)
#define MTK_SYSCTL_RSTSTAT_UART1_SHIFT 		19
#define MTK_SYSCTL_RSTSTAT_UART1_MASK 		BIT(MTK_SYSCTL_RSTSTAT_UART1_SHIFT)

/* SYCTL_AGPIO_CFG */
#define MTK_SYSCTL_AGPIO_CFG_P0_DIS_SHIFT 	16
#define MTK_SYSCTL_AGPIO_CFG_P0_DIS_MASK 	BIT(MTK_SYSCTL_AGPIO_CFG_P0_DIS_SHIFT)
#define MTK_SYSCTL_AGPIO_CFG_GPIO_AIO_EN_SHIFT 	17
#define MTK_SYSCTL_AGPIO_CFG_GPIO_AIO_EN_MASK 	BITS(MTK_SYSCTL_AGPIO_CFG_GPIO_AIO_EN_SHIFT, 4)

/* SYSCTL_GPIO1_MODE */
#define MTK_SYSCTL_GPIO1_MODE_GPIO_SHIFT 	0
#define MTK_SYSCTL_GPIO1_MODE_GPIO_MASK 	BITS(MTK_SYSCTL_GPIO1_MODE_GPIO_SHIFT, 2)
#define MTK_SYSCTL_GPIO1_MODE_SPIS_SHIFT 	2
#define MTK_SYSCTL_GPIO1_MODE_SPIS_MASK 	BITS(MTK_SYSCTL_GPIO1_MODE_SPIS_SHIFT, 2)
#define MTK_SYSCTL_GPIO1_MODE_SPI_CS1_SHIFT 	4
#define MTK_SYSCTL_GPIO1_MODE_SPI_CS1_MASK 	BITS(MTK_SYSCTL_GPIO1_MODE_SPI_CS1_SHIFT, 2)
#define MTK_SYSCTL_GPIO1_MODE_I2S_SHIFT 	6
#define MTK_SYSCTL_GPIO1_MODE_I2S_MASK 		BITS(MTK_SYSCTL_GPIO1_MODE_I2S_SHIFT, 2)
#define MTK_SYSCTL_GPIO1_MODE_UART0_SHIFT 	8
#define MTK_SYSCTL_GPIO1_MODE_UART0_MASK 	BITS(MTK_SYSCTL_GPIO1_MODE_UART0_SHIFT, 2)
#define MTK_SYSCTL_GPIO1_MODE_SD_SHIFT 		10
#define MTK_SYSCTL_GPIO1_MODE_SD_MASK 		BITS(MTK_SYSCTL_GPIO1_MODE_SD_SHIFT, 2)
#define MTK_SYSCTL_GPIO1_MODE_SPI_SHIFT 	12
#define MTK_SYSCTL_GPIO1_MODE_SPI_MASK 		BIT(MTK_SYSCTL_GPIO1_MODE_SPI_SHIFT)
#define MTK_SYSCTL_GPIO1_MODE_WDT_SHIFT 	14
#define MTK_SYSCTL_GPIO1_MODE_WDT_MASK 		BIT(MTK_SYSCTL_GPIO1_MODE_WDT_SHIFT)
#define MTK_SYSCTL_GPIO1_MODE_PERST_SHIFT 	16
#define MTK_SYSCTL_GPIO1_MODE_PERST_MASK 	BIT(MTK_SYSCTL_GPIO1_MODE_PERST_SHIFT)
#define MTK_SYSCTL_GPIO1_MODE_REFCLK_SHIFT 	18
#define MTK_SYSCTL_GPIO1_MODE_REFCLK_MASK 	BIT(MTK_SYSCTL_GPIO1_MODE_REFCLK_SHIFT)
#define MTK_SYSCTL_GPIO1_MODE_I2C_SHIFT 	20
#define MTK_SYSCTL_GPIO1_MODE_I2C_MASK 		BITS(MTK_SYSCTL_GPIO1_MODE_I2C_SHIFT, 2)
#define MTK_SYSCTL_GPIO1_MODE_UART1_SHIFT 	24
#define MTK_SYSCTL_GPIO1_MODE_UART1_MASK 	BITS(MTK_SYSCTL_GPIO1_MODE_UART1_SHIFT, 2)
#define MTK_SYSCTL_GPIO1_MODE_UART2_SHIFT 	26
#define MTK_SYSCTL_GPIO1_MODE_UART2_MASK 	BITS(MTK_SYSCTL_GPIO1_MODE_UART2_SHIFT, 2)
#define MTK_SYSCTL_GPIO1_MODE_PWM0_SHIFT 	28
#define MTK_SYSCTL_GPIO1_MODE_PWM0_MASK 	BITS(MTK_SYSCTL_GPIO1_MODE_PWM0_SHIFT, 2)
#define MTK_SYSCTL_GPIO1_MODE_PWM1_SHIFT 	30
#define MTK_SYSCTL_GPIO1_MODE_PWM1_MASK 	BITS(MTK_SYSCTL_GPIO1_MODE_PWM1_SHIFT, 2)

/* SYSCTL_GPIO2_MODE */
#define MTK_SYSCTL_GPIO2_MODE_WLED_AN_SHIFT 	0
#define MTK_SYSCTL_GPIO2_MODE_WLED_AN_MASK 	BITS(MTK_SYSCTL_GPIO2_MODE_WLED_AN_SHIFT, 2)
#define MTK_SYSCTL_GPIO2_MODE_P0_LED_AN_SHIFT 	2
#define MTK_SYSCTL_GPIO2_MODE_P0_LED_AN_MASK 	BITS(MTK_SYSCTL_GPIO2_MODE_P0_LED_AN_SHIFT, 2)
#define MTK_SYSCTL_GPIO2_MODE_P1_LED_AN_SHIFT 	4
#define MTK_SYSCTL_GPIO2_MODE_P1_LED_AN_MASK 	BITS(MTK_SYSCTL_GPIO2_MODE_P1_LED_AN_SHIFT, 2)
#define MTK_SYSCTL_GPIO2_MODE_P2_LED_AN_SHIFT 	6
#define MTK_SYSCTL_GPIO2_MODE_P2_LED_AN_MASK 	BITS(MTK_SYSCTL_GPIO2_MODE_P2_LED_AN_SHIFT, 2)
#define MTK_SYSCTL_GPIO2_MODE_P3_LED_AN_SHIFT 	8
#define MTK_SYSCTL_GPIO2_MODE_P3_LED_AN_MASK 	BITS(MTK_SYSCTL_GPIO2_MODE_P3_LED_AN_SHIFT, 2)
#define MTK_SYSCTL_GPIO2_MODE_P4_LED_AN_SHIFT 	10
#define MTK_SYSCTL_GPIO2_MODE_P4_LED_AN_MASK 	BITS(MTK_SYSCTL_GPIO2_MODE_P4_LED_AN_SHIFT, 2)
#define MTK_SYSCTL_GPIO2_MODE_WLED_KN_SHIFT 	16
#define MTK_SYSCTL_GPIO2_MODE_WLED_KN_MASK 	BITS(MTK_SYSCTL_GPIO2_MODE_WLED_KN_SHIFT, 2)
#define MTK_SYSCTL_GPIO2_MODE_P0_LED_KN_SHIFT 	18
#define MTK_SYSCTL_GPIO2_MODE_P0_LED_KN_MASK 	BITS(MTK_SYSCTL_GPIO2_MODE_P0_LED_KN_SHIFT, 2)
#define MTK_SYSCTL_GPIO2_MODE_P1_LED_KN_SHIFT 	20
#define MTK_SYSCTL_GPIO2_MODE_P1_LED_KN_MASK 	BITS(MTK_SYSCTL_GPIO2_MODE_P1_LED_KN_SHIFT, 2)
#define MTK_SYSCTL_GPIO2_MODE_P2_LED_KN_SHIFT 	22
#define MTK_SYSCTL_GPIO2_MODE_P2_LED_KN_MASK 	BITS(MTK_SYSCTL_GPIO2_MODE_P2_LED_KN_SHIFT, 2)
#define MTK_SYSCTL_GPIO2_MODE_P3_LED_KN_SHIFT 	24
#define MTK_SYSCTL_GPIO2_MODE_P3_LED_KN_MASK 	BITS(MTK_SYSCTL_GPIO2_MODE_P3_LED_KN_SHIFT, 2)
#define MTK_SYSCTL_GPIO2_MODE_P4_LED_KN_SHIFT 	26
#define MTK_SYSCTL_GPIO2_MODE_P4_LED_KN_MASK 	BITS(MTK_SYSCTL_GPIO2_MODE_P4_LED_KN_SHIFT, 2)

/*
 * MEM base registers
 */
#define MTK_MEM_SDRAM_CFG_0			MTK_MEM_BASE + 0x00
#define MTK_MEM_SDRAM_CFG_1			MTK_MEM_BASE + 0x04
#define MTK_MEM_DDR_SLF_RFS 			MTK_MEM_BASE + 0x10
#define MTK_MEM_DLL_DBG 			MTK_MEM_BASE + 0x20
#define MTK_MEM_DDR_CFG2 			MTK_MEM_BASE + 0x48
#define MTK_MEM_DDR_DQS_DLY 			MTK_MEM_BASE + 0x64

/* MEM_SDRAM_CFG0 */
#define MTK_MEM_SDRAM_CFG0_TRP_SHIFT		0
#define MTK_MEM_SDRAM_CFG0_TRP_MASK		BITS(MTK_MEM_SDRAM_CFG0_TRP_SHIFT, 2)
#define MTK_MEM_SDRAM_CFG0_TRCD_SHIFT		8
#define MTK_MEM_SDRAM_CFG0_TRCD_MASK		BITS(MTK_MEM_SDRAM_CFG0_TRCD_SHIFT, 2)
#define MTK_MEM_SDRAM_CFG0_TRAS_SHIFT		12
#define MTK_MEM_SDRAM_CFG0_TRAS_MASK		BITS(MTK_MEM_SDRAM_CFG0_TRAS_SHIFT, 4)
#define MTK_MEM_SDRAM_CFG0_CAS_SHIFT		16
#define MTK_MEM_SDRAM_CFG0_CAS_MASK		BITS(MTK_MEM_SDRAM_CFG0_CAS_SHIFT, 2)

/* MEM_SDRAM_CFG1 */
#define MTK_MEM_SDRAM_CFG1_WIDTH_SHIFT		24
#define MTK_MEM_SDRAM_CFG1_WIDTH_MASK		BIT(MTK_MEM_SDRAM_CFG1_WIDTH_SHIFT)

/* MEM_DDR_SLF_RFS */
#define MTK_MEM_DDR_SLF_RFS_SR_AUTO_EN_SHIFT 	4
#define MTK_MEM_DDR_SLF_RFS_SR_AUTO_EN_MASK 	BIT(MTK_MEM_DDR_SLF_RFS_SR_AUTO_EN_SHIFT)

/* MTK_MEM_DDR_CFG2 */
#define MTK_MEM_DDR_CFG2_DQS1_GW_SHIFT 		26
#define MTK_MEM_DDR_CFG2_DQS1_GW_MASK 		BITS(MTK_MEM_DDR_CFG2_DQS1_GW_SHIFT, 2)
#define MTK_MEM_DDR_CFG2_DQS0_GW_SHIFT 		28
#define MTK_MEM_DDR_CFG2_DQS0_GW_MASK 		BITS(MTK_MEM_DDR_CFG2_DQS0_GW_SHIFT, 2)

/*
 * RBUS Matrix base registers
 */
#define MTK_RBUS_DYN_CFG0 			MTK_RBUS_BASE + 0x40

/* RBUS_DYN_CFG0 */
#define MTK_RBUS_DYN_CFG0_CPU_FFRAC_SHIFT 	0
#define MTK_RBUS_DYN_CFG0_CPU_FFRAC_MASK 	BITS(MTK_RBUS_DYN_CFG0_CPU_FFRAC_SHIFT, 4)
#define MTK_RBUS_DYN_CFG0_CPU_DIV_SHIFT 	8
#define MTK_RBUS_DYN_CFG0_CPU_DIV_MASK 		BITS(MTK_RBUS_DYN_CFG0_CPU_DIV_SHIFT, 4)

/*
 * GPIO base registers
 */
#define MTK_GPIO_CTRL_0 			MTK_GPIO_BASE + 0x00
#define MTK_GPIO_CTRL_1 			MTK_GPIO_BASE + 0x04
#define MTK_GPIO_CTRL_2 			MTK_GPIO_BASE + 0x08
#define MTK_GPIO_DATA_0 			MTK_GPIO_BASE + 0x20
#define MTK_GPIO_DATA_1 			MTK_GPIO_BASE + 0x24
#define MTK_GPIO_DATA_2 			MTK_GPIO_BASE + 0x28

/*
 * SPIctl base registers
 */
#define MTK_SPICTL_SPI_MASTER 			MTK_SPICTL_BASE + 0x28

/* SPICTL_SPI_MASTER */
#define MTK_SPICTL_SPI_MASTER_RS_CLK_SEL_SHIFT 	16
#define MTK_SPICTL_SPI_MASTER_RS_CLK_SEL_MASK 	BITS(MTK_SPICTL_SPI_MASTER_RS_CLK_SEL_SHIFT, 11)

/*
 * Switch base registers
*/
#define MTK_SWITCH_PVIDC0			MTK_SWITCH_BASE + 0x40
#define MTK_SWITCH_PVIDC1			MTK_SWITCH_BASE + 0x44
#define MTK_SWITCH_PVIDC2			MTK_SWITCH_BASE + 0x48
#define MTK_SWITCH_VMSC0			MTK_SWITCH_BASE + 0x70
#define MTK_SWITCH_VMSC1			MTK_SWITCH_BASE + 0x74
#define MTK_SWITCH_POC0				MTK_SWITCH_BASE + 0x90

/* MTK_SWITCH_POC0 */
#define MTK_SWITCH_POC0_DIS_PORT_SHIFT		23
#define MTK_SWITCH_POC0_DIS_PORT_MASK		BITS(MTK_SWITCH_POC0_DIS_PORT_SHIFT, 5)

/*
 * Macros
 */
#define RALINK_REG(x) 				(*((volatile u32 *)(x)))

/*
 * Functions
 */
#ifndef __ASSEMBLY__
u32  mtk_dram_type(void);
void mtk_full_chip_reset(void);
u32  mtk_dram_ddr_width(void);
void mtk_dram_init(void);
u32  mtk_dram_size(void);
u32  mtk_dram_cas_lat(void);
u32  mtk_dram_trcd_lat(void);
u32  mtk_dram_trp_lat(void);
u32  mtk_dram_tras_lat(void);
void mtk_dram_calibrate(void);
void mtk_sys_clocks(u32 *cpu_clk, u32 *ddr_clk, u32 *ahb_clk, u32 *spi_clk, u32 *ref_clk);
u32  mtk_xtal_is_40mhz(void);
#endif /* !__ASSEMBLY__ */

#endif /* _MTK_SOC_COMMON_H_ */
