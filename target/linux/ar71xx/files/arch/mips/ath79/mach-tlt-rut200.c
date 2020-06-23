/*
 *  Teltonika RUT900 board support
 *
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define TLT_RUT900_GPIO_BTN_RESET	22

#define TLT_RUT900_GPIO_LED_SS1	23
#define TLT_RUT900_GPIO_LED_SS2	7
#define TLT_RUT900_GPIO_LED_SS3	6
#define TLT_RUT900_GPIO_LED_SS4	26
#define TLT_RUT900_GPIO_LED_SS5	27

#define TLT_RUT900_GPIO_LED_2G		8
#define TLT_RUT900_GPIO_LED_3G		24
#define TLT_RUT900_GPIO_LED_4G		21

#define TLT_RUT900_GPIO_USB_POWER 19
#define TLT_RUT900_GPIO_BTN_SIM_HOLDER	20
#define TLT_RUT900_GPIO_BTN_INPUT	16

#define TLT_RUT900_KEYS_POLL_INTERVAL	20	/* msecs */
#define TLT_RUT900_KEYS_DEBOUNCE_INTERVAL (3 * TLT_RUT900_KEYS_POLL_INTERVAL)

#define RUT900_WMAC_CALDATA_OFFSET	0x1000

int hwver;

static void __init
tlt_rut200_lanwan_setup(void)
{
	u8 *hwversion = (u8 *) KSEG1ADDR(0x1f020052);
	static char rut200hwver[2];
	memcpy(rut200hwver, hwversion, 2);
	if (!strcmp(rut200hwver, "04") || !strcmp(rut200hwver, "03")
	    || !strcmp(rut200hwver, "02") || !strcmp(rut200hwver, "01")) {
		hwver = 0;
		ath79_setup_ar933x_phy4_switch(true, true);
		ath79_gpio_function_disable(AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
									AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
									AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
									AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
									AR933X_GPIO_FUNC_ETH_SWITCH_LED4_EN);
	} else if (!strcmp(rut200hwver, "05")) {
		hwver = 1;
		ath79_setup_ar933x_phy4_switch(false, false);
		ath79_gpio_function_enable(AR933X_GPIO_FUNC_ETH_SWITCH_LED4_EN);
		ath79_gpio_function_disable(AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
									AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
									AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
									AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN);
	} else {
		hwver = 1;
		ath79_setup_ar933x_phy4_switch(false, false);
		ath79_gpio_function_enable(AR933X_GPIO_FUNC_ETH_SWITCH_LED4_EN | AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN);
		ath79_gpio_function_disable(AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
									AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
									AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN);
	}
	printk(KERN_INFO "HWver %s\n", rut200hwver);
}

static struct gpio_led tlt_rut200_leds_gpio[] __initdata = {
	{
		.name		= "signal_bar0",
		.gpio		= TLT_RUT900_GPIO_LED_SS1,
		.active_low	= 1,
	}, {
		.name		= "signal_bar1",
		.gpio		= TLT_RUT900_GPIO_LED_SS2,
		.active_low	= 1,
	}, {
		.name		= "signal_bar2",
		.gpio		= TLT_RUT900_GPIO_LED_SS3,
		.active_low	= 1,
	}, {
		.name		= "signal_bar3",
		.gpio		= TLT_RUT900_GPIO_LED_SS4,
		.active_low	= 1,
	}, {
		.name		= "signal_bar4",
		.gpio		= TLT_RUT900_GPIO_LED_SS5,
		.active_low	= 1,
	}, {
		.name		= "status_2g",
		.gpio		= TLT_RUT900_GPIO_LED_2G,
		.active_low	= 0,
	}, {
		.name		= "status_3g",
		.gpio		= TLT_RUT900_GPIO_LED_3G,
		.active_low	= 0,
	}, {
		.name		= "status_4g",
		.gpio		= TLT_RUT900_GPIO_LED_4G,
		.active_low	= 0,
	}, {
		.name		= "led_lan",
		.gpio		= 13,
		.active_low	= 0,
	}, {
		.name		= "led_wan",
		.gpio		= 14,
		.active_low	= 0,
	},
};

static struct gpio_led tlt_rut200_leds_gpio1[] __initdata = {
	{
		.name		= "signal_bar0",
		.gpio		= TLT_RUT900_GPIO_LED_SS1,
		.active_low	= 1,
	}, {
		.name		= "signal_bar1",
		.gpio		= TLT_RUT900_GPIO_LED_SS2,
		.active_low	= 1,
	}, {
		.name		= "signal_bar2",
		.gpio		= TLT_RUT900_GPIO_LED_SS3,
		.active_low	= 1,
	}, {
		.name		= "signal_bar3",
		.gpio		= TLT_RUT900_GPIO_LED_SS4,
		.active_low	= 1,
	}, {
		.name		= "signal_bar4",
		.gpio		= TLT_RUT900_GPIO_LED_SS5,
		.active_low	= 1,
	}, {
		.name		= "status_2g",
		.gpio		= TLT_RUT900_GPIO_LED_2G,
		.active_low	= 0,
	}, {
		.name		= "status_3g",
		.gpio		= TLT_RUT900_GPIO_LED_3G,
		.active_low	= 0,
	}, {
		.name		= "status_4g",
		.gpio		= TLT_RUT900_GPIO_LED_4G,
		.active_low	= 0,
	}, {
		.name		= "led_lan",
		.gpio		= 14,
		.active_low	= 0,
	}, {
		.name		= "led_wan",
		.gpio		= 17,
		.active_low	= 0,
	},
};
static const char *rut200_part_probes[] = {
	"teltonika",
	NULL,
};

static struct flash_platform_data rut200_flash_data = {
	.part_probes = rut200_part_probes,
};

static struct gpio_keys_button tlt_rut200_gpio_keys[] __initdata = {
	{
	 .desc = "reset",
	 .type = EV_KEY,
	 .code = KEY_RESTART,
	 .debounce_interval = TLT_RUT900_KEYS_DEBOUNCE_INTERVAL,
	 .gpio = TLT_RUT900_GPIO_BTN_RESET,
	 .active_low = 0,
	 },
	{
	 .desc = "input",
	 .type = EV_KEY,
	 .code = BTN_0,
	 .debounce_interval = TLT_RUT900_KEYS_DEBOUNCE_INTERVAL,
	 .gpio = TLT_RUT900_GPIO_BTN_INPUT,
	 .active_low = 1,
	 },
	{
	 .desc = "SIM holder",
	 .type = EV_KEY,
	 .code = KEY_RFKILL,
	 .debounce_interval = TLT_RUT900_KEYS_DEBOUNCE_INTERVAL,
	 .gpio = TLT_RUT900_GPIO_BTN_SIM_HOLDER,
	 .active_low = 1,
	 }
};

static void __init
tl_ap121_setup(void)
{

	u8 *mac = (u8 *) KSEG1ADDR(0x1f020000);
	u8 *ee = (u8 *) KSEG1ADDR(0x1f031000);
	u8 tmpmac[ETH_ALEN];
	ath79_init_mac(tmpmac, mac, 2);

	ath79_register_m25p80(&rut200_flash_data);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(1);
	ath79_register_eth(0);

	//ath79_register_wmac(ee + RUT900_WMAC_CALDATA_OFFSET, tmpmac);
	ath79_register_wmac(ee, tmpmac);

}

static void __init
tlt_rut200_setup(void)
{
	tl_ap121_setup();

	tlt_rut200_lanwan_setup();

	ath79_register_usb();
	if (hwver == 1) {
		ath79_register_leds_gpio(-1, ARRAY_SIZE(tlt_rut200_leds_gpio1), tlt_rut200_leds_gpio1);
	} else {
		ath79_register_leds_gpio(-1, ARRAY_SIZE(tlt_rut200_leds_gpio), tlt_rut200_leds_gpio);
	}

	ath79_register_gpio_keys_polled(1, TLT_RUT900_KEYS_POLL_INTERVAL, ARRAY_SIZE(tlt_rut200_gpio_keys), tlt_rut200_gpio_keys);

}

MIPS_MACHINE(ATH79_MACH_TLT_RUT900, "TLT-RUT900", "Teltonika RUT900", tlt_rut200_setup);
