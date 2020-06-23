#include <config.h>
#include <common.h>
#include <asm/addrspace.h>
#include <soc/qca_soc_common.h>

#define SR_74X164_TIME_UNIT 1 /* 1 usec */

/* No support for register chaining for now. */
static void sr_74x164_set_multiple(u8 mask)
{
	int i;

	/* Punch in the data */
	for (i = sizeof(mask) * 8; i > 0; i--, mask <<= 1) {
		//u8 bit = mask & 0x80;

		if (mask & 0x80)
			qca_soc_reg_read_set(QCA_GPIO_OUT_REG, GPIO_SR_74X164_SER);
		else
			qca_soc_reg_read_clear(QCA_GPIO_OUT_REG, GPIO_SR_74X164_SER);

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

void all_led_on_sr(void)
{
	sr_74x164_set_multiple(0x5F);
	all_led_on();
}

void all_led_off_sr(void)
{
	all_led_off();
	sr_74x164_set_multiple(0);
}
