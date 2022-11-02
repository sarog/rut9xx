/*
 * MediaTek/Ralink WiSoCs system clocks related functions
 *
 * Copyright (C) 2021 Jokubas Maciulaitis <jokubas.maciulaitis@teltonika.lt>
 *
 * Based on:
 * u-boot/cpu/mips/ar7240/qca_clocks.c
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <soc/mtk_soc_common.h>

/*
 * Get CPU, RAM, AHB and SPI clocks
 * TODO: confirm nfracdiv values
 */
void mtk_sys_clocks(u32 *cpu_clk, u32 *ddr_clk, u32 *ahb_clk, u32 *spi_clk,
		    u32 *ref_clk)
{
	u32 mtk_cpu_clk, mtk_ddr_clk, mtk_ahb_clk, mtk_spi_clk, mtk_ref_clk;
	u32 reg;

	if (mtk_xtal_is_40mhz()) {
		mtk_ref_clk = VAL_40MHz;
	} else {
		mtk_ref_clk = VAL_25MHz;
	}

	reg = RALINK_REG(MTK_SYSCTL_CLKCFG_0);

	if ((reg & MTK_SYSCTL_CLKCFG0_CPU_FRM_XTAL_MASK) >>
	    MTK_SYSCTL_CLKCFG0_CPU_FRM_XTAL_SHIFT) {
		mtk_cpu_clk = mtk_ref_clk;
	} else {
		mtk_cpu_clk = ((reg & MTK_SYSCTL_CLKCFG0_FRM_BBP_MASK) >>
			       MTK_SYSCTL_CLKCFG0_FRM_BBP_SHIFT) ?
					    MTK_CPU_CLK_PLL_480 :
					    MTK_CPU_CLK_PLL_580;
	}

	mtk_ahb_clk = mtk_cpu_clk / 3;
	mtk_ddr_clk = mtk_ahb_clk * 2;

	if (spi_clk != NULL) {
		reg = (RALINK_REG(MTK_SPICTL_SPI_MASTER) &
		       MTK_SPICTL_SPI_MASTER_RS_CLK_SEL_MASK) >>
		      MTK_SPICTL_SPI_MASTER_RS_CLK_SEL_SHIFT;
		mtk_spi_clk = mtk_ahb_clk / (reg + 2);
	}

	if (cpu_clk != NULL)
		*cpu_clk = mtk_cpu_clk;

	if (ddr_clk != NULL)
		*ddr_clk = mtk_ddr_clk;

	if (ahb_clk != NULL)
		*ahb_clk = mtk_ahb_clk;

	if (spi_clk != NULL)
		*spi_clk = mtk_spi_clk;

	if (ref_clk != NULL)
		*ref_clk = mtk_ref_clk;
}
