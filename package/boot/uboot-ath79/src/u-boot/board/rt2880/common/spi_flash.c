#include <common.h>
#include <command.h>
#include <rt_mmap.h>
#include <configs/rt2880.h>
#include <malloc.h>
#include "bbu_spiflash.h"

/******************************************************************************
 * SPI FLASH elementray definition and function
 ******************************************************************************/

#define FLASH_PAGESIZE 256

/* Flash opcodes. */
#define OPCODE_WREN 6 /* Write enable */
#define OPCODE_WRDI 4 /* Write disable */
#define OPCODE_RDSR 5 /* Read status register */
#define OPCODE_WRSR 1 /* Write status register */

/* Status Register bits. */
#define SR_WIP	1 /* Write in progress */
#define SR_WEL	2 /* Write enable latch */
#define SR_BP0	4 /* Block protect 0 */
#define SR_BP1	8 /* Block protect 1 */
#define SR_BP2	0x10 /* Block protect 2 */
#define SR_EPE	0x20 /* Erase/Program error */
#define SR_SRWD 0x80 /* SR write protect */

#define SPIC_READ_BYTES	 (1 << 0)
#define SPIC_WRITE_BYTES (1 << 1)

#define ra_inb(offset) (*(volatile unsigned char *)(offset))
#define ra_inw(offset) (*(volatile unsigned short *)(offset))
#define ra_inl(offset) (*(volatile unsigned long *)(offset))

#define ra_outb(offset, val) (*(volatile unsigned char *)(offset) = val)
#define ra_outw(offset, val) (*(volatile unsigned short *)(offset) = val)
#define ra_outl(offset, val) (*(volatile unsigned long *)(offset) = val)

#define ra_and(addr, value) ra_outl(addr, (ra_inl(addr) & (value)))
#define ra_or(addr, value)  ra_outl(addr, (ra_inl(addr) | (value)))

static int raspi_wait_ready(int sleep_ms);
int raspi_write(char *buf, unsigned int to, int len);

static int bbu_spic_busy_wait(void)
{
	int n = 100000;
	do {
		if ((ra_inl(SPI_REG_CTL) & SPI_CTL_BUSY) == 0)
			return 0;
		udelay(1);
	} while (--n > 0);

	printf("%s: fail \n", __func__);
	return -1;
}

static int bbu_mb_spic_trans(const u8 code, const u32 addr, u8 *buf,
			     const size_t n_tx, const size_t n_rx, int flag)
{
	u32 reg;
	int i, q, r;
	int rc = -1;

	if (flag != SPIC_READ_BYTES && flag != SPIC_WRITE_BYTES) {
		printf("we currently support more-byte-mode for reading and writing data only\n");
		return -1;
	}

	/* step 0. enable more byte mode */
	ra_or(SPI_REG_MASTER, (1 << 2));

	bbu_spic_busy_wait();

	/* step 1. set opcode & address, and fix cmd bit count to 32 (or 40) */
	ra_outl(SPI_REG_OPCODE, (code << 24) & 0xff000000);
	ra_or(SPI_REG_OPCODE, (addr & 0xffffff));

	ra_and(SPI_REG_MOREBUF, ~SPI_MBCTL_CMD_MASK);
	ra_or(SPI_REG_MOREBUF, (32 << 24));

	/* step 2. write DI/DO data #0 ~ #7 */
	if (flag & SPIC_WRITE_BYTES) {
		if (buf == NULL) {
			printf("%s: write null buf\n", __func__);
			goto RET_MB_TRANS;
		}
		for (i = 0; i < n_tx; i++) {
			q = i / 4;
			r = i % 4;
			if (r == 0)
				ra_outl(SPI_REG_DATA(q), 0);
			ra_or(SPI_REG_DATA(q), (*(buf + i) << (r * 8)));
		}
	}

	/* step 3. set rx (miso_bit_cnt) and tx (mosi_bit_cnt) bit count */
	ra_and(SPI_REG_MOREBUF, ~SPI_MBCTL_TX_RX_CNT_MASK);
	ra_or(SPI_REG_MOREBUF, (n_rx << 3 << 12));
	ra_or(SPI_REG_MOREBUF, n_tx << 3);

	/* step 4. kick */
	ra_or(SPI_REG_CTL, SPI_CTL_START);

	/* step 5. wait spi_master_busy */
	bbu_spic_busy_wait();
	if (flag & SPIC_WRITE_BYTES) {
		rc = 0;
		goto RET_MB_TRANS;
	}

	/* step 6. read DI/DO data #0 */
	if (flag & SPIC_READ_BYTES) {
		if (buf == NULL) {
			printf("%s: read null buf\n", __func__);
			return -1;
		}
		for (i = 0; i < n_rx; i++) {
			q	   = i / 4;
			r	   = i % 4;
			reg	   = ra_inl(SPI_REG_DATA(q));
			*(buf + i) = (u8)(reg >> (r * 8));
		}
	}

	rc = 0;
RET_MB_TRANS:
	/* step #. disable more byte mode */
	ra_and(SPI_REG_MASTER, ~(1 << 2));
	return rc;
}

static int bbu_spic_trans(const u8 code, const u32 addr, u8 *buf,
			  const size_t n_tx, const size_t n_rx, int flag)
{
	u32 reg;

	bbu_spic_busy_wait();

	/* step 1. set opcode & address */
	ra_outl(SPI_REG_OPCODE, ((addr & 0xffffff) << 8));
	ra_or(SPI_REG_OPCODE, code);

	/* step 2. write DI/DO data #0 */
	if (flag & SPIC_WRITE_BYTES) {
		if (buf == NULL) {
			printf("%s: write null buf\n", __func__);
			return -1;
		}
		ra_outl(SPI_REG_DATA0, 0);
		switch (n_tx) {
		case 8:
			ra_or(SPI_REG_DATA0, (*(buf + 3) << 24));
		case 7:
			ra_or(SPI_REG_DATA0, (*(buf + 2) << 16));
		case 6:
			ra_or(SPI_REG_DATA0, (*(buf + 1) << 8));
		case 5:
		case 2:
			ra_or(SPI_REG_DATA0, *buf);
			break;
		default:
			printf("%s: fixme, write of length %d\n", __func__,
			       n_tx);
			return -1;
		}
	}

	/* step 3. set mosi_byte_cnt */
	ra_and(SPI_REG_CTL, ~SPI_CTL_TX_RX_CNT_MASK);
	ra_or(SPI_REG_CTL, (n_rx << 4));
	ra_or(SPI_REG_CTL, n_tx);

	/* step 4. kick */
	ra_or(SPI_REG_CTL, SPI_CTL_START);

	/* step 5. wait spi_master_busy */
	bbu_spic_busy_wait();
	if (flag & SPIC_WRITE_BYTES)
		return 0;

	/* step 6. read DI/DO data #0 */
	if (flag & SPIC_READ_BYTES) {
		if (buf == NULL) {
			printf("%s: read null buf\n", __func__);
			return -1;
		}
		reg = ra_inl(SPI_REG_DATA0);
		switch (n_rx) {
		case 4:
			*(buf + 3) = (u8)(reg >> 24);
		case 3:
			*(buf + 2) = (u8)(reg >> 16);
		case 2:
			*(buf + 1) = (u8)(reg >> 8);
		case 1:
			*buf = (u8)reg;
			break;
		default:
			printf("%s: fixme, read of length %d\n", __func__,
			       n_rx);
			return -1;
		}
	}
	return 0;
}

static int raspi_read_rg(u8 code, u8 *val);
static int raspi_write_rg(u8 code, u8 *val);

static int raspi_read_sr(u8 *val)
{
	return raspi_read_rg(OPCODE_RDSR, val);
}
static int raspi_write_sr(u8 *val)
{
	return raspi_write_rg(OPCODE_WRSR, val);
}

u32 mtk_sf_jedec_id(u32 bank)
{
	u32 n_rx = 3;
	u32 n_tx = 1;
	u32 reg;

	bbu_spic_busy_wait();

	/* step 1. set opcode & address */
	ra_outl(SPI_REG_OPCODE, (0x00FFFFFF << 8));
	ra_or(SPI_REG_OPCODE, SPI_FLASH_CMD_JEDEC);

	/* step 3. set mosi_byte_cnt */
	ra_and(SPI_REG_CTL, ~SPI_CTL_TX_RX_CNT_MASK);
	ra_or(SPI_REG_CTL, (n_rx << 4));
	ra_or(SPI_REG_CTL, n_tx);

	/* step 4. kick */
	ra_or(SPI_REG_CTL, SPI_CTL_START);

	/* step 5. wait spi_master_busy */
	bbu_spic_busy_wait();

	/* step 6. read DI/DO data #0 */
	reg = ra_inl(SPI_REG_DATA0);

	return cpu_to_be32(reg & 0x00FFFFFF) >> 8;
}

/*
 * read status register
 */
static int raspi_read_rg(u8 code, u8 *val)
{
	ssize_t retval;

	retval = bbu_spic_trans(code, 0, val, 1, 1, SPIC_READ_BYTES);
	return retval;
}

/*
 * write status register
 */
static int raspi_write_rg(u8 code, u8 *val)
{
	ssize_t retval;

	// put the value to be written in address register, so it will be transfered
	u32 address = (*val) << 24;
	retval = bbu_spic_trans(code, address, val, 2, 0, SPIC_WRITE_BYTES);
	return retval;
}

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline int raspi_write_enable(void)
{
	u8 code = OPCODE_WREN;

	return bbu_spic_trans(code, 0, NULL, 1, 0, 0);
}

static inline int raspi_write_disable(void)
{
	u8 code = OPCODE_WRDI;

	return bbu_spic_trans(code, 0, NULL, 1, 0, 0);
}

/*
 * Set all sectors (global) unprotected if they are protected.
 * Returns negative if error occurred.
 */
static inline void raspi_unprotect(void)
{
	u8 sr = 0;

	if (raspi_read_sr(&sr) < 0) {
		printf("%s: read_sr fail: %x\n", __func__, sr);
		return;
	}

	if ((sr & (SR_BP0 | SR_BP1 | SR_BP2)) != 0) {
		sr = 0;
		raspi_write_sr(&sr);
	}
}

/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int raspi_wait_ready(int sleep_ms)
{
	int count;
	int sr = 0;

	//udelay(1000 * sleep_ms);

	/* one chip guarantees max 5 msec wait here after page writes,
	 * but potentially three seconds (!) after page erase.
	 */
	for (count = 0; count < ((sleep_ms + 1) * 1000 * 500); count++) {
		if ((raspi_read_sr((u8 *)&sr)) < 0)
			break;
		else if (!(sr & SR_WIP)) {
			return 0;
		}

		udelay(1);
		/* REVISIT sometimes sleeping would be best */
	}

	printf("%s: read_sr fail: %x\n", __func__, sr);
	return -1;
}

/*
 * Erase one sector of flash memory at offset ``offset'' which is any
 * address within the sector which should be erased.
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int raspi_erase_sector(u32 offset)
{
	/* Wait until finished previous write command. */
	if (raspi_wait_ready(950))
		return -1;

	/* Send write enable, then erase commands. */
	raspi_write_enable();
	raspi_unprotect();

	bbu_spic_trans(STM_OP_SECTOR_ERASE, offset, NULL, 4, 0, 0);
	raspi_wait_ready(950);

	raspi_write_disable();

	return 0;
}

/* Use 4 MB by default */
#ifndef CONFIG_DEFAULT_FLASH_SIZE_IN_MB
#define CONFIG_DEFAULT_FLASH_SIZE_IN_MB 4
#endif

/*
 * Find SPI NOR FLASH chip info for selected bank,
 * based on JEDEC ID and copy data to global flash_info variable
 */
static u32 flash_info_find(flash_info_t *info, u32 jedec_id)
{
	u32 i;

	for (i = 0; i < spi_nor_ids_count; i++) {
		if (jedec_id == spi_nor_ids[i].flash_id) {
			info->model_name   = spi_nor_ids[i].model_name;
			info->size	   = spi_nor_ids[i].size;
			info->sector_size  = spi_nor_ids[i].sector_size;
			info->page_size	   = spi_nor_ids[i].page_size;
			info->erase_cmd	   = spi_nor_ids[i].erase_cmd;
			info->sector_count = info->size / info->sector_size;

			return 0;
		}
	}

	return 1;
}

/*
 * Scan all configured FLASH banks one by one
 * and try to get information about the chips
 */
u32 flash_init(void)
{
	u32 bank, i, jedec_id;
	u32 total_size = 0;
	flash_info_t *info;

	for (bank = 0; bank < CFG_MAX_FLASH_BANKS; bank++) {
		info = &flash_info[bank];

		jedec_id = mtk_sf_jedec_id(bank);

		if (jedec_id == 0) {
			printf_err("SPI NOR FLASH chip in bank #%d\n"
				   "   is not responding, skipping\n\n",
				   bank + 1);
			continue;
		}

		info->manuf_name = (char *)flash_manuf_name(jedec_id);
		info->flash_id	 = jedec_id;
		info->bank	 = bank;
		if (info->flash_id != MICRON_N25Q128_ID)
			info->can_be_locked = 0;
		else
			info->can_be_locked = 1;

		flash_info_find(info, jedec_id);

		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = CFG_FLASH_BASE + total_size +
					 (i * info->sector_size);
		}

		total_size += flash_info[bank].size;
	}

	return total_size;
}

/*
 * Erase all FLASH sectors in provided range
 */
u32 flash_erase(flash_info_t *info, u32 s_first, u32 s_last)
{
	u32 i, j;

	printf("Erasing: ");

#ifndef LEAVE_LEDS_ALONE
#if defined(CONFIG_CMD_LED)
	all_led_off();
#if defined(CONFIG_SHIFT_REG)
	sr_led_off();
#endif
#endif // CONFIG_CMD_LED
#endif // LEAVE_LEDS_ALONE

	j = 0;
	for (i = s_first; i <= s_last; i++) {
		raspi_erase_sector(i * info->sector_size);

#ifndef LEAVE_LEDS_ALONE
#if defined(CONFIG_CMD_LED)
		led_animation(0);
#if defined(CONFIG_SHIFT_REG)
		sr_led_animation(0);
#endif
#endif // CONFIG_CMD_LED
#endif // LEAVE_LEDS_ALONE

		if (j == 39) {
			puts("\n         ");
			j = 0;
		}
		puts("#");

		j++;
	}

	printf("\n\n");

	return 0;
}

/*
 * Write a buffer from memory to a FLASH:
 * call page program for every <= 256 bytes
 *
 * Assumption: caller has already erased the appropriate sectors
 */
u32 write_buff(flash_info_t *info, uchar *source, ulong addr, ulong len)
{
	u32 total = 0, len_this_lp, bytes_this_page;
	u32 dst;
	u8 *src;

#ifndef LEAVE_LEDS_ALONE
	int i = 0;
	all_led_on();
#endif // LEAVE_LEDS_ALONE

	printf("Writing at address: 0x%08lX\n", addr);
	addr = addr - CFG_FLASH_BASE;

	while (total < len) {
		src		= source + total;
		dst		= addr + total;
		bytes_this_page = info->page_size - (addr % info->page_size);
		len_this_lp	= ((len - total) > bytes_this_page) ?
						bytes_this_page :
						(len - total);

		raspi_write((char *)src, dst, len_this_lp);

		total += len_this_lp;

#ifndef LEAVE_LEDS_ALONE
		if (i == 256) {
			led_animation(1);
#if defined(CONFIG_SHIFT_REG)
			sr_led_animation(1);
#endif
			i = 0;
		}

		i++;
#endif // LEAVE_LEDS_ALONE
	}


	puts("\n");

	return 0;
}

int raspi_write(char *buf, unsigned int to, int len)
{
	u32 page_offset, page_size;
	int rc = 0, retlen = 0;
	int wrto, wrlen, more;
	u8 *wrbuf;

	/* sanity checks */
	if (len == 0)
		return 0;
	//if (to + len > spi_chip_info->sector_size * spi_chip_info->n_sectors)
	//	return -1;

	/* Wait until finished previous write command. */
	if (raspi_wait_ready(2))
		return -1;

	/* what page do we start with? */
	page_offset = to % FLASH_PAGESIZE;

	/* write everything in PAGESIZE chunks */
	while (len > 0) {
		page_size   = min(len, FLASH_PAGESIZE - page_offset);
		page_offset = 0;

		/* write the next page to flash */

		raspi_wait_ready(3);
		raspi_write_enable();
		raspi_unprotect();

		wrto  = to;
		wrlen = page_size;
		wrbuf = (u8 *)buf;
		do {
			more = 32;
			if (wrlen <= more) {
				bbu_mb_spic_trans(STM_OP_PAGE_PGRM, wrto, wrbuf,
						  wrlen, 0, SPIC_WRITE_BYTES);
				retlen += wrlen;
				wrlen = 0;
			} else {
				bbu_mb_spic_trans(STM_OP_PAGE_PGRM, wrto, wrbuf,
						  more, 0, SPIC_WRITE_BYTES);
				retlen += more;
				wrto += more;
				wrlen -= more;
				wrbuf += more;
			}
			if (wrlen > 0) {
				raspi_write_disable();
				raspi_wait_ready(100);
				raspi_write_enable();
			}
		} while (wrlen > 0);

		if (rc > 0) {
			retlen += rc;
			if (rc < page_size) {
				printf("%s: rc:%x page_size:%x\n", __func__, rc,
				       page_size);
				return retlen;
			}
		}

		len -= page_size;
		to += page_size;
		buf += page_size;
	}

	raspi_wait_ready(100);
	raspi_write_disable();

	return retlen;
}
