#include <common.h>
#include <command.h>
#include <rt_mmap.h>
#include <configs/rt2880.h>

DECLARE_GLOBAL_DATA_PTR;

#define outw(address, value)                                                   \
	*((volatile uint32_t *)(address)) = cpu_to_le32(value)
#define inw(address) le32_to_cpu(*(volatile u32 *)(address))

#define PHY_CONTROL_0	   0xC0
#define PHY_CONTROL_1	   0xC4
#define MDIO_PHY_CONTROL_0 (RALINK_ETH_SW_BASE + PHY_CONTROL_0)
#define MDIO_PHY_CONTROL_1 (RALINK_ETH_SW_BASE + PHY_CONTROL_1)

#define GPIO_MDIO_BIT	    (1 << 7)
#define GPIO_PURPOSE_SELECT 0x60
#define GPIO_PRUPOSE	    (RALINK_SYSCTL_BASE + GPIO_PURPOSE_SELECT)

void enable_mdio(int enable)
{
}

u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data)
{
	u32 volatile status	       = 0;
	u32 rc			       = 0;
	unsigned long volatile t_start = get_timer(0);
	bd_t *bd		       = gd->bd;

	/* We enable mdio gpio purpose register, and disable it when exit.	 */
	enable_mdio(1);

	// make sure previous read operation is complete
	while (1) {
		// rd_rdy: read operation is complete
		if (!(inw(MDIO_PHY_CONTROL_1) & (0x1 << 1))) {
			break;
		} else if (get_timer(t_start) > (5 * CFG_HZ)) {
			enable_mdio(0);
			printf("\n MDIO Read operation is ongoing !!\n");
			return rc;
		}
	}

	outw(MDIO_PHY_CONTROL_0, (1 << 14) | (phy_register << 8) | (phy_addr));
	//printf("\n Set Command [0x%08X] to PHY !!\n",MDIO_PHY_CONTROL_0);

	// make sure read operation is complete
	t_start = get_timer(0);
	while (1) {
		if (inw(MDIO_PHY_CONTROL_1) & (0x1 << 1)) {
			status	   = inw(MDIO_PHY_CONTROL_1);
			*read_data = (u32)(status >> 16);

			enable_mdio(0);
			return 1;
		} else if (get_timer(t_start) > (5 * CFG_HZ)) {
			enable_mdio(0);
			printf("\n MDIO Read operation is ongoing and Time Out!!\n");
			return 0;
		}
	}
}

u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
	unsigned long volatile t_start = get_timer(0);
	u32 volatile data;
	bd_t *bd = gd->bd;

	enable_mdio(1);

	// make sure previous write operation is complete
	while (1) {
		if (!(inw(MDIO_PHY_CONTROL_1) & (0x1 << 0))) {
			break;
		} else if (get_timer(t_start) > (5 * CFG_HZ)) {
			enable_mdio(0);
			printf("\n MDIO Write operation is ongoing !!\n");
			return 0;
		}
	}

	data = ((write_data & 0xFFFF) << 16);
	data |= (phy_register << 8) | (phy_addr);
	data |= (1 << 13);
	outw(MDIO_PHY_CONTROL_0, data);
	//printf("\n Set Command [0x%08X] to PHY !!\n",MDIO_PHY_CONTROL_0);

	t_start = get_timer(0);

	// make sure write operation is complete
	while (1) {
		if (inw(MDIO_PHY_CONTROL_1) & (0x1 << 0)) //wt_done ?= 1
		{
			enable_mdio(0);
			return 1;
		} else if (get_timer(t_start) > (5 * CFG_HZ)) {
			enable_mdio(0);
			printf("\n MDIO Write operation is ongoing and Time Out!!\n");
			return 0;
		}
	}
}
