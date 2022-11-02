#ifndef _MTK_SERIAL_H_
#define _MTK_SERIAL_H_

/* FIXME: Only this does work for u-boot... find out why... [RS] */
#undef io_p2v
#undef __REG
#ifndef __ASSEMBLY__
#define io_p2v(PhAdd) (PhAdd)
#define __REG(x)      (*((volatile u32 *)io_p2v(x)))
#define __REG2(x, y)  (*(volatile u32 *)((u32)&__REG(x) + (y)))
#else
#define __REG(x) (x)
#endif

/*
 * UART registers
 */
#define RT2880_UART1	   0x0C00 /* UART Lite */
#define RT2880_UART2	   0x0D00 /* UART Lite */
#define RT2880_UART3	   0x0E00 /* UART Lite */
#define CFG_RT2880_CONSOLE RT2880_UART1

#define RT2880_UART_RBR_OFFSET 0x00
#define RT2880_UART_TBR_OFFSET 0x00
#define RT2880_UART_IER_OFFSET 0x04
#define RT2880_UART_IIR_OFFSET 0x08
#define RT2880_UART_FCR_OFFSET 0x08
#define RT2880_UART_LCR_OFFSET 0x0C
#define RT2880_UART_LSR_OFFSET 0x14
#define RT2880_UART_DLL_OFFSET 0x00
#define RT2880_UART_DLM_OFFSET 0x04

#define LCR_DLAB (1 << 7) /* Divisor Latch Access Bit */
#define LCR_WLS1 (1 << 1) /* Word Length Select */
#define LCR_WLS0 (1 << 0) /* Word Length Select */

#define LSR_TEMT (1 << 6) /* Transmitter Empty */
#define LSR_DR	 (1 << 0) /* Data Ready */

#define RBR(x) __REG(MTK_SYSCTL_BASE + (x) + RT2880_UART_RBR_OFFSET)
#define TBR(x) __REG(MTK_SYSCTL_BASE + (x) + RT2880_UART_TBR_OFFSET)
#define IER(x) __REG(MTK_SYSCTL_BASE + (x) + RT2880_UART_IER_OFFSET)
#define FCR(x) __REG(MTK_SYSCTL_BASE + (x) + RT2880_UART_FCR_OFFSET)
#define LCR(x) __REG(MTK_SYSCTL_BASE + (x) + RT2880_UART_LCR_OFFSET)
#define LSR(x) __REG(MTK_SYSCTL_BASE + (x) + RT2880_UART_LSR_OFFSET)
#define DLL(x) __REG(MTK_SYSCTL_BASE + (x) + RT2880_UART_DLL_OFFSET)
#define DLM(x) __REG(MTK_SYSCTL_BASE + (x) + RT2880_UART_DLM_OFFSET)

#endif // _MTK_SERIAL_H_