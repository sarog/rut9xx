/*
 *
 * Includes for tlt-acm.c
 *
 * Infineon flashing device is mainly a cdc-acm device, but there are some packets in the 
 * standard driver that makes the device not to work. This is a simplified driver that
 * shall be used with Telit devices based on Infineon chipset
 * 
 * 2012 Daniele Palmas
 * 
 * Based on cdc-acm module by the following authors:
 * Armin Fuerst, Pavel Machek, Johannes Erdfelt, Vojtech Pavlik, David Kubicek, Johan Hovold
 *
 */

/*
 * CMSPAR, some architectures can't have space and mark parity.
 */

#ifndef CMSPAR
#define CMSPAR			0
#endif

/*
 * Major and minor numbers.
 */

#define TLT_TTY_MAJOR		61
#define TLT_TTY_MINORS		32

/*
 * Internal driver structures.
 */

/*
 * The only reason to have several buffers is to accommodate assumptions
 * in line disciplines. They ask for empty space amount, receive our URB size,
 * and proceed to issue several 1-character writes, assuming they will fit.
 * The very first write takes a complete URB. Fortunately, this only happens
 * when processing onlcr, so we only need 2 buffers. These values must be
 * powers of 2.
 */
#define TLT_NW  16
#define TLT_NR  16

struct tlt_wb {
	unsigned char *buf;
	dma_addr_t dmah;
	int len;
	int use;
	struct urb		*urb;
	struct tlt		*instance;
};

struct tlt_rb {
	int			size;
	unsigned char		*base;
	dma_addr_t		dma;
	int			index;
	struct tlt		*instance;
};

struct tlt {
	struct usb_device *dev;				/* the corresponding usb device */
	struct usb_interface *control;			/* control interface */
	struct usb_interface *data;			/* data interface */
	struct tty_port port;			 	/* our tty port data */
	u8 *ctrl_buffer;				/* buffers of urbs */
	dma_addr_t ctrl_dma;				/* dma handles of buffers */
	struct tlt_wb wb[TLT_NW];
	unsigned long read_urbs_free;
	struct urb *read_urbs[TLT_NR];
	struct tlt_rb read_buffers[TLT_NR];
	int rx_buflimit;
	int rx_endpoint;
	spinlock_t read_lock;
	int write_used;					/* number of non-empty write buffers */
	int transmitting;
	spinlock_t write_lock;
	struct mutex mutex;
	struct usb_cdc_line_coding line;		/* bits, stop, parity */
	struct work_struct work;			/* work queue entry for line discipline waking up */
	unsigned int writesize;				/* max packet size for the output bulk endpoint */
	unsigned int readsize,ctrlsize;			/* buffer sizes for freeing */
	unsigned int minor;				/* acm minor number */
	unsigned int ctrl_caps;				/* control capabilities from the class specific header */
	unsigned int susp_count;			/* number of suspended interfaces */
	unsigned int combined_interfaces:1;		/* control and data collapsed */
	unsigned int is_int_ep:1;			/* interrupt endpoints contrary to spec used */
	unsigned int throttled:1;			/* actually throttled */
	unsigned int throttle_req:1;			/* throttle requested */
	u8 bInterval;
	struct tlt_wb *delayed_wb;			/* write queued for a device about to be woken */
};
