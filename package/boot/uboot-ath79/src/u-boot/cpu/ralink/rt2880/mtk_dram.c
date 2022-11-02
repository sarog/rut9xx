/*
 * MediaTek/Ralink WiSoCs DRAM related
 * functions for WiSoC families:
 * - MediaTek MT7628
 *
 * Based on u-boot/cpu/mips/ar7240/qca_dram.c
 *
 * Copyright (C) 2021 Jokubas Maciulaitis <jokubas.maciulaitis@teltonika.lt>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <config.h>
#include <common.h>
#include <asm/addrspace.h>
#include <soc/mtk_soc_common.h>
#include <asm-mips/cacheops.h>

#define CONFIG_SYS_CACHELINE_SIZE 32

/*
 * Returns size (in bytes) of the DRAM memory
 *
 * Note: taken from mt7628 uboot sources
 *
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'.
 */
u32 mtk_dram_size(void)
{
	volatile long *base = (volatile long *)MTK_SDRAM_MIN_SIZE;
	long maxsize	    = MTK_SDRAM_MAX_SIZE;
	volatile long *addr;
	long save[32];
	long cnt;
	long val;
	int size;
	int i = 0;
	ulong our_address;

	asm volatile("move %0, $25" : "=r"(our_address) :);

	for (cnt = (maxsize / sizeof(long)) >> 1; cnt > 0; cnt >>= 1) {
		addr	  = base + cnt; /* pointer arith! */
		save[i++] = *addr;

		*addr = ~cnt;
	}

	addr	= base;
	save[i] = *addr;

	*addr = 0;

	if ((val = *addr) != 0) {
		/* Restore the original data before leaving the function. */
		*addr = save[i];

		for (cnt = 1; cnt < maxsize / sizeof(long); cnt <<= 1) {
			addr  = base + cnt;
			*addr = save[--i];
		}

		return 0;
	}

	for (cnt = 1; cnt < maxsize / sizeof(long); cnt <<= 1) {
		addr = base + cnt; /* pointer arith! */

		val   = *addr;
		*addr = save[--i];

		if (val != ~cnt) {
			size = cnt * sizeof(long);

			/* Restore the original data before leaving the function. */
			for (cnt <<= 1; cnt < maxsize / sizeof(long);
			     cnt <<= 1) {
				addr  = base + cnt;
				*addr = save[--i];
			}

			return size;
		}
	}

	return maxsize;
}

/*
 * Return memory type value from SYCTL_SYSCFG0 register
 */
u32 mtk_dram_type(void)
{
#if defined(CONFIG_BOARD_DRAM_TYPE_SDR)
#error "SDRAM is not supported!"
	return RAM_MEMORY_TYPE_SDR;
#elif defined(CONFIG_BOARD_DRAM_TYPE_DDR1)
	return RAM_MEMORY_TYPE_DDR1;
#elif defined(CONFIG_BOARD_DRAM_TYPE_DDR2)
	return RAM_MEMORY_TYPE_DDR2;
#else
	u32 dram_type;

	dram_type = ((RALINK_REG(MTK_SYSCTL_SYSCFG_0) &
		      MTK_SYSCTL_SYSCFG0_DRAM_TYPE_MASK) >>
		     MTK_SYSCTL_SYSCFG0_DRAM_TYPE_SHIFT);

	if (dram_type)
		dram_type = RAM_MEMORY_TYPE_DDR1;
	else
		dram_type = RAM_MEMORY_TYPE_DDR2;

	return dram_type;
#endif
}

/*
 * Returns DDR width (16 or 32)
 */
u32 mtk_dram_ddr_width(void)
{
#ifndef CONFIG_BOARD_DRAM_DDR_WIDTH
	if ((RALINK_REG(MTK_MEM_SDRAM_CFG_1) & MTK_MEM_SDRAM_CFG1_WIDTH_MASK) >>
	    MTK_MEM_SDRAM_CFG1_WIDTH_SHIFT)
		return 16;

	return 32;
#else
	return CONFIG_BOARD_DRAM_DDR_WIDTH;
#endif
}

/*
 * Returns CAS latency
 */
u32 mtk_dram_cas_lat(void)
{
#ifndef CONFIG_BOARD_DRAM_CAS_LATENCY
	u32 reg;

	reg = (RALINK_REG(MTK_MEM_SDRAM_CFG_0) & MTK_MEM_SDRAM_CFG0_CAS_MASK) >>
	      MTK_MEM_SDRAM_CFG0_CAS_SHIFT;

	return reg;
#else
	return CONFIG_BOARD_DRAM_CAS_LATENCY;
#endif
}

/*
 * Returns tRCD latency
 */
u32 mtk_dram_trcd_lat(void)
{
	u32 reg;

	reg = (RALINK_REG(MTK_MEM_SDRAM_CFG_0) &
	       MTK_MEM_SDRAM_CFG0_TRCD_MASK) >>
	      MTK_MEM_SDRAM_CFG0_TRCD_SHIFT;

	return reg;
}

/*
 * Returns tRP latency
 */
u32 mtk_dram_trp_lat(void)
{
	u32 reg;

	reg = (RALINK_REG(MTK_MEM_SDRAM_CFG_0) & MTK_MEM_SDRAM_CFG0_TRP_MASK) >>
	      MTK_MEM_SDRAM_CFG0_TRP_SHIFT;

	return reg;
}

/*
 * Returns tRAS latency
 */
u32 mtk_dram_tras_lat(void)
{
	u32 reg;

	reg = (RALINK_REG(MTK_MEM_SDRAM_CFG_0) &
	       MTK_MEM_SDRAM_CFG0_TRAS_MASK) >>
	      MTK_MEM_SDRAM_CFG0_TRAS_SHIFT;

	return reg;
}

/*
 * ===============
 * DDR calibration
 * ===============
 */
#define pref_op(hint, addr)                                                    \
	__asm__ __volatile__(                                                  \
		"       .set    push                                    \n"    \
		"       .set    noreorder                               \n"    \
		"       pref   %0, %1                                   \n"    \
		"       .set    pop                                     \n"    \
		:                                                              \
		: "i"(hint), "R"(*(unsigned char *)(addr)))

#define cache_op(op, addr)                                                     \
	__asm__ __volatile__(                                                  \
		"       .set    push                                    \n"    \
		"       .set    noreorder                               \n"    \
		"       .set    mips3\n\t                               \n"    \
		"       cache   %0, %1                                  \n"    \
		"       .set    pop                                     \n"    \
		:                                                              \
		: "i"(op), "R"(*(unsigned char *)(addr)))

static inline void cal_memcpy(void *src, void *dst, unsigned int size)
{
	unsigned char *psrc = (unsigned char *)src;
	unsigned char *pdst = (unsigned char *)dst;

	for (int i = 0; i < size; i++, psrc++, pdst++) {
		*pdst = *psrc;
	}
}

static inline void cal_memset(void *src, unsigned char pat, unsigned int size)
{
	unsigned char *psrc = (unsigned char *)src;

	for (int i = 0; i < size; i++, psrc++) {
		*psrc = pat;
	}
}

static void inline cal_invalidate_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr  = start_addr & ~(lsize - 1);
	unsigned long aend  = (stop - 1) & ~(lsize - 1);

	while (1) {
		cache_op(Hit_Invalidate_D, addr);

		if (addr == aend) {
			break;
		}

		addr += lsize;
	}
}

static void inline cal_patgen(unsigned long *start_addr, unsigned int size,
			      unsigned bias)
{
	for (int i = 0; i < size; i++) {
		start_addr[i] = ((ulong)start_addr + i + bias);
	}
}

#define NUM_OF_CACHELINE 128
#define MIN_START	 6
#define MIN_FINE_START	 0xF
#define MAX_START	 7
#define MAX_FINE_START	 0x0

void dram_cali(void)
{
#if defined(ON_BOARD_64M_DRAM_COMPONENT)
#define DRAM_BUTTOM 0x800000
#endif
#if defined(ON_BOARD_128M_DRAM_COMPONENT)
#define DRAM_BUTTOM 0x1000000
#endif
#if defined(ON_BOARD_256M_DRAM_COMPONENT)
#define DRAM_BUTTOM 0x2000000
#endif
#if defined(ON_BOARD_512M_DRAM_COMPONENT)
#define DRAM_BUTTOM 0x4000000
#endif
#if defined(ON_BOARD_1024M_DRAM_COMPONENT)
#define DRAM_BUTTOM 0x8000000
#endif
#if defined(ON_BOARD_2048M_DRAM_COMPONENT)
#define DRAM_BUTTOM 0x10000000
#endif

	unsigned int *nc_addr =
		(unsigned int *)(0xA0000000 + DRAM_BUTTOM - 0x0400);
	unsigned int *c_addr =
		(unsigned int *)(0x80000000 + DRAM_BUTTOM - 0x0400);
	unsigned int min_coarse_dqs[2];
	unsigned int max_coarse_dqs[2];
	unsigned int min_fine_dqs[2];
	unsigned int max_fine_dqs[2];
	unsigned int coarse_dqs[2];
	unsigned int fine_dqs[2];
	int reg = 0, ddr_cfg2_reg = 0;
	int flag = 0;
	int i	 = 0, k;
	int dqs	 = 0;
	unsigned int min_coarse_dqs_bnd, min_fine_dqs_bnd, coarse_dqs_dll,
		fine_dqs_dll;
	u32 value, test_count = 0;
	u32 fdiv = 0, frac = 0;

	/* set-up cpu frequency divider and fractional */
	value = RALINK_REG(MTK_RBUS_DYN_CFG0);
	fdiv  = (value & MTK_RBUS_DYN_CFG0_CPU_DIV_MASK) >>
	       MTK_RBUS_DYN_CFG0_CPU_DIV_SHIFT;

	if ((CPU_FRAC_DIV < 1) || (CPU_FRAC_DIV > 10)) {
		frac = (value & MTK_RBUS_DYN_CFG0_CPU_FFRAC_MASK) >>
		       MTK_RBUS_DYN_CFG0_CPU_FFRAC_SHIFT;
	} else {
		frac = CPU_FRAC_DIV;
	}

	while (frac < fdiv) {
		value = RALINK_REG(MTK_RBUS_DYN_CFG0);
		fdiv  = (value & MTK_RBUS_DYN_CFG0_CPU_DIV_MASK) >>
		       MTK_RBUS_DYN_CFG0_CPU_DIV_SHIFT;

		fdiv--;

		/* clear out divider */
		value &= ~(MTK_RBUS_DYN_CFG0_CPU_DIV_MASK);

		/* set divider */
		value |= (fdiv << MTK_RBUS_DYN_CFG0_CPU_DIV_SHIFT);

		/* apply new divider values */
		RALINK_REG(MTK_RBUS_DYN_CFG0) = value;
		udelay(500);

		i++;

		value = RALINK_REG(MTK_RBUS_DYN_CFG0);
		fdiv  = (value & MTK_RBUS_DYN_CFG0_CPU_DIV_MASK) >>
		       MTK_RBUS_DYN_CFG0_CPU_DIV_SHIFT;
	}

	/* disable auto ddr self-refresh */
	RALINK_REG(MTK_MEM_DDR_SLF_RFS) &=
		~(MTK_MEM_DDR_SLF_RFS_SR_AUTO_EN_MASK);

	/* get ddr2 cfg2 register which will be used later */
	ddr_cfg2_reg = RALINK_REG(MTK_MEM_DDR_CFG2);

	/* clear out dqs0 and dqs1 gating window */
	RALINK_REG(MTK_MEM_DDR_CFG2) &= ~(MTK_MEM_DDR_CFG2_DQS0_GW_MASK |
					  MTK_MEM_DDR_CFG2_DQS1_GW_MASK);

	/* start calibration */
	min_coarse_dqs[0] = MIN_START;
	min_coarse_dqs[1] = MIN_START;
	min_fine_dqs[0]	  = MIN_FINE_START;
	min_fine_dqs[1]	  = MIN_FINE_START;
	max_coarse_dqs[0] = MAX_START;
	max_coarse_dqs[1] = MAX_START;
	max_fine_dqs[0]	  = MAX_FINE_START;
	max_fine_dqs[1]	  = MAX_FINE_START;
	dqs		  = 0;

	// Add by KP, DQS MIN boundary
	reg = RALINK_REG(MTK_MEM_DLL_DBG);

	coarse_dqs_dll = (reg & 0xF00) >> 8;
	fine_dqs_dll   = (reg & 0xF0) >> 4;

	if (coarse_dqs_dll <= 8) {
		min_coarse_dqs_bnd = 8 - coarse_dqs_dll;
	} else {
		min_coarse_dqs_bnd = 0;
	}

	if (fine_dqs_dll <= 8) {
		min_fine_dqs_bnd = 8 - fine_dqs_dll;
	} else {
		min_fine_dqs_bnd = 0;
	}
	// DQS MIN boundary

DQS_CAL:
	flag = 0;

	for (k = 0; k < 2; k++) {
		unsigned int test_dqs;

		if (k == 0) {
			test_dqs = MAX_START;
		} else {
			test_dqs = MAX_FINE_START;
		}

		flag = 0;

		do {
			flag = 0;

			for (nc_addr = (unsigned int *)0xA0000000;
			     nc_addr <
			     (unsigned int *)(0xA0000000 + DRAM_BUTTOM -
					      NUM_OF_CACHELINE * 32);
			     nc_addr += ((DRAM_BUTTOM >> 6) + 1 * 0x400)) {
				RALINK_REG(MTK_MEM_DDR_DQS_DLY) = 0x00007474;
				wmb();

				c_addr = (unsigned int *)((ulong)nc_addr &
							  0xDFFFFFFF);
				cal_memset(((unsigned char *)c_addr), 0x1F,
					   NUM_OF_CACHELINE * 32);
				cal_patgen((unsigned long *)nc_addr,
					   NUM_OF_CACHELINE * 8, 3);

				if (dqs > 0)
					RALINK_REG(MTK_MEM_DDR_DQS_DLY) =
						0x00000074 |
						(((k == 1) ?
								max_coarse_dqs[dqs] :
								test_dqs)
						 << 12) |
						(((k == 0) ? 0xF : test_dqs)
						 << 8);
				else
					RALINK_REG(MTK_MEM_DDR_DQS_DLY) =
						0x00007400 |
						(((k == 1) ?
								max_coarse_dqs[dqs] :
								test_dqs)
						 << 4) |
						(((k == 0) ? 0xF : test_dqs)
						 << 0);
				wmb();

				cal_invalidate_dcache_range(
					((ulong)c_addr),
					((ulong)c_addr) +
						NUM_OF_CACHELINE * 32);
				wmb();
				for (i = 0; i < NUM_OF_CACHELINE * 8; i++) {
					if (i % 8 == 0)
						pref_op(0, &c_addr[i]);
				}
				for (i = 0; i < NUM_OF_CACHELINE * 8; i++) {
					if (c_addr[i] !=
					    (ulong)nc_addr + i + 3) {
						flag = -1;
						goto MAX_FAILED;
					}
				}
			}
MAX_FAILED:
			if (flag == -1) {
				break;
			} else
				test_dqs++;
		} while (test_dqs <= 0xF);

		if (k == 0) {
			max_coarse_dqs[dqs] = test_dqs;
		} else {
			test_dqs--;

			if (test_dqs == MAX_FINE_START - 1) {
				max_coarse_dqs[dqs]--;
				max_fine_dqs[dqs] = 0xF;
			} else {
				max_fine_dqs[dqs] = test_dqs;
			}
		}
	}

	for (k = 0; k < 2; k++) {
		unsigned int test_dqs;
		if (k == 0)
			test_dqs = MIN_START;
		else
			test_dqs = MIN_FINE_START;
		flag = 0;
		do {
			for (nc_addr = (unsigned int *)0xA0000000;
			     nc_addr <
			     (unsigned int *)(0xA0000000 + DRAM_BUTTOM -
					      NUM_OF_CACHELINE * 32);
			     (nc_addr += (DRAM_BUTTOM >> 6) + 1 * 0x480)) {
				RALINK_REG(MTK_MEM_DDR_DQS_DLY) = 0x00007474;
				wmb();
				c_addr = (unsigned int *)((ulong)nc_addr &
							  0xDFFFFFFF);
				RALINK_REG(MTK_MEM_DDR_DQS_DLY) = 0x00007474;
				wmb();
				cal_memset(((unsigned char *)c_addr), 0x1F,
					   NUM_OF_CACHELINE * 32);

				cal_patgen((unsigned long *)nc_addr,
					   NUM_OF_CACHELINE * 8, 1);

				if (dqs > 0)
					RALINK_REG(MTK_MEM_DDR_DQS_DLY) =
						0x00000074 |
						(((k == 1) ?
								min_coarse_dqs[dqs] :
								test_dqs)
						 << 12) |
						(((k == 0) ? 0x0 : test_dqs)
						 << 8);
				else
					RALINK_REG(MTK_MEM_DDR_DQS_DLY) =
						0x00007400 |
						(((k == 1) ?
								min_coarse_dqs[dqs] :
								test_dqs)
						 << 4) |
						(((k == 0) ? 0x0 : test_dqs)
						 << 0);
				wmb();
				cal_invalidate_dcache_range(
					((ulong)c_addr),
					((ulong)c_addr) +
						NUM_OF_CACHELINE * 32);
				wmb();
				for (i = 0; i < NUM_OF_CACHELINE * 8; i++) {
					if (i % 8 == 0)
						pref_op(0, &c_addr[i]);
				}
				for (i = 0; i < NUM_OF_CACHELINE * 8; i++) {
					if (c_addr[i] != (ulong)nc_addr + i + 1)

					{
						flag = -1;
						goto MIN_FAILED;
					}
				}
			}
MIN_FAILED:
			if (k == 0) {
				if ((flag == -1) ||
				    (test_dqs == min_coarse_dqs_bnd)) {
					break;
				} else
					test_dqs--;

				if (test_dqs < min_coarse_dqs_bnd)
					break;
			} else {
				if (flag == -1) {
					test_dqs++;
					break;
				} else if (test_dqs == min_fine_dqs_bnd) {
					break;
				} else {
					test_dqs--;
				}

				if (test_dqs < min_fine_dqs_bnd)
					break;
			}
		} while (test_dqs >= 0);

		if (k == 0) {
			min_coarse_dqs[dqs] = test_dqs;
		} else {
			if (test_dqs == MIN_FINE_START + 1) {
				min_coarse_dqs[dqs]++;
				min_fine_dqs[dqs] = 0x0;
			} else {
				min_fine_dqs[dqs] = test_dqs;
			}
		}
	}

	if (dqs == 0) {
		dqs = 1;
		goto DQS_CAL;
	}

	for (i = 0; i < 2; i++) {
		unsigned int temp;
		coarse_dqs[i] = (max_coarse_dqs[i] + min_coarse_dqs[i]) >> 1;
		temp = (((max_coarse_dqs[i] + min_coarse_dqs[i]) % 2) * 4) +
		       ((max_fine_dqs[i] + min_fine_dqs[i]) >> 1);
		if (temp >= 0x10) {
			coarse_dqs[i]++;
			fine_dqs[i] = (temp - 0x10) + 0x8;
		} else {
			fine_dqs[i] = temp;
		}
	}
	reg = (coarse_dqs[1] << 12) | (fine_dqs[1] << 8) |
	      (coarse_dqs[0] << 4) | fine_dqs[0];

	RALINK_REG(MTK_MEM_DDR_SLF_RFS) &= ~(0x1 << 4);

	RALINK_REG(MTK_MEM_DDR_DQS_DLY) = reg;
	RALINK_REG(MTK_MEM_DDR_CFG2)	= ddr_cfg2_reg;

	RALINK_REG(MTK_MEM_DDR_SLF_RFS) |= (0x1 << 4);

	test_count++;
}

/*
 * DRAM calibration
 */
void mtk_dram_calibrate(void)
{
	void (*ptr)(void);

	ptr = dram_cali;
	ptr = (void *)((u32)ptr & ~(1 << 29));
	(*ptr)();
}
