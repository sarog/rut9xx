#include <common.h>
#include <command.h>

#include <malloc.h>
#include <net.h>
#include <asm/addrspace.h>
#include <rt_mmap.h>
#include <soc/mtk_soc_common.h>

/* ====================================== */
//GDMA1 uni-cast frames destination port
#define GDM_UFRC_P_CPU	 ((u32)(~(0x7 << 12)))
#define GDM_UFRC_P_GDMA1 (1 << 12)
#define GDM_UFRC_P_GDMA2 (2 << 12)
#define GDM_UFRC_P_DROP	 (7 << 12)
//GDMA1 broad-cast MAC address frames
#define GDM_BFRC_P_CPU	 ((u32)(~(0x7 << 8)))
#define GDM_BFRC_P_GDMA1 (1 << 8)
#define GDM_BFRC_P_GDMA2 (2 << 8)
#define GDM_BFRC_P_PPE	 (6 << 8)
#define GDM_BFRC_P_DROP	 (7 << 8)
//GDMA1 multi-cast MAC address frames
#define GDM_MFRC_P_CPU	 ((u32)(~(0x7 << 4)))
#define GDM_MFRC_P_GDMA1 (1 << 4)
#define GDM_MFRC_P_GDMA2 (2 << 4)
#define GDM_MFRC_P_PPE	 (6 << 4)
#define GDM_MFRC_P_DROP	 (7 << 4)
//GDMA1 other MAC address frames destination port
#define GDM_OFRC_P_CPU	 ((u32)(~(0x7)))
#define GDM_OFRC_P_GDMA1 1
#define GDM_OFRC_P_GDMA2 2
#define GDM_OFRC_P_PPE	 6
#define GDM_OFRC_P_DROP	 7

#define RST_DRX_IDX0 BIT(16)
#define RST_DTX_IDX0 BIT(0)

#define TX_WB_DDONE BIT(6)
#define RX_DMA_BUSY BIT(3)
#define TX_DMA_BUSY BIT(1)
#define RX_DMA_EN   BIT(2)
#define TX_DMA_EN   BIT(0)

#define GP1_FRC_EN  BIT(15)
#define GP1_FC_TX   BIT(11)
#define GP1_FC_RX   BIT(10)
#define GP1_LNK_DWN BIT(9)
#define GP1_AN_OK   BIT(8)

/*
 * FE_INT_STATUS
 */
#define CNT_PPE_AF   BIT(31)
#define CNT_GDM1_AF  BIT(29)
#define PSE_P1_FC    BIT(22)
#define PSE_P0_FC    BIT(21)
#define PSE_FQ_EMPTY BIT(20)
#define GE1_STA_CHG  BIT(18)
#define TX_COHERENT  BIT(17)
#define RX_COHERENT  BIT(16)

#define TX_DONE_INT1 BIT(9)
#define TX_DONE_INT0 BIT(8)
#define RX_DONE_INT0 BIT(2)
#define TX_DLY_INT   BIT(1)
#define RX_DLY_INT   BIT(0)

#define NUM_RX_DESC 24
#define NUM_TX_DESC 24

/*
 * Ethernet chip registers.RT2880
 */
#define RT2880_SYS_CNTL_BASE			(RALINK_SYSCTL_BASE)
#define RT2880_RSTCTRL_REG			(RT2880_SYS_CNTL_BASE+0x34)
#define RT2880_AGPIOCFG_REG			(RT2880_SYS_CNTL_BASE+0x3c)

#define PDMA_RELATED 0x0800
/* 1. PDMA */
#define TX_BASE_PTR0 (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x000)
#define TX_MAX_CNT0  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x004)
#define TX_CTX_IDX0  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x008)
#define TX_DTX_IDX0  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x00C)

#define TX_BASE_PTR1 (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x010)
#define TX_MAX_CNT1  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x014)
#define TX_CTX_IDX1  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x018)
#define TX_DTX_IDX1  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x01C)

#define TX_BASE_PTR2 (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x020)
#define TX_MAX_CNT2  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x024)
#define TX_CTX_IDX2  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x028)
#define TX_DTX_IDX2  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x02C)

#define TX_BASE_PTR3 (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x030)
#define TX_MAX_CNT3  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x034)
#define TX_CTX_IDX3  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x038)
#define TX_DTX_IDX3  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x03C)

#define RX_BASE_PTR0 (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x100)
#define RX_MAX_CNT0  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x104)
#define RX_CALC_IDX0 (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x108)
#define RX_DRX_IDX0  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x10C)

#define RX_BASE_PTR1 (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x110)
#define RX_MAX_CNT1  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x114)
#define RX_CALC_IDX1 (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x118)
#define RX_DRX_IDX1  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x11C)

#define PDMA_INFO     (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x200)
#define PDMA_GLO_CFG  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x204)
#define PDMA_RST_IDX  (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x208)
#define PDMA_RST_CFG  (RALINK_FRAME_ENGINE_BASE + PDMA_RST_IDX)
#define DLY_INT_CFG   (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x20C)
#define FREEQ_THRES   (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x210)
#define INT_STATUS    (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x220)
#define FE_INT_STATUS (INT_STATUS)
#define INT_MASK      (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x228)
#define FE_INT_ENABLE (INT_MASK)
#define PDMA_WRR      (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x280)
#define PDMA_SCH_CFG  (PDMA_WRR)

#define SDM_RELATED 0x0C00
#define SDM_CON                                                                \
	(RALINK_FRAME_ENGINE_BASE + SDM_RELATED +                              \
	 0x00) //Switch DMA configuration
#define SDM_RRING                                                              \
	(RALINK_FRAME_ENGINE_BASE + SDM_RELATED + 0x04) //Switch DMA Rx Ring
#define SDM_TRING                                                              \
	(RALINK_FRAME_ENGINE_BASE + SDM_RELATED + 0x08) //Switch DMA Tx Ring
#define SDM_MAC_ADRL                                                           \
	(RALINK_FRAME_ENGINE_BASE + SDM_RELATED + 0x0C) //Switch MAC address LSB
#define SDM_MAC_ADRH                                                           \
	(RALINK_FRAME_ENGINE_BASE + SDM_RELATED + 0x10) //Switch MAC Address MSB
#define SDM_TPCNT                                                              \
	(RALINK_FRAME_ENGINE_BASE + SDM_RELATED +                              \
	 0x100) //Switch DMA Tx packet count
#define SDM_TBCNT                                                              \
	(RALINK_FRAME_ENGINE_BASE + SDM_RELATED +                              \
	 0x104) //Switch DMA Tx byte count
#define SDM_RPCNT                                                              \
	(RALINK_FRAME_ENGINE_BASE + SDM_RELATED +                              \
	 0x108) //Switch DMA rx packet count
#define SDM_RBCNT                                                              \
	(RALINK_FRAME_ENGINE_BASE + SDM_RELATED +                              \
	 0x10C) //Switch DMA rx byte count
#define SDM_CS_ERR                                                             \
	(RALINK_FRAME_ENGINE_BASE + SDM_RELATED +                              \
	 0x110) //Switch DMA rx checksum error count

#define INTERNAL_LOOPBACK_ENABLE  1
#define INTERNAL_LOOPBACK_DISABLE 0

#define TOUT_LOOP 1000
#define ENABLE	  1
#define DISABLE	  0

/*=========================================
      PDMA RX Descriptor Format define
=========================================*/

//-------------------------------------------------
typedef struct _PDMA_RXD_INFO1_ PDMA_RXD_INFO1_T;

struct _PDMA_RXD_INFO1_ {
	unsigned int PDP0;
};
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO2_ PDMA_RXD_INFO2_T;

struct _PDMA_RXD_INFO2_ {
	unsigned int PLEN1 : 14;
	unsigned int LS1 : 1;
	unsigned int UN_USED : 1;
	unsigned int PLEN0 : 14;
	unsigned int LS0 : 1;
	unsigned int DDONE_bit : 1;
};
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO3_ PDMA_RXD_INFO3_T;

struct _PDMA_RXD_INFO3_ {
	unsigned int PDP1;
};
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO4_ PDMA_RXD_INFO4_T;

struct _PDMA_RXD_INFO4_ {
	unsigned int FOE_Entry : 14;
	unsigned int FVLD : 1;
	unsigned int UN_USE1 : 1;
	unsigned int AI : 8;
	unsigned int SP : 3;
	unsigned int AIS : 1;
	unsigned int L4F : 1;
	unsigned int IPF : 1;
	unsigned int L4FVLD_bit : 1;
	unsigned int IPFVLD_bit : 1;
};

struct PDMA_rxdesc {
	PDMA_RXD_INFO1_T rxd_info1;
	PDMA_RXD_INFO2_T rxd_info2;
	PDMA_RXD_INFO3_T rxd_info3;
	PDMA_RXD_INFO4_T rxd_info4;
};
/*=========================================
      PDMA TX Descriptor Format define
=========================================*/
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO1_ PDMA_TXD_INFO1_T;

struct _PDMA_TXD_INFO1_ {
	unsigned int SDP0;
};
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO2_ PDMA_TXD_INFO2_T;

struct _PDMA_TXD_INFO2_ {
	unsigned int SDL1 : 14;
	unsigned int LS1_bit : 1;
	unsigned int BURST_bit : 1;
	unsigned int SDL0 : 14;
	unsigned int LS0_bit : 1;
	unsigned int DDONE_bit : 1;
};
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO3_ PDMA_TXD_INFO3_T;

struct _PDMA_TXD_INFO3_ {
	unsigned int SDP1;
};
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO4_ PDMA_TXD_INFO4_T;

struct _PDMA_TXD_INFO4_ {
	unsigned int VIDX : 4;
	unsigned int VPRI : 3;
	unsigned int INSV : 1;
	unsigned int SIDX : 4;
	unsigned int INSP : 1;
	unsigned int UN_USE3 : 3;
	unsigned int QN : 3;
	unsigned int UN_USE2 : 5;
	unsigned int PN : 3;
	unsigned int UN_USE1 : 2;
	unsigned int TUI_CO : 3;
};

struct PDMA_txdesc {
	PDMA_TXD_INFO1_T txd_info1;
	PDMA_TXD_INFO2_T txd_info2;
	PDMA_TXD_INFO3_T txd_info3;
	PDMA_TXD_INFO4_T txd_info4;
};

static struct PDMA_txdesc tx_ring0_cache[NUM_TX_DESC]
	__attribute__((aligned(32))); /* TX descriptor ring         */
static struct PDMA_rxdesc rx_ring_cache[NUM_RX_DESC]
	__attribute__((aligned(32))); /* RX descriptor ring         */

static int
	rx_dma_owner_idx0; /* Point to the next RXD DMA wants to use in RXD Ring#0.  */
static int
	rx_wants_alloc_idx0; /* Point to the next RXD CPU wants to allocate to RXD Ring #0. */
static int
	tx_cpu_owner_idx0; /* Point to the next TXD in TXD_Ring0 CPU wants to use */
static volatile struct PDMA_rxdesc *rx_ring;
static volatile struct PDMA_txdesc *tx_ring0;

static char rxRingSize;
static char txRingSize;

static int rt2880_eth_init(struct eth_device *dev, bd_t *bis);
static int rt2880_eth_send(struct eth_device *dev, volatile void *packet,
			   int length);
static int rt2880_eth_recv(struct eth_device *dev);
void rt2880_eth_halt(struct eth_device *dev);

int mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data);
int mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data);

void rt305x_esw_init(void);

static int rt2880_eth_setup(struct eth_device *dev);
static int rt2880_eth_initd;
char console_buffer[CFG_CBSIZE]; /* console I/O buffer	*/

#define phys_to_bus(a) (a & 0x1FFFFFFF)

#define PCI_WAIT_INPUT_CHAR(ch) while ((ch = getc()) == 0)

struct eth_device *rt2880_pdev;

volatile uchar *PKT_HEADER_Buf; // = (uchar *)CFG_EMBEDED_SRAM_SDP0_BUF_START;
static volatile uchar
	PKT_HEADER_Buf_Pool[(PKTBUFSRX * PKTSIZE_ALIGN) + PKTALIGN];
extern volatile uchar *NetTxPacket; /* THE transmit packet			*/

#define PIODIR_R      (RALINK_PIO_BASE + 0X24)
#define PIODATA_R     (RALINK_PIO_BASE + 0X20)
#define PIODIR3924_R  (RALINK_PIO_BASE + 0x4c)
#define PIODATA3924_R (RALINK_PIO_BASE + 0x48)

void START_ETH(struct eth_device *dev)
{
	s32 omr;
	omr = RALINK_REG(PDMA_GLO_CFG);
	udelay(100);
	omr |= TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN;

	RALINK_REG(PDMA_GLO_CFG) = omr;
	udelay(500);
}

void STOP_ETH(struct eth_device *dev)
{
	s32 omr;
	omr = RALINK_REG(PDMA_GLO_CFG);
	udelay(100);
	omr &= ~(TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
	RALINK_REG(PDMA_GLO_CFG) = omr;
	udelay(500);
}

int rt2880_eth_initialize(bd_t *bis)
{
	struct eth_device *dev;
	u32 regValue;

	rt305x_esw_init();

	if (!(dev = (struct eth_device *)malloc(sizeof *dev))) {
		printf("Failed to allocate memory\n");
		return 0;
	}

	memset(dev, 0, sizeof(*dev));

	sprintf(dev->name, "Eth0 (10/100-M)");

	dev->iobase = RALINK_FRAME_ENGINE_BASE;
	dev->init   = rt2880_eth_init;
	dev->halt   = rt2880_eth_halt;
	dev->send   = rt2880_eth_send;
	dev->recv   = rt2880_eth_recv;

	eth_register(dev);
	rt2880_pdev = dev;

	rt2880_eth_initd = 0;
	PKT_HEADER_Buf	 = PKT_HEADER_Buf_Pool;
	NetTxPacket	 = NULL;
	rx_ring	 = (struct PDMA_rxdesc *)KSEG1ADDR((ulong)&rx_ring_cache[0]);
	tx_ring0 = (struct PDMA_txdesc *)KSEG1ADDR((ulong)&tx_ring0_cache[0]);

	//set clock resolution

	//extern unsigned long mips_bus_feq;
	u32 mips_bus_feq;
	mtk_sys_clocks(NULL, NULL, &mips_bus_feq, NULL, NULL);
	regValue = le32_to_cpu(
		*(volatile u_long *)(RALINK_FRAME_ENGINE_BASE + 0x0008));
	regValue |= ((mips_bus_feq / 1000000) << 8);
	*((volatile u_long *)(RALINK_FRAME_ENGINE_BASE + 0x0008)) =
		cpu_to_le32(regValue);

	return 1;
}

static int rt2880_eth_init(struct eth_device *dev, bd_t *bis)
{
	if (rt2880_eth_initd == 0) {
		rt2880_eth_setup(dev);
	} else {
		START_ETH(dev);
	}

	rt2880_eth_initd = 1;
	return (1);
}

int isDMABusy(struct eth_device *dev)
{
	u32 reg;

	reg = RALINK_REG(PDMA_GLO_CFG);

	if ((reg & RX_DMA_BUSY)) {
		return 1;
	}

	if ((reg & TX_DMA_BUSY)) {
		printf("\n  TX_DMA_BUSY !!! ");
		return 1;
	}
	return 0;
}

void mt7628_ephy_init(void)
{
	int i;

	mii_mgr_write(0, 31, 0x2000); //change G2 page
	mii_mgr_write(0, 26, 0x0000);

	for (i = 0; i < 5; i++) {
		mii_mgr_write(i, 31, 0x8000); //change L0 page
		mii_mgr_write(i, 0, 0x3100);
		/*EEE disable*/

		mii_mgr_write(i, 30, 0xa000);
		mii_mgr_write(i, 31, 0xa000); // change L2 page
		mii_mgr_write(i, 16, 0x0606);
		mii_mgr_write(i, 23, 0x0f0e);
		mii_mgr_write(i, 24, 0x1610);
		mii_mgr_write(i, 30, 0x1f15);
		mii_mgr_write(i, 28, 0x6111);
	}

	//100Base AOI setting
	mii_mgr_write(0, 31, 0x5000); //change G5 page
	mii_mgr_write(0, 19, 0x004a);
	mii_mgr_write(0, 20, 0x015a);
	mii_mgr_write(0, 21, 0x00ee);
	mii_mgr_write(0, 22, 0x0033);
	mii_mgr_write(0, 23, 0x020a);
	mii_mgr_write(0, 24, 0x0000);
	mii_mgr_write(0, 25, 0x024a);
	mii_mgr_write(0, 26, 0x035a);
	mii_mgr_write(0, 27, 0x02ee);
	mii_mgr_write(0, 28, 0x0233);
	mii_mgr_write(0, 29, 0x000a);
	mii_mgr_write(0, 30, 0x0000);
	/* Fix EPHY idle state abnormal behavior */
	mii_mgr_write(0, 31, 0x4000); //change G4 page
	mii_mgr_write(0, 29, 0x000d);
	mii_mgr_write(0, 30, 0x0500);
}

void rt305x_esw_init(void)
{
	u32 i;

	/*
	 * FC_RLS_TH=200, FC_SET_TH=160
	 * DROP_RLS=120, DROP_SET_TH=80
	 */
	RALINK_REG(RALINK_ETH_SW_BASE + 0x0008) = 0xC8A07850;
	RALINK_REG(RALINK_ETH_SW_BASE + 0x00E4) = 0x00000000;
	RALINK_REG(RALINK_ETH_SW_BASE + 0x0014) = 0x00405555;
	RALINK_REG(RALINK_ETH_SW_BASE + 0x0090) = 0x00007f7f;
	RALINK_REG(RALINK_ETH_SW_BASE + 0x0098) = 0x00007f7f; //disable VLAN
	RALINK_REG(RALINK_ETH_SW_BASE + 0x00CC) = 0x0002500c;

	// place ethernet ports into separate vlans

	// set up vlan ids
	RALINK_REG(MTK_SWITCH_PVIDC0) = (1 & 0x0FFF) | ((2 & 0x0FFF) << 12);
	RALINK_REG(MTK_SWITCH_PVIDC1) = (3 & 0x0FFF) | ((4 & 0x0FFF) << 12);
	RALINK_REG(MTK_SWITCH_PVIDC2) = (5 & 0x0FFF) | ((6 & 0x0FFF) << 12);

	// set up vlan ports
	RALINK_REG(MTK_SWITCH_VMSC0) = 0b01111111;
	RALINK_REG(MTK_SWITCH_VMSC0) |= (0b01000010 << 8);
	RALINK_REG(MTK_SWITCH_VMSC0) |= (0b01000100 << 16);
	RALINK_REG(MTK_SWITCH_VMSC0) |= (0b01001000 << 24);

	RALINK_REG(MTK_SWITCH_VMSC1) = 0b01010000;
	RALINK_REG(MTK_SWITCH_VMSC1) |= (0b01100000 << 8);

	RALINK_REG(RALINK_ETH_SW_BASE + 0x009C) =
		0x0008a301; //hashing algorithm=XOR48, aging interval=300sec

	RALINK_REG(RALINK_ETH_SW_BASE + 0x008C) = 0x02404040;

	RALINK_REG(RALINK_ETH_SW_BASE + 0x00C8) =
		0x3f502b28; //Ext PHY Addr=0x1F
	RALINK_REG(RALINK_ETH_SW_BASE + 0x0084) = 0x00000000;
	RALINK_REG(RALINK_ETH_SW_BASE + 0x0110) =
		0x7d000000; //1us cycle number=125 (FE's clock=125Mhz)

#define RSTCTRL_EPHY_RST      (1 << 24)
#define MT7628_EPHY_EN	      (0x1f << 16)
#define MT7628_P0_EPHY_AIO_EN (1 << 16)
	/*TODO: Init MT7628 ASIC PHY HERE*/
	i = RALINK_REG(RT2880_AGPIOCFG_REG);

	i = i & ~(MT7628_EPHY_EN);

	RALINK_REG(RT2880_AGPIOCFG_REG) = i;

	// reset phy
	i			       = RALINK_REG(RT2880_RSTCTRL_REG);
	i			       = i | RSTCTRL_EPHY_RST;
	RALINK_REG(RT2880_RSTCTRL_REG) = i;
	i			       = i & ~(RSTCTRL_EPHY_RST);
	RALINK_REG(RT2880_RSTCTRL_REG) = i;
	i			       = RALINK_REG(RALINK_SYSCTL_BASE + 0x64);

	i &= 0xf003f003;
	i |= 0b0101010101 << 2; // set ethernet LEDs as GPIO
	RALINK_REG(RALINK_SYSCTL_BASE + 0x64) = i;

	udelay(5000);
	mt7628_ephy_init();
}

static int rt2880_eth_setup(struct eth_device *dev)
{
	u32 i;
	u32 regValue;
	u16 wTmp;

	while (1)
		if (!isDMABusy(dev))
			break;

	/* Set MAC address. */
	wTmp	 = (u16)dev->enetaddr[0];
	regValue = (wTmp << 8) | dev->enetaddr[1];

	RALINK_REG(SDM_MAC_ADRH) = regValue;

	wTmp	 = (u16)dev->enetaddr[2];
	regValue = (wTmp << 8) | dev->enetaddr[3];
	regValue = regValue << 16;
	wTmp	 = (u16)dev->enetaddr[4];
	regValue |= (wTmp << 8) | dev->enetaddr[5];
	RALINK_REG(SDM_MAC_ADRL) = regValue;

	for (i = 0; i < NUM_RX_DESC; i++) {
		rx_ring[i].rxd_info2.DDONE_bit = 0;

		{
			rx_ring[i].rxd_info2.LS0 = 1;
			rx_ring[i].rxd_info1.PDP0 =
				cpu_to_le32(phys_to_bus((u32)NetRxPackets[i]));
			flush_cache((u32)NetRxPackets[i], PKTSIZE_ALIGN);
		}
	}

	for (i = 0; i < NUM_TX_DESC; i++) {
		tx_ring0[i].txd_info2.LS0_bit	= 1;
		tx_ring0[i].txd_info2.DDONE_bit = 1;
		/* PN:
		 *  0:CPU
		 *  1:GE1
		 *  2:GE2 (for RT2883)
		 *  6:PPE
		 *  7:Discard
		 */
		tx_ring0[i].txd_info4.PN = 1;
	}
	rxRingSize = NUM_RX_DESC;
	txRingSize = NUM_TX_DESC;

	rx_dma_owner_idx0   = 0;
	rx_wants_alloc_idx0 = (NUM_RX_DESC - 1);
	tx_cpu_owner_idx0   = 0;

	regValue = RALINK_REG(PDMA_GLO_CFG);
	udelay(100);

	{
		regValue &= 0x0000FFFF;

		RALINK_REG(PDMA_GLO_CFG) = regValue;
		udelay(500);
		regValue = RALINK_REG(PDMA_GLO_CFG);
	}

	/* Tell the adapter where the TX/RX rings are located. */
	RALINK_REG(RX_BASE_PTR0) = phys_to_bus((u32)&rx_ring[0]);

	//printf("\n rx_ring=%08X ,RX_BASE_PTR0 = %08X \n",&rx_ring[0],RALINK_REG(RX_BASE_PTR0));
	RALINK_REG(TX_BASE_PTR0) = phys_to_bus((u32)&tx_ring0[0]);

	//printf("\n tx_ring0=%08X, TX_BASE_PTR0 = %08X \n",&tx_ring0[0],RALINK_REG(TX_BASE_PTR0));

	RALINK_REG(RX_MAX_CNT0) = cpu_to_le32((u32)NUM_RX_DESC);
	RALINK_REG(TX_MAX_CNT0) = cpu_to_le32((u32)NUM_TX_DESC);

	RALINK_REG(TX_CTX_IDX0)	 = cpu_to_le32((u32)tx_cpu_owner_idx0);
	RALINK_REG(PDMA_RST_IDX) = cpu_to_le32((u32)RST_DTX_IDX0);

	RALINK_REG(RX_CALC_IDX0) = cpu_to_le32((u32)(NUM_RX_DESC - 1));
	RALINK_REG(PDMA_RST_IDX) = cpu_to_le32((u32)RST_DRX_IDX0);

	udelay(500);
	START_ETH(dev);

	return 1;
}

static int rt2880_eth_send(struct eth_device *dev, volatile void *packet,
			   int length)
{
	int status = -1;
	int i;
	int retry_count = 0, temp;
	char *p		= (char *)KSEG1ADDR(packet);

Retry:
	if (retry_count > 10) {
		return (status);
	}

	if (length <= 0) {
		printf("%s: bad packet size: %d\n", dev->name, length);
		return (status);
	}

#define PADDING_LENGTH 60
	if (length < PADDING_LENGTH) {
		//	print_packet(packet,length);
		for (i = 0; i < PADDING_LENGTH - length; i++) {
			p[length + i] = 0;
		}
		length = PADDING_LENGTH;
	}

	for (i = 0; tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit == 0; i++)

	{
		if (i >= TOUT_LOOP) {
			printf("%s: TX DMA is Busy !! TX desc is Empty!\n",
			       dev->name);
			goto Done;
		}
	}
	//dump_reg();

	temp = RALINK_REG(TX_DTX_IDX0);

	if (temp == (tx_cpu_owner_idx0 + 1) % NUM_TX_DESC) {
		puts(" @ ");
		goto Done;
	}

	flush_cache((u32)packet, length);
	tx_ring0[tx_cpu_owner_idx0].txd_info1.SDP0 =
		cpu_to_le32(phys_to_bus((u32)packet));
	tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0 = length;

	tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit = 0;
	status						= length;

	tx_cpu_owner_idx0	= (tx_cpu_owner_idx0 + 1) % NUM_TX_DESC;
	RALINK_REG(TX_CTX_IDX0) = cpu_to_le32((u32)tx_cpu_owner_idx0);

	return status;
Done:
	udelay(500);
	retry_count++;
	goto Retry;
}

static int rt2880_eth_recv(struct eth_device *dev)
{
	int length	       = 0;
	int inter_loopback_cnt = 0;
	u32 *rxd_info;

	for (;;) {
		rxd_info =
			(u32 *)KSEG1ADDR(&rx_ring[rx_dma_owner_idx0].rxd_info2);

		if ((*rxd_info & BIT(31)) == 0) {
			break;
		}

		udelay(1);
		length = rx_ring[rx_dma_owner_idx0].rxd_info2.PLEN0;

		if (length == 0) {
			printf("\n Warring!! Packet Length has error !!,In normal mode !\n");
		}

		if (rx_ring[rx_dma_owner_idx0].rxd_info4.SP ==
		    0) { // Packet received from CPU port
			printf("\n Normal Mode,Packet received from CPU port,plen=%d \n",
			       length);
			//print_packet((void *)KSEG1ADDR(NetRxPackets[rx_dma_owner_idx0]),length);
			inter_loopback_cnt++;
			length = inter_loopback_cnt; //for return
		} else {
			NetReceive((void *)KSEG1ADDR(
					   NetRxPackets[rx_dma_owner_idx0]),
				   length);
			flush_cache((u32)NetRxPackets[rx_dma_owner_idx0],
				    PKTSIZE_ALIGN);
		}

		rxd_info  = (u32 *)&rx_ring[rx_dma_owner_idx0].rxd_info4;
		*rxd_info = 0;

		rxd_info  = (u32 *)&rx_ring[rx_dma_owner_idx0].rxd_info2;
		*rxd_info = 0;
		rx_ring[rx_dma_owner_idx0].rxd_info2.LS0 = 1;

		/* Tell the adapter where the TX/RX rings are located. */
		RALINK_REG(RX_BASE_PTR0) = phys_to_bus((u32)&rx_ring[0]);

		//udelay(10000);
		/*  Move point to next RXD which wants to alloc*/
		RALINK_REG(RX_CALC_IDX0) = cpu_to_le32((u32)rx_dma_owner_idx0);

		/* Update to Next packet point that was received.
		 */
		rx_dma_owner_idx0 = (rx_dma_owner_idx0 + 1) % NUM_RX_DESC;

		//printf("\n ************************************************* \n");
		//printf("\n RX_CALC_IDX0=%d \n", RALINK_REG(RX_CALC_IDX0));
		//printf("\n RX_DRX_IDX0 = %d \n",RALINK_REG(RX_DRX_IDX0));
		//printf("\n ************************************************* \n");
	}
	return length;
}

void rt2880_eth_halt(struct eth_device *dev)
{
	STOP_ETH(dev);
}


void rt2880_port_enable(unsigned port, unsigned value)
{
	uint32_t reg;

	if (port > 5) {
		return;
	}

	reg = (1 << port);
	reg <<= MTK_SWITCH_POC0_DIS_PORT_SHIFT;

	if (value) {
		RALINK_REG(MTK_SWITCH_POC0) &= ~reg;
	} else {
		RALINK_REG(MTK_SWITCH_POC0) |= reg;
	}
}
