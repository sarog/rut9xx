/*
 * MediaTek/Ralink WiSoCs common/helper functions
 *
 * Copyright (C) 2021 Jokubas Maciulaitis <jokubas.maciulaitis@teltonika.lt>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <config.h>
#include <common.h>
#include <soc/mtk_soc_common.h>
#include <mtk_gpio.h>

/* MT7628 contains 3 gpio chips
 * Each gpip chip supports 32 GPIO pins
 */
#ifdef MTK_MT7628_SOC
#define MTK_GPIO_CHIP_0 31
#define MTK_GPIO_CHIP_1 63
#define MTK_GPIO_CHIP_2 95
#endif // MTK_MT7628_SOC

/* GPIO function modes as defined in MT datasheet
 * Usually we need GPIO function which is "1"
 */
#ifdef MTK_MT7628_SOC
#define MTK_GPIO_FUNC_0 0
#define MTK_GPIO_FUNC_1 1
#define MTK_GPIO_FUNC_2 2
#define MTK_GPIO_FUNC_3 3
#endif // MTK_MT7628_SOC

/* LED_ACT and OUT_INIT are configued in the same cycle
 * in case if one is not defined, re-define it
 */
#if !defined(CONFIG_MTK_GPIO_MASK_LED_ACT_L) &&                                \
	defined(CONFIG_MTK_GPIO_MASK_OUT_INIT_L)
#define CONFIG_MTK_GPIO_MASK_LED_ACT_L CONFIG_MTK_GPIO_MASK_OUT_INIT_L
#endif // CONFIG_MTK_GPIO_MASK_LED_ACT_L

#if !defined(CONFIG_MTK_GPIO_MASK_OUT_INIT_L) &&                               \
	defined(CONFIG_MTK_GPIO_MASK_LED_ACT_L)
#define CONFIG_MTK_GPIO_MASK_OUT_INIT_L CONFIG_MTK_GPIO_MASK_LED_ACT_L
#endif // CONFIG_MTK_GPIO_MASK_OUT_INIT_L

#if !defined(CONFIG_MTK_GPIO_MASK_LED_ACT_H) &&                                \
	defined(CONFIG_MTK_GPIO_MASK_OUT_INIT_H)
#define CONFIG_MTK_GPIO_MASK_LED_ACT_H CONFIG_MTK_GPIO_MASK_OUT_INIT_H
#endif // CONFIG_MTK_GPIO_MASK_LED_ACT_H

#if !defined(CONFIG_MTK_GPIO_MASK_OUT_INIT_H) &&                               \
	defined(CONFIG_MTK_GPIO_MASK_LED_ACT_H)
#define CONFIG_MTK_GPIO_MASK_OUT_INIT_H CONFIG_MTK_GPIO_MASK_LED_ACT_H
#endif // CONFIG_MTK_GPIO_MASK_OUT_INIT_H

#ifndef SR_74X164_TIME_UNIT
#define SR_74X164_TIME_UNIT 1 /* 1 usec */
#endif

typedef struct {
	u64 gpio;
	u32 reg;
	u32 mask;
} gpio_func;

static const gpio_func gpio_out_func[] = {
	{
		/* I2S_MODE GPIO0 */
		.gpio = GPIO0,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_I2S_SHIFT),
	},
	{
		/* I2S_MODE GPIO1 */
		.gpio = GPIO1,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_I2S_SHIFT),
	},
	{
		/* I2S_MODE GPIO2 */
		.gpio = GPIO2,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_I2S_SHIFT),
	},
	{
		/* I2S_MODE GPIO3 */
		.gpio = GPIO3,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_I2S_SHIFT),
	},
	{
		/* I2C_MODE GPIO4 */
		.gpio = GPIO4,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_I2C_SHIFT),
	},
	{
		/* I2C_MODE GPIO5 */
		.gpio = GPIO5,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_I2C_SHIFT),
	},
	{
		/* SPI_CS1 GPIO6 */
		.gpio = GPIO6,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1
			 << MTK_SYSCTL_GPIO1_MODE_SPI_CS1_SHIFT),
	},
	{
		/* SPI_MODE GPIO7 */
		.gpio = GPIO7,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_SPI_SHIFT),
	},
	{
		/* SPI_MODE GPIO8 */
		.gpio = GPIO8,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_SPI_SHIFT),
	},
	{
		/* SPI_MODE GPIO9 */
		.gpio = GPIO9,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_SPI_SHIFT),
	},
	{
		/* SPI_MODE GPIO10 */
		.gpio = GPIO10,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_SPI_SHIFT),
	},
	{
		/* GPIO_MODE GPIO11 */
		.gpio = GPIO11,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_GPIO_SHIFT),
	},
	{
		/* UART0_MODE GPIO12 */
		.gpio = GPIO12,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_UART0_SHIFT),
	},
	{
		/* UART0_MODE GPIO13 */
		.gpio = GPIO13,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_UART0_SHIFT),
	},
	{
		/* P4_LED_KN_MODE GPIO30 */
		.gpio = GPIO30,
		.reg  = MTK_SYSCTL_GPIO2_MODE,
		.mask = (MTK_GPIO_FUNC_1
			 << MTK_SYSCTL_GPIO2_MODE_P4_LED_KN_SHIFT),
	},
	{
		/* P3_LED_KN_MODE GPIO31 */
		.gpio = GPIO31,
		.reg  = MTK_SYSCTL_GPIO2_MODE,
		.mask = (MTK_GPIO_FUNC_1
			 << MTK_SYSCTL_GPIO2_MODE_P3_LED_KN_SHIFT),
	},
	{
		/* P2_LED_KN_MODE GPIO32 */
		.gpio = GPIO32,
		.reg  = MTK_SYSCTL_GPIO2_MODE,
		.mask = (MTK_GPIO_FUNC_1
			 << MTK_SYSCTL_GPIO2_MODE_P2_LED_KN_SHIFT),
	},
	{
		/* P1_LED_KN_MODE GPIO33 */
		.gpio = GPIO33,
		.reg  = MTK_SYSCTL_GPIO2_MODE,
		.mask = (MTK_GPIO_FUNC_1
			 << MTK_SYSCTL_GPIO2_MODE_P1_LED_KN_SHIFT),
	},
	{
		/* P0_LED_KN_MODE */
		.gpio = GPIO34,
		.reg  = MTK_SYSCTL_GPIO2_MODE,
		.mask = (MTK_GPIO_FUNC_1
			 << MTK_SYSCTL_GPIO2_MODE_P0_LED_KN_SHIFT),
	},
	{
		/* WLED_KN_MODE GPIO35 */
		.gpio = GPIO35,
		.reg  = MTK_SYSCTL_GPIO2_MODE,
		.mask = (MTK_GPIO_FUNC_1
			 << MTK_SYSCTL_GPIO2_MODE_WLED_KN_SHIFT),
	},
	{
		/* PERST_MODE GPIO36 */
		.gpio = GPIO36,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_PERST_SHIFT),
	},
	{
		/* WDT_MODE GPIO37 */
		.gpio = GPIO37,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_WDT_SHIFT),
	},
	{
		/* REFCLK_MODE GPIO38 */
		.gpio = GPIO38,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_REFCLK_SHIFT),
	},
	{
		/* P4_LED_AN_MODE GPIO39 */
		.gpio = GPIO39,
		.reg  = MTK_SYSCTL_GPIO2_MODE,
		.mask = (MTK_GPIO_FUNC_1
			 << MTK_SYSCTL_GPIO2_MODE_P4_LED_AN_SHIFT),
	},
	{
		/* P3_LED_AN_MODE GPIO40 */
		.gpio = GPIO40,
		.reg  = MTK_SYSCTL_GPIO2_MODE,
		.mask = (MTK_GPIO_FUNC_1
			 << MTK_SYSCTL_GPIO2_MODE_P3_LED_AN_SHIFT),
	},
	{
		/* P2_LED_AN_MODE GPIO41 */
		.gpio = GPIO41,
		.reg  = MTK_SYSCTL_GPIO2_MODE,
		.mask = (MTK_GPIO_FUNC_1
			 << MTK_SYSCTL_GPIO2_MODE_P2_LED_AN_SHIFT),
	},
	{
		/* P1_LED_AN_MODE GPIO42 */
		.gpio = GPIO42,
		.reg  = MTK_SYSCTL_GPIO2_MODE,
		.mask = (MTK_GPIO_FUNC_1
			 << MTK_SYSCTL_GPIO2_MODE_P1_LED_AN_SHIFT),
	},
	{
		/* P0_LED_AN_MODE GPIO43 */
		.gpio = GPIO43,
		.reg  = MTK_SYSCTL_GPIO2_MODE,
		.mask = (MTK_GPIO_FUNC_1
			 << MTK_SYSCTL_GPIO2_MODE_P0_LED_AN_SHIFT),
	},
	{
		/* WLED_AN_MODE GPIO44 */
		.gpio = GPIO44,
		.reg  = MTK_SYSCTL_GPIO2_MODE,
		.mask = (MTK_GPIO_FUNC_1
			 << MTK_SYSCTL_GPIO2_MODE_WLED_AN_SHIFT),
	},
	{
		/* UART1_MODE GPIO45 */
		.gpio = GPIO45,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_UART1_SHIFT),
	},
	{
		/* UART1_MODE GPIO46 */
		.gpio = GPIO46,
		.reg  = MTK_SYSCTL_GPIO1_MODE,
		.mask = (MTK_GPIO_FUNC_1 << MTK_SYSCTL_GPIO1_MODE_UART1_SHIFT),
	},
};

static u32 gpio_out_func_len = sizeof(gpio_out_func) / sizeof(gpio_out_func[0]);

u32 mtk_gpio_num(u64 mask)
{
	for (u32 i = 0; i < 50; i++) {
		if ((1ULL << i) & mask) {
			return i;
		}
	}

	return 0;
}

u32 mtk_gpio_ctrl_reg(u32 gpio)
{
	if (gpio <= MTK_GPIO_CHIP_0) {
		return MTK_GPIO_CTRL_0;
	} else if (gpio <= MTK_GPIO_CHIP_1) {
		return MTK_GPIO_CTRL_1;
	} else {
		return MTK_GPIO_CTRL_2;
	}
}

u32 mtk_gpio_data_reg(u32 gpio)
{
	if (gpio <= MTK_GPIO_CHIP_0) {
		return MTK_GPIO_DATA_0;
	} else if (gpio <= MTK_GPIO_CHIP_1) {
		return MTK_GPIO_DATA_1;
	} else {
		return MTK_GPIO_DATA_2;
	}
}

u32 mtk_gpio_mask(u32 gpio)
{
	if (gpio <= MTK_GPIO_CHIP_0) {
		return (1 << gpio);
	} else if (gpio <= MTK_GPIO_CHIP_1) {
		return (1 << (gpio - (MTK_GPIO_CHIP_0 + 1)));
	} else {
		return (1 << (gpio - (MTK_GPIO_CHIP_1 + 2)));
	}
}

/* set gpio level by number in arr */
static void gpio_set_level(uint8_t nr, uint8_t high) {
	u32 num, reg, mask;

	if (nr >= gpio_out_func_len) return;

	// switch to gpio function
	RALINK_REG(gpio_out_func[nr].reg) |= gpio_out_func[nr].mask;

	num  = mtk_gpio_num(gpio_out_func[nr].gpio);
	reg  = mtk_gpio_ctrl_reg(num);
	mask = mtk_gpio_mask(num);

	// configure as output
	RALINK_REG(reg) |= mask;

	reg = mtk_gpio_data_reg(num);
	if (high) 
		RALINK_REG(reg) |= mask;
	else
		RALINK_REG(reg) &= ~mask;
}

#if defined(CONFIG_DO_PCIE_PERST_RESET)
static void pcie_perst_pulse() {

	for (u32 i = 0; i < gpio_out_func_len; i++) {
		if (!(gpio_out_func[i].gpio & GPIO36)) {
			continue;
		}

		gpio_set_level(i, 0);
		udelay(50 * 1000);
		gpio_set_level(i, 1);
		break;
	}

}
#endif

void all_led_on(void)
{
	u32 num, reg, mask;

#if defined(CONFIG_MTK_GPIO_MASK_LED_ACT_H)
	for (u32 i = 0; i < gpio_out_func_len; i++) {
		if (!(gpio_out_func[i].gpio & CONFIG_MTK_GPIO_MASK_LED_ACT_H)) {
			continue;
		}
		gpio_set_level(i, 1);
		break;
	}
#endif // CONFIG_MTK_GPIO_MASK_LED_ACT_H

#if defined(CONFIG_MTK_GPIO_MASK_LED_ACT_L)
	for (u32 i = 0; i < gpio_out_func_len; i++) {
		if (!(gpio_out_func[i].gpio & CONFIG_MTK_GPIO_MASK_LED_ACT_L)) {
			continue;
		}
		gpio_set_level(i, 0);
		break;
	}
#endif // CONFIG_MTK_GPIO_MASK_LED_ACT_L
}

void all_led_off(void)
{
	u32 num, reg, mask;

#if defined(CONFIG_MTK_GPIO_MASK_LED_ACT_H)
	for (u32 i = 0; i < gpio_out_func_len; i++) {
		if (!(gpio_out_func[i].gpio & CONFIG_MTK_GPIO_MASK_LED_ACT_H)) {
			continue;
		}
		gpio_set_level(i, 0);
		break;
	}
#endif // CONFIG_MTK_GPIO_MASK_LED_ACT_H

#if defined(CONFIG_MTK_GPIO_MASK_LED_ACT_L)
	for (u32 i = 0; i < gpio_out_func_len; i++) {
		if (!(gpio_out_func[i].gpio & CONFIG_MTK_GPIO_MASK_LED_ACT_L)) {
			continue;
		}
		gpio_set_level(i, 1);
		break;
	}
#endif // CONFIG_MTK_GPIO_MASK_LED_ACT_L
}

int early_gpio_init(void)
{
	u32 num, reg, mask;

#ifdef CONFIG_SHIFT_REG
	for (u32 i = 0; i < gpio_out_func_len; i++) {
		if (gpio_out_func[i].gpio != GPIO_SR_74X164_RCLK && gpio_out_func[i].gpio != GPIO_SR_74X164_SER &&
		    gpio_out_func[i].gpio != GPIO_SR_74X164_SRCLK) {
			continue;
		}
		gpio_set_level(i, 0);
	}
#endif

#if defined(CONFIG_MTK_GPIO_MASK_LED_ACT_H) ||                                 \
	defined(CONFIG_MTK_GPIO_MASK_OUT_INIT_H)
	for (u32 i = 0; i < gpio_out_func_len; i++) {
		if (!(gpio_out_func[i].gpio & CONFIG_MTK_GPIO_MASK_LED_ACT_H) ||
		    !(gpio_out_func[i].gpio & CONFIG_MTK_GPIO_MASK_OUT_INIT_H)) {
			continue;
		}
		gpio_set_level(i, 1);
	}
#endif // CONFIG_MTK_GPIO_MASK_LED_ACT_H

#if defined(CONFIG_MTK_GPIO_MASK_LED_ACT_L) ||                                 \
	defined(CONFIG_MTK_GPIO_MASK_OUT_INIT_L)
	for (u32 i = 0; i < gpio_out_func_len; i++) {
		if (!(gpio_out_func[i].gpio & CONFIG_MTK_GPIO_MASK_LED_ACT_L) ||
		    !(gpio_out_func[i].gpio & CONFIG_MTK_GPIO_MASK_OUT_INIT_L)) {
			continue;
		}
		gpio_set_level(i, 0);
	}
#endif // CONFIG_MTK_GPIO_MASK_LED_ACT_L

#if defined(CONFIG_MTK_GPIO_MASK_IN)
	for (u32 i = 0; i < gpio_out_func_len; i++) {
		if (!(gpio_out_func[i].gpio & CONFIG_MTK_GPIO_MASK_IN)) {
			continue;
		}

		// switch to gpio function
		RALINK_REG(gpio_out_func[i].reg) |= gpio_out_func[i].mask;

		num  = mtk_gpio_num(gpio_out_func[i].gpio);
		reg  = mtk_gpio_ctrl_reg(num);
		mask = mtk_gpio_mask(num);

		// configure as input
		RALINK_REG(reg) &= ~mask;
	}

#endif // CONFIG_MTK_GPIO_MASK_IN

	all_led_on();
	sr_led_on();

#if defined(CONFIG_DO_PCIE_PERST_RESET)
	/* rut9m reset pulse for usb hub */
	pcie_perst_pulse();
#endif

	return 0;
}

static void set_74x164(u16 mask)
{
#ifdef CONFIG_SHIFT_REG

	enum {
		BANG_DATA,
		BANG_CLOCK,
		BANG_LATCH,
	};

	struct {
		u32 reg, mask;

	} bang[] = {
		{
			.reg  = mtk_gpio_data_reg(mtk_gpio_num(GPIO_SR_74X164_SER)),
			.mask = mtk_gpio_mask(mtk_gpio_num(GPIO_SR_74X164_SER)),
		},
		{
			.reg  = mtk_gpio_data_reg(mtk_gpio_num(GPIO_SR_74X164_SRCLK)),
			.mask = mtk_gpio_mask(mtk_gpio_num(GPIO_SR_74X164_SRCLK)),
		},
		{
			.reg  = mtk_gpio_data_reg(mtk_gpio_num(GPIO_SR_74X164_RCLK)),
			.mask = mtk_gpio_mask(mtk_gpio_num(GPIO_SR_74X164_RCLK)),
		},
	};

	/* Punch in the data */

	for (int i = sizeof(mask) * 8; i > 0; i--, mask <<= 1) {
		if (mask & 0x8000) {
			RALINK_REG(bang[BANG_DATA].reg) |= bang[BANG_DATA].mask;

		} else {
			RALINK_REG(bang[BANG_DATA].reg) &= ~bang[BANG_DATA].mask;
		}

		udelay(SR_74X164_TIME_UNIT);

		RALINK_REG(bang[BANG_CLOCK].reg) |= bang[BANG_CLOCK].mask;
		udelay(SR_74X164_TIME_UNIT);

		RALINK_REG(bang[BANG_CLOCK].reg) &= ~bang[BANG_CLOCK].mask;
		udelay(SR_74X164_TIME_UNIT);
	}

	/* Store it */
	RALINK_REG(bang[BANG_LATCH].reg) |= bang[BANG_LATCH].mask;
	udelay(SR_74X164_TIME_UNIT);
	RALINK_REG(bang[BANG_LATCH].reg) &= ~bang[BANG_LATCH].mask;

#endif
}

void sr_led_on(void)
{
#if defined(CONFIG_SHIFT_REG)
	set_74x164(CONFIG_SR_LED_ALL_ON_MASK);
#endif
}

void sr_led_off(void)
{
#if defined(CONFIG_SHIFT_REG)
	set_74x164(CONFIG_SR_LED_ALL_OFF_MASK);
#endif
}

void sr_led_animation(int reverse)
{
#if defined(CONFIG_SHIFT_REG)
	const u16 array[] = { CONFIG_SR_LED_ANIMATION_MASK };

	static int cycle = 0;
	int len		 = sizeof(array) / sizeof(array[0]);
	u16 mask	 = 0;

	if (!reverse) {
		for (int i = 0; i < cycle; i++) {
			mask += array[i];
		}

		if (cycle == len) {
			cycle = -1;
		}

		cycle++;
	} else {
		for (int i = len - 1; i >= cycle; i--) {
			mask += array[i];
		}

		if (cycle == 0) {
			cycle = len + 1;
		}

		cycle--;
	}

	set_74x164(mask);
#endif
}
