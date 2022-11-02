/*
 * Copyright (C) 2016 Piotr Dymacz <piotr@dymacz.pl>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <953x.h>

#if defined(CONFIG_FOR_TELTONIKA_RUT9XX) || defined(CONFIG_FOR_TELTONIKA_RUT952)

	#define DDR_TUNE_ADDR  KSEG1 + PMU2_ADDRESS
	#define DDR_TUNE_VALUE PMU2_LDO_TUNE_SET(2)

#endif
