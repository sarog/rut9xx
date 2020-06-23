/* dk_pci_bus.c - contains functions specific to pci bus */

#if defined(OWL_PB42) || defined(PYTHON_EMU)
	#include <linux/pci.h>
	#include <linux/module.h>
	#include <linux/version.h>
	#include "dk.h"
	#include "client.h"
	#define ATHEROS_VENDOR_ID 0x168c
	#define MAX_CFG_OFFSET	256
	#define PCIE_1_LINK_ADDRESS 0xb80f0018
	#define PCIE_2_LINK_ADDRESS 0xb8280018

	//########################################################################
	#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)//z
		static INT32 dk_pci_enable_wake
			(
			 	struct pci_dev *dev,
				UINT32 state,
				INT32 enable
			)
			{//dk_pci_enable_wake
			#ifdef DK_DEBUG
					printk("DK::Pci enable wake \n");
			#endif
					return 0;
		}//dk_pci_enable_wake
	#endif//z
	//########################################################################
	static INT32 dk_pci_probe
	(
	 	struct pci_dev *dev,
		const struct pci_device_id *id
	)
	{//dk_pci_probe

		INT32 error;
		A_UINT_PTR baseaddr[MAX_BARS];
		UINT32 len[MAX_BARS];
		UINT32 irq;
		#if  defined(PYTHON_EMU)//0
		    UINT32 *addr;
			INT32 pci_ret_val;
		#endif//0
		#ifndef PYTHON_EMU
			UINT8  csz;
			UINT32 val;
		#endif//1
		UINT32 iIndex, numBars;
		#ifdef DK_DEBUG
		    UINT32 device_id, vendor_id;
		#endif//1
		INT8 ret_val;
		UINT32 sIndex = WMAC_FN_DEV_START_NUM;
	
		#ifdef DK_DEBUG//2
			printk("DK::Pci probe \n");
		#endif//2
		error = pci_enable_device(dev);
		if (error != 0) {//3
				printk(KERN_ERR "DK:: pci_enable_device failed \n");
				return error;
		}//3
		
//		#ifndef PYTHON_EMU//4
//		    pci_read_config_byte(dev, PCI_CACHE_LINE_SIZE, &csz);
//		    if (csz == 0) {//5
//		        /*
//		         * Linux 2.4.18 (at least) writes the cache line size
//		         * register as a 16-bit wide register which is wrong.
//		         * We must have this setup properly for rx buffer
//		         * DMA to work so force a reasonable value here if it
//		         * comes up zero.
//		         */
//		        csz = L1_CACHE_BYTES / sizeof(UINT32);
//		        pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, csz);
//		    }//5
//		    /*
//		     * The default setting of latency timer yields poor results,
//		     * set it to the value used by other systems.  It may be worth
//		     * tweaking this setting more.
//		     */
//		    pci_write_config_byte(dev, PCI_LATENCY_TIMER, 0xa8);
//		    pci_set_master(dev);
//		    /*
//		     * Disable the RETRY_TIMEOUT register (0x41) to keep
//		     * PCI Tx retries from interfering with C3 CPU state.
//		     *
//		     * Code taken from ipw2100 driver - jg
//		     */
//		    pci_read_config_dword(dev, 0x40, &val);
//		    if ((val & 0x0000ff00) != 0) {//6
//		    	pci_write_config_dword(dev, 0x40, val & 0xffff00ff);
//		    }//6
//		#endif//4
		
		#ifdef PYTHON_EMU//0
			
			#define CHIP_ID_LOCATION 0xb8060090
			#ifndef OCTEON//1
			        INT32 chip_rev_id=0;
			#endif//1
			
			#ifndef OWL_PB42//2
			        get_chip_id(0,CHIP_ID_LOCATION,4,&chip_rev_id);
			#endif//2
			#define CHIP_REV_ID_SCORPION_A 0x013 // last nibble is for Chip revision which is ignored
			#define CHIP_REV_ID_SCORPION_B 0x113 // last nibble is for Chip revision which is ignored
			if (((chip_rev_id& 0xfff0)>>4 == CHIP_REV_ID_SCORPION_A)||((chip_rev_id& 0xfff0)>>4 == CHIP_REV_ID_SCORPION_B)){//5 // Scorpion packages A and B (PCIE_2_LINK_ADDRESS is only valid for Scorpion)
			    addr = (UINT32 *)(PCIE_2_LINK_ADDRESS);
			    pci_ret_val = readl(addr);
				if(pci_ret_val==7){//4
			   		pci_write_config_dword(dev,0x10,0x1201ffff);
				}//4
			}//5
			addr = (UINT32 *)(PCIE_1_LINK_ADDRESS);
			pci_ret_val = readl(addr);
			if(pci_ret_val==7){//6
			   	pci_write_config_dword(dev,0x10,0xffff);
			}//6
		#endif//0
		
		for (iIndex=0; iIndex<MAX_BARS; iIndex++) {//0
			baseaddr[iIndex] = pci_resource_start(dev,iIndex);
			printk(KERN_ERR" Base Phsycal address :0x%08lx\n", baseaddr[iIndex]);
			len[iIndex] = pci_resource_len(dev,iIndex);
			if (len[iIndex] == 0){//1
				break;
			}//1
		}//0

		numBars = iIndex;

		#ifdef DK_DEBUG//0
			printk("DK::num bars = %d\n", numBars);
			(void) pci_read_config_dword(dev, PCI_DEVICE_ID, &device_id);
			(void) pci_read_config_dword(dev, PCI_VENDOR_ID, &vendor_id);
		    printk("DK::Vendor Id=%x:Device id = %x\n", vendor_id, device_id);
		#endif//0

		irq = dev->irq;

//		#ifndef PYTHON_EMU//0
//		    for (iIndex=0; iIndex<numBars; iIndex++) {
//			   pci_write_config_dword(dev,PCI_BASE_ADDRESS_0 + (iIndex *4), baseaddr[iIndex]);
//			   pci_write_config_byte(dev,PCI_INTERRUPT_LINE, irq);
//		    }
//		#endif//0

		(void) pci_read_config_byte(dev, 0xb, &ret_val); // Get the class code
		if (ret_val == NETWORK_CLASS) {//a
				sIndex = WMAC_FN_DEV_START_NUM;
		}//a

		if (ret_val == SIMPLE_COMM_CLASS) {//b
				sIndex = UART_FN_DEV_START_NUM;
		}//b

		printk("Class code = %d:start search index=%d\n", ret_val, sIndex);
		
		if (add_client(dev,baseaddr,len,irq, numBars, sIndex,1) < 0) {//c
			printk(KERN_ERR "DK:: unable to add client \n");
			#if LINUX_VERSION_CODE > 132098//0
				pci_disable_device(dev);
			#endif//0
			return -ENODEV;
		}//c
		
		// #if 0
		// 	virmem = (UINT32)ioremap(pci_resource_start(dev, 0), pci_resource_len(dev,0));
		
		//         printk(KERN_ERR"IOREMAP Addr: 0x%x\n", virmem);
		
		// 	writel(0x1, (UINT32*)((UINT8*)virmem+0x704c));
		// 	writel(0x0, (UINT32*)((UINT8*)virmem+0x7040));
		// 	writel(0x5, (UINT32*)((UINT8*)virmem+0x7040));
		// 	printk(KERN_ERR" Address: 0x%x, Status****: 0x%x\n", (UINT32)((UINT8*)virmem+0x7044), readl((UINT32*)((UINT8*)virmem+0x7044)));
		// #endif
		return 0;
	}//dk_pci_probe

	//########################################################################

	static VOID dk_pci_remove
	(
		struct pci_dev *dev
	)
	{//dk_pci_remove
		#ifdef DK_DEBUG//0
			printk("DK::Pci remove \n");
		#endif//0

		remove_client(dev);

		#if LINUX_VERSION_CODE > 132098//1
			pci_disable_device(dev);
		#endif//1
		
		return;
	}//dk_pci_remove

	//########################################################################

	#if LINUX_VERSION_CODE >= 132623
		static INT32 dk_pci_suspend
		(
		 	struct pci_dev *dev,
			pm_message_t state
		)
	#elif (LINUX_VERSION_CODE > 132098) && (LINUX_VERSION_CODE < 132623)
		static INT32 dk_pci_suspend
		(
		 	struct pci_dev *dev,
			UINT32 state
		)
	#else
		static VOID dk_pci_suspend
		(
		 	struct pci_dev *dev
		)
	#endif
	{//dk_pci_suspend
		#ifdef DK_DEBUG//0
				printk("DK::Pci suspend \n");
		#endif//0

		#if LINUX_VERSION_CODE > 132098//1
				return 0;
		#else
			return;
		#endif//1
	}//dk_pci_suspend

	//########################################################################

	#if LINUX_VERSION_CODE > 132098
		static INT32 dk_pci_resume
		(
			struct pci_dev *dev
		)
	#else
		static VOID dk_pci_resume
		(
			struct pci_dev *dev
		)
	#endif
	{//dk_pci_resume
		#ifdef DK_DEBUG//0
			printk("DK::Pci resume \n");
		#endif//0

		#if LINUX_VERSION_CODE > 132098//1
			return 0;
		#else
			return;
		#endif//1
	}//dk_pci_resume
		
//	static struct pci_device_id __devinitdata dk_id_tbl[] = {//a
	static struct pci_device_id dk_id_tbl[] = {//a
		{ATHEROS_VENDOR_ID, 0x0011, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"MAUI"},
		{ATHEROS_VENDOR_ID, 0x0012, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"OAHU"},
		{ATHEROS_VENDOR_ID, 0x0013, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"VENICE"},
		{ATHEROS_VENDOR_ID, 0x0014, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"VENICE_DERBY"},
		{ATHEROS_VENDOR_ID, 0xff16, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"GRIFFIN_MAC"},
		{ATHEROS_VENDOR_ID, 0x0023, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"OWL"},
		{ATHEROS_VENDOR_ID, 0x0026, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"NALA"},
		{ATHEROS_VENDOR_ID, 0x0027, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"SOWL"},
		{ATHEROS_VENDOR_ID, 0xff1c, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"SOWL"},
		{ATHEROS_VENDOR_ID, 0x0028, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"SOWL_PCIE"},
		{ATHEROS_VENDOR_ID, 0x0029, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"MERLIN"},
		{ATHEROS_VENDOR_ID, 0x002a, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"MERLIN_PCIE"},
		{ATHEROS_VENDOR_ID, 0x002b, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"KITE_PCIE"},
		{ATHEROS_VENDOR_ID, 0xff1d, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"OWL"},
		{ATHEROS_VENDOR_ID, 0x001d, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"NALA"},
		{ATHEROS_VENDOR_ID, 0xff1a, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"NALA"},
	    {ATHEROS_VENDOR_ID, 0x002d, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"KIWI"},
	    {ATHEROS_VENDOR_ID, 0x002e, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"KIWI_PCIE"},
	    {ATHEROS_VENDOR_ID, 0x0030, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"OSPREY"},
	    {ATHEROS_VENDOR_ID, 0x0033, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"PEACOCK"},
	    {ATHEROS_VENDOR_ID, 0xabcd, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"OSPREY"},
		{0,}
	};//a
		
	#ifdef DK_UART//b
		static struct pci_device_id __devinitdata dk_uart_id_tbl[] = {
			{ATHEROS_VENDOR_ID, 0xff96, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)"GRIFFIN_UART"},
			{0,}
		};
	#endif//b
		
	static struct pci_driver dkpci_driver = {
			name:	"dkkernel",
			id_table: dk_id_tbl,
			probe: dk_pci_probe,
			remove: dk_pci_remove,
			suspend: dk_pci_suspend,
			resume: dk_pci_resume,
		#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
			enable_wake: dk_pci_enable_wake
		#endif
	};
		
	#ifdef DK_UART
		static struct pci_driver dkuart_pci_driver = {
			name:	"dkuartkernel",
			id_table: dk_uart_id_tbl,
			probe: dk_pci_probe,
			remove: dk_pci_remove,
			suspend: dk_pci_suspend,
			resume: dk_pci_resume,
		};	
	#endif

	//########################################################################

	INT32 bus_module_init
	(
	 	VOID
	)
	{//bus_module_init
		int status;
		#ifdef DK_DEBUG
			printk("DK::Bus module init  \n");
		#endif // DK_DEBUG

		#ifdef MODULE
			#ifdef DK_DEBUG
				printk("DK::MODULE\n");
			#endif // DK_DEBUG
			#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
				status = pci_module_init(&dkpci_driver);
			#else
				status = pci_register_driver(&dkpci_driver);
			#endif
			printk("DK::bus_module_init:status=%d from dkpci_driver\n", status);
			#ifdef DK_UART
				status |= pci_module_init(&dkuart_pci_driver);
				printk("DK::bus_module_init:status=%d from dkuart_pci_driver\n", status);
			#endif
			return status;
		#else
			return pci_register_driver(&dkpci_driver);
		#endif // MODULE
	}//bus_module_init
	//########################################################################

	VOID bus_module_exit
	(
	 	VOID
	)
	{//bus_module_exit
		#ifdef DK_DEBUG
			printk("DK::Bus module exit  \n");
		#endif // DK_DEBUG
			pci_unregister_driver(&dkpci_driver);
		#ifdef DK_UART
			pci_unregister_driver(&dkuart_pci_driver);
		#endif
	}//bus_module_exit

	#ifdef MODULE
		MODULE_DEVICE_TABLE(pci,dk_id_tbl);
	#endif

	//########################################################################

	INT32 bus_dev_init
	(
	 	void  *bus_dev
	)
	{//bus_dev_init
		struct pci_dev *dev;
		UINT32 baseaddr;
		UINT32 irq;
		
		dev = (struct pci_dev *)bus_dev;
		pci_set_master(dev);
		
		baseaddr = pci_resource_start(dev,0);
		irq = dev->irq;
		
		/*
		 * Program the base address and irq as the device may
		 * be hotplugged without aware of the kernel
		 */
		#ifndef PYTHON_EMU
			pci_write_config_dword(dev,PCI_BASE_ADDRESS_0, baseaddr);
			pci_write_config_byte(dev,PCI_INTERRUPT_LINE, irq);
		#endif
		return 0;
	}//bus_dev_init
	//########################################################################
	VOID bus_dev_exit
		(
		 	void  *bus_dev
		)
		{//bus_dev_exit
			return;
	}//bus_dev_exit
	//########################################################################
	INT32 bus_cfg_read
	(
	 	void  *bus_dev,
	 	INT32 offset,
		INT32 size,
		INT32 *ret_val
	)
	{//bus_cfg_read
		struct pci_dev *dev;
		int ret =  -1;
	
		dev = (struct pci_dev *)bus_dev;
	
		if (size < MAX_CFG_OFFSET) {//0
			switch (size) {//1
				case 1:
					ret = pci_read_config_byte(dev,offset, (INT8 *)ret_val);
					break;
				case 2:
					offset = offset & 0xfe;
					ret = pci_read_config_word(dev,offset, (INT16 *)ret_val);
					break;
				case 4:
					offset = offset & 0xfc;
					ret = pci_read_config_dword(dev,offset, ret_val);
					break;
				default:
					break;
			}//1
		}//0
		return ret;
	}//bus_cfg_read
	//########################################################################
	INT32 bus_cfg_write
	(
	 	void  *bus_dev,
	 	INT32 offset,
		INT32 size,
		INT32 val
	)
	{//bus_cfg_write
		struct pci_dev *dev;
		int ret =  -1;
		dev = (struct pci_dev *)bus_dev;
		if (size < MAX_CFG_OFFSET) {//0
			switch (size) {//1
				case 1:
					ret = pci_write_config_byte(dev,offset, val);
					break;
				case 2:
					offset = offset & 0xfe;
					ret = pci_write_config_word(dev,offset, val);
					break;
				case 4:
					offset = offset & 0xfc;
					ret = pci_write_config_dword(dev,offset,val);
					break;
				default:
					break;
			}//1
		}//0
		return ret;
	}//bus_cfg_write

#endif
