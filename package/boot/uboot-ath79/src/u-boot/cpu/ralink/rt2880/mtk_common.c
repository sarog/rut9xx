/*
 * MediaTek/Ralink WiSoCs common/helper functions
 *
 * Copyright (C) 2021 Jokubas Maciulaitis <jokubas.maciulaitis@teltonika.lt>
 *
 * Based on:
 * u-boot/cpu/mips/ar7240/qca_common.c
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <config.h>
#include <common.h>
#include <asm/addrspace.h>
#include <soc/mtk_soc_common.h>

/*
 * Returns 1 if reference clock is 40 MHz
 */
u32 mtk_xtal_is_40mhz(void)
{
	return ((RALINK_REG(MTK_SYSCTL_SYSCFG_0) &
		 MTK_SYSCTL_SYSCFG0_XTAL_FREQ_SEL_MASK) >>
		MTK_SYSCTL_SYSCFG0_XTAL_FREQ_SEL_SHIFT);
}

/*
 * Performs full chip reset
 */
void mtk_full_chip_reset(void)
{
	volatile u32 i = 1;

	do {
		RALINK_REG(MTK_SYSCTL_RSTCTL) =
			(1 << MTK_SYSCTL_RSTCTL_SYS_RST_SHIFT);
	} while (i);
}