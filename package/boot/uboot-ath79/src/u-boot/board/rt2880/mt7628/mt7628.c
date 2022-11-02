/*
 * Copyright (C) 2021 Jokubas Maciulaitis <jokubas.maciulaitis@teltonika.lt>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <common.h>
#include <command.h>
#include <soc/mtk_soc_common.h>

/*
 * Pre init to run
 * Note: nothing is initialized at this point
 */
void pre_init(void)
{
	mtk_dram_calibrate();
}

/*
 * DRAM init
 */
long int dram_init(void)
{
	return (long int)mtk_dram_size();
}
