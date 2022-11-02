/*
 * Copyright (C) 2016 Piotr Dymacz <piotr@dymacz.pl>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <config.h>
#include <common.h>
#include <asm/addrspace.h>
#include <soc/qca_soc_common.h>
#include <soc/qca_dram.h>
#include <configs/db12x_dram.h>

/*
 * DRAM init
 */
long int dram_init()
{
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	qca_dram_init();
#endif

	return (long int)qca_dram_size();
}

void qca_dram_post_clk_init()
{
#ifdef DDR_TUNE
	qca_soc_reg_write(DDR_TUNE_ADDR, DDR_TUNE_VALUE);
#endif
}
