/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm-mips/mipsregs.h>
#include <asm-mips/cacheops.h>
#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <rt_mmap.h>

#define _ULCAST_ (unsigned long)

#define MIPS_CONF1_DL_SHIFT	10
#define MIPS_CONF1_DL		(_ULCAST_(7) << 10)
#define MIPS_CONF1_IL_SHIFT	19
#define MIPS_CONF1_IL		(_ULCAST_(7) << 19)

#define read_c0_config1()	__read_32bit_c0_register($16, 1)

#define cache_op(op,addr)						\
	 __asm__ __volatile__(						\
	"       .set    push                                    \n"	\
	"       .set    noreorder                               \n"	\
	"       .set    mips3\n\t                               \n"	\
	"       cache   %0, %1                                  \n"	\
	"       .set    pop                                     \n"	\
	:								\
	: "i" (op), "R" (*(unsigned char *)(addr)))


int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf("Resetting the board...");
	milisecdelay(500);

	full_reset();

	/* After full chip reset we should not reach next step... */
	puts("\n");
	printf_err("RESET FAILED!\n");

	return 0;
}

static inline unsigned long icache_line_size(void)
{
	unsigned long conf1, il;
	conf1 = read_c0_config1();
	il = (conf1 & MIPS_CONF1_IL) >> MIPS_CONF1_IL_SHIFT;
	if (!il)
		return 0;
	return 2 << il;
}

static inline unsigned long dcache_line_size(void)
{
	unsigned long conf1, dl;
	conf1 = read_c0_config1();
	dl = (conf1 & MIPS_CONF1_DL) >> MIPS_CONF1_DL_SHIFT;
	if (!dl)
		return 0;
	return 2 << dl;
}

void flush_cache(ulong start_addr, ulong size)
{
	unsigned long ilsize = icache_line_size();
	unsigned long dlsize = dcache_line_size();
	unsigned long addr, aend;

	/* aend will be miscalculated when size is zero, so we return here */
	if (size == 0)
		return;

	addr = start_addr & ~(dlsize - 1);
	aend = (start_addr + size - 1) & ~(dlsize - 1);

	if (ilsize == dlsize) {
		/* flush I-cache & D-cache simultaneously */
		while (1) {
			cache_op(Hit_Writeback_Inv_D, addr);
			cache_op(Hit_Invalidate_I, addr);
			if (addr == aend)
				break;
			addr += dlsize;
		}
		return;
	}

	/* flush D-cache */
	while (1) {
		cache_op(Hit_Writeback_Inv_D, addr);
		if (addr == aend)
			break;
		addr += dlsize;
	}

	/* flush I-cache */
	addr = start_addr & ~(ilsize - 1);
	aend = (start_addr + size - 1) & ~(ilsize - 1);
	while (1) {
		cache_op(Hit_Invalidate_I, addr);
		if (addr == aend)
			break;
		addr += ilsize;
	}
}

void invalidate_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = dcache_line_size();
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);

	while (1) {
		cache_op(Hit_Invalidate_D, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
}

/*
 * Read CPU type and put its name into buffer
 * For now only 24KEc is supported
 */
void cpu_name(char *name)
{
	u32 cpu_id = read_c0_prid();

	if (name == NULL)
		return;

	switch (cpu_id & PRID_IMP_MASK) {
	case PRID_IMP_24KE:
		sprintf(name, "MIPS 24KEc");
		break;
	default:
		sprintf(name, "MIPS Unknown");
		break;
	}
}
