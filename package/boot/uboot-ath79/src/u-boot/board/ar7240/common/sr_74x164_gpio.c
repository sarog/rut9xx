#include <config.h>
#include <common.h>
#include <asm/addrspace.h>
#include <soc/qca_soc_common.h>

#define SR_74X164_TIME_UNIT 1 /* 1 usec */

/* No support for register chaining for now. */
static void set_74x164(u16 mask)
{
	int i;

	/* Punch in the data */

	for (i = sizeof(mask) * 8; i > 0; i--, mask <<= 1) {

		if (mask & 0x8000) {
			qca_soc_reg_read_set(QCA_GPIO_OUT_REG, GPIO_SR_74X164_SER);
		}
		else {
			qca_soc_reg_read_clear(QCA_GPIO_OUT_REG, GPIO_SR_74X164_SER);
		}

		udelay(SR_74X164_TIME_UNIT);

		qca_soc_reg_read_set(QCA_GPIO_OUT_REG, GPIO_SR_74X164_SRCLK);
		udelay(SR_74X164_TIME_UNIT);
		qca_soc_reg_read_clear(QCA_GPIO_OUT_REG, GPIO_SR_74X164_SRCLK);
		udelay(SR_74X164_TIME_UNIT);
	}

	/* Store it */
	qca_soc_reg_read_set(QCA_GPIO_OUT_REG, GPIO_SR_74X164_RCLK);
	udelay(SR_74X164_TIME_UNIT);
	qca_soc_reg_read_clear(QCA_GPIO_OUT_REG, GPIO_SR_74X164_RCLK);
}

void sr_led_on(void)
{
	set_74x164(CONFIG_SR_LED_ALL_ON_MASK);
}

void sr_led_off(void)
{
	set_74x164(CONFIG_SR_LED_ALL_OFF_MASK);
}

void sr_led_animation(int reverse)
{
#if defined(CONFIG_SR_LED_ANIMATION_MASK)
	const u16 array[] = {
		CONFIG_SR_LED_ANIMATION_MASK
	};

	static int cycle = 0;
	int len = sizeof(array) / sizeof(array[0]);
	u16 mask = 0;

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