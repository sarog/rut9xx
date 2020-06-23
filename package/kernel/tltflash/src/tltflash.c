/*
 * tltflash.c
 *
 * 2012 Daniele Palmas
 *
 * USB Abstract Control Model driver for Telit HE family
 * modems in flashing mode
 * 
 * Based on the module cdc-acm created by the following authors:
 * Armin Fuerst, Pavel Machek, Johannes Erdfelt, Vojtech Pavlik, David Kubicek, Johan Hovold
 * 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#undef DEBUG
#undef VERBOSE_DEBUG

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/serial.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/usb/cdc.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <linux/list.h>

#include "tltflash.h"


#define DRIVER_AUTHOR "Daniele Palmas"
#define DRIVER_DESC "USB Abstract Control Model driver for Telit HE family modems in flashing mode"

static struct usb_driver tlt_driver;
static struct tty_driver *tlt_tty_driver;
static struct tlt *tlt_table[TLT_TTY_MINORS];
static struct tty_port *ports[1];

static DEFINE_MUTEX(open_mutex);

#define TLT_READY(tlt)	(tlt && tlt->dev && tlt->port.count)

static const struct tty_port_operations tlt_port_ops = {
};

/*
 * Write buffer management.
 * All of these assume proper locks taken by the caller.
 */

static int tlt_wb_alloc(struct tlt *tlt)
{
	int i, wbn;
	struct tlt_wb *wb;

	wbn = 0;
	i = 0;
	for (;;) {
		wb = &tlt->wb[wbn];
		if (!wb->use) {
			wb->use = 1;
			return wbn;
		}
		wbn = (wbn + 1) % TLT_NW;
		if (++i >= TLT_NW)
			return -1;
	}
}

static int tlt_wb_is_avail(struct tlt *tlt)
{
	int i, n;
	unsigned long flags;

	n = TLT_NW;
	spin_lock_irqsave(&tlt->write_lock, flags);
	for (i = 0; i < TLT_NW; i++)
		n -= tlt->wb[i].use;
	spin_unlock_irqrestore(&tlt->write_lock, flags);
	return n;
}

/*
 * Finish write. Caller must hold tlt->write_lock
 */
static void tlt_write_done(struct tlt *tlt, struct tlt_wb *wb)
{
	wb->use = 0;
	tlt->transmitting--;
	usb_autopm_put_interface_async(tlt->control);
}

/*
 * Poke write.
 *
 * the caller is responsible for locking
 */

static int tlt_start_wb(struct tlt *tlt, struct tlt_wb *wb)
{
	int rc;

	tlt->transmitting++;

	wb->urb->transfer_buffer = wb->buf;
	wb->urb->transfer_dma = wb->dmah;
	wb->urb->transfer_buffer_length = wb->len;
	wb->urb->dev = tlt->dev;

	rc = usb_submit_urb(wb->urb, GFP_ATOMIC);
	if (rc < 0) {
		dev_err(&tlt->data->dev,
			"%s - usb_submit_urb(write bulk) failed: %d\n",
			__func__, rc);
		tlt_write_done(tlt, wb);
	}
	return rc;
}

static int tlt_write_start(struct tlt *tlt, int wbn)
{
	unsigned long flags;
	struct tlt_wb *wb = &tlt->wb[wbn];
	int rc;

	spin_lock_irqsave(&tlt->write_lock, flags);
	if (!tlt->dev) {
		wb->use = 0;
		spin_unlock_irqrestore(&tlt->write_lock, flags);
		return -ENODEV;
	}

	dev_vdbg(&tlt->data->dev, "%s - susp_count %d\n", __func__,
							tlt->susp_count);
	usb_autopm_get_interface_async(tlt->control);
	if (tlt->susp_count) {
		if (!tlt->delayed_wb)
			tlt->delayed_wb = wb;
		else
			usb_autopm_put_interface_async(tlt->control);
		spin_unlock_irqrestore(&tlt->write_lock, flags);
		return 0;	/* A white lie */
	}
	usb_mark_last_busy(tlt->dev);

	rc = tlt_start_wb(tlt, wb);
	spin_unlock_irqrestore(&tlt->write_lock, flags);

	return rc;
}

static int tlt_submit_read_urb(struct tlt *tlt, int index, gfp_t mem_flags)
{
	int res;
	

	if (!test_and_clear_bit(index, &tlt->read_urbs_free))
		return 0;

	dev_vdbg(&tlt->data->dev, "%s - urb %d\n", __func__, index);

	res = usb_submit_urb(tlt->read_urbs[index], mem_flags);
	if (res) {
		if (res != -EPERM) {
			dev_err(&tlt->data->dev,
					"%s - usb_submit_urb failed: %d\n",
					__func__, res);
		}
		set_bit(index, &tlt->read_urbs_free);
		return res;
	}

	return 0;
}

static int tlt_submit_read_urbs(struct tlt *tlt, gfp_t mem_flags)
{
	int res;
	int i;
	

	for (i = 0; i < tlt->rx_buflimit; ++i) {
		res = tlt_submit_read_urb(tlt, i, mem_flags);
		if (res)
			return res;
	}

	return 0;
}

static void tlt_process_read_urb(struct tlt *tlt, struct urb *urb)
{
	struct tty_struct *tty;

	if (!urb->actual_length)
		return;
	
	tty = tty_port_tty_get(&tlt->port);
	if (!tty)
		return;

	tty_insert_flip_string(&tlt->port, urb->transfer_buffer, urb->actual_length);
	tty_flip_buffer_push(&tlt->port);

	tty_kref_put(tty);
}

static void tlt_read_bulk_callback(struct urb *urb)
{
	struct tlt_rb *rb = urb->context;
	struct tlt *tlt = rb->instance;
	unsigned long flags;
	

	dev_vdbg(&tlt->data->dev, "%s - urb %d, len %d\n", __func__,
					rb->index, urb->actual_length);
	set_bit(rb->index, &tlt->read_urbs_free);

	if (!tlt->dev) {
		dev_dbg(&tlt->data->dev, "%s - disconnected\n", __func__);
		return;
	}
	usb_mark_last_busy(tlt->dev);

	if (urb->status) {
		dev_dbg(&tlt->data->dev, "%s - non-zero urb status: %d\n",
							__func__, urb->status);
		return;
	}
	tlt_process_read_urb(tlt, urb);

	/* throttle device if requested by tty */
	spin_lock_irqsave(&tlt->read_lock, flags);
	tlt->throttled = tlt->throttle_req;
	if (!tlt->throttled && !tlt->susp_count) {
		spin_unlock_irqrestore(&tlt->read_lock, flags);
		tlt_submit_read_urb(tlt, rb->index, GFP_ATOMIC);
	} else {
		spin_unlock_irqrestore(&tlt->read_lock, flags);
	}
}

/* data interface wrote those outgoing bytes */
static void tlt_write_bulk(struct urb *urb)
{
	struct tlt_wb *wb = urb->context;
	struct tlt *tlt = wb->instance;
	unsigned long flags;

	if (urb->status	|| (urb->actual_length != urb->transfer_buffer_length))
		dev_vdbg(&tlt->data->dev, "%s - len %d/%d, status %d\n",
			__func__,
			urb->actual_length,
			urb->transfer_buffer_length,
			urb->status);

	spin_lock_irqsave(&tlt->write_lock, flags);
	tlt_write_done(tlt, wb);
	spin_unlock_irqrestore(&tlt->write_lock, flags);
	if (TLT_READY(tlt))
		schedule_work(&tlt->work);
}

static void tlt_softint(struct work_struct *work)
{
	struct tlt *tlt = container_of(work, struct tlt, work);
	struct tty_struct *tty;

	dev_vdbg(&tlt->data->dev, "%s\n", __func__);

	if (!TLT_READY(tlt))
		return;
	tty = tty_port_tty_get(&tlt->port);
	if (!tty)
		return;
	tty_wakeup(tty);
	tty_kref_put(tty);
}

/*
 * TTY handlers
 */

static int tlt_tty_open(struct tty_struct *tty, struct file *filp)
{
	struct tlt *tlt;
	int rv = -ENODEV;

	mutex_lock(&open_mutex);
	
	tlt = tlt_table[tty->index];
	if (!tlt || !tlt->dev)
		goto out;
	else
		rv = 0;

	dev_dbg(&tlt->control->dev, "============%s\n", __func__);

	set_bit(TTY_NO_WRITE_SPLIT, &tty->flags);

	tty->driver_data = tlt;
	tty_port_tty_set(&tlt->port, tty);

	if (usb_autopm_get_interface(tlt->control) < 0)
		goto early_bail;
	else
		tlt->control->needs_remote_wakeup = 1;

	mutex_lock(&tlt->mutex);
	if (tlt->port.count++) {
		mutex_unlock(&tlt->mutex);
		usb_autopm_put_interface(tlt->control);
		goto out;
	}

	usb_autopm_put_interface(tlt->control);

	if (tlt_submit_read_urbs(tlt, GFP_KERNEL))
		goto bail_out;

	set_bit(ASYNCB_INITIALIZED, &tlt->port.flags);
	rv = tty_port_block_til_ready(&tlt->port, tty, filp);

	mutex_unlock(&tlt->mutex);
out:
	mutex_unlock(&open_mutex);
	return rv;

bail_out:
	tlt->port.count--;
	mutex_unlock(&tlt->mutex);
	usb_autopm_put_interface(tlt->control);
early_bail:
	mutex_unlock(&open_mutex);
	tty_port_tty_set(&tlt->port, NULL);
	return -EIO;
}

static void tlt_tty_unregister(struct tlt *tlt)
{
	int i;

	tty_unregister_device(tlt_tty_driver, tlt->minor);
	usb_put_intf(tlt->control);
	tlt_table[tlt->minor] = NULL;
	for (i = 0; i < TLT_NW; i++)
		usb_free_urb(tlt->wb[i].urb);
	for (i = 0; i < tlt->rx_buflimit; i++)
		usb_free_urb(tlt->read_urbs[i]);
	kfree(tlt);
}

static void tlt_port_down(struct tlt *tlt)
{
	int i;

	mutex_lock(&open_mutex);
	if (tlt->dev) {
		usb_autopm_get_interface(tlt->control);
		for (i = 0; i < TLT_NW; i++)
			usb_kill_urb(tlt->wb[i].urb);
		for (i = 0; i < tlt->rx_buflimit; i++)
			usb_kill_urb(tlt->read_urbs[i]);
		tlt->control->needs_remote_wakeup = 0;
		usb_autopm_put_interface(tlt->control);
	}
	mutex_unlock(&open_mutex);
}

static void tlt_tty_close(struct tty_struct *tty, struct file *filp)
{
	struct tlt *tlt = tty->driver_data;

	/* Perform the closing process and see if we need to do the hardware
	   shutdown */
	if (!tlt)
		return;
	if (tty_port_close_start(&tlt->port, tty, filp) == 0) {
		mutex_lock(&open_mutex);
		if (!tlt->dev) {
			tty_port_tty_set(&tlt->port, NULL);
			tlt_tty_unregister(tlt);
			tty->driver_data = NULL;
		}
		mutex_unlock(&open_mutex);
		return;
	}
	tlt_port_down(tlt);
	tty_port_close_end(&tlt->port, tty);
	tty_port_tty_set(&tlt->port, NULL);
}

static int tlt_tty_write(struct tty_struct *tty,
					const unsigned char *buf, int count)
{
	struct tlt *tlt = tty->driver_data;
	int stat;
	unsigned long flags;
	int wbn;
	struct tlt_wb *wb;

	if (!TLT_READY(tlt))
		return -EINVAL;
	if (!count)
		return 0;

	dev_vdbg(&tlt->data->dev, "%s - count %d\n", __func__, count);

	spin_lock_irqsave(&tlt->write_lock, flags);
	wbn = tlt_wb_alloc(tlt);
	if (wbn < 0) {
		spin_unlock_irqrestore(&tlt->write_lock, flags);
		return 0;
	}
	wb = &tlt->wb[wbn];

	count = (count > tlt->writesize) ? tlt->writesize : count;
	dev_vdbg(&tlt->data->dev, "%s - write %d\n", __func__, count);
	memcpy(wb->buf, buf, count);
	wb->len = count;
	spin_unlock_irqrestore(&tlt->write_lock, flags);

	stat = tlt_write_start(tlt, wbn);
	if (stat < 0)
		return stat;
	return count;
}

static int tlt_tty_write_room(struct tty_struct *tty)
{
	struct tlt *tlt = tty->driver_data;
	if (!TLT_READY(tlt))
		return -EINVAL;
	/*
	 * Do not let the line discipline to know that we have a reserve,
	 * or it might get too enthusiastic.
	 */
	return tlt_wb_is_avail(tlt) ? tlt->writesize : 0;
}

static int tlt_tty_chars_in_buffer(struct tty_struct *tty)
{
	struct tlt *tlt = tty->driver_data;
	if (!TLT_READY(tlt))
		return 0;
	/*
	 * This is inaccurate (overcounts), but it works.
	 */
	return (TLT_NW - tlt_wb_is_avail(tlt)) * tlt->writesize;
}

static void tlt_tty_throttle(struct tty_struct *tty)
{
	struct tlt *tlt = tty->driver_data;

	if (!TLT_READY(tlt))
		return;

	spin_lock_irq(&tlt->read_lock);
	tlt->throttle_req = 1;
	spin_unlock_irq(&tlt->read_lock);
}

static void tlt_tty_unthrottle(struct tty_struct *tty)
{
	struct tlt *tlt = tty->driver_data;
	unsigned int was_throttled;

	if (!TLT_READY(tlt))
		return;

	spin_lock_irq(&tlt->read_lock);
	was_throttled = tlt->throttled;
	tlt->throttled = 0;
	tlt->throttle_req = 0;
	spin_unlock_irq(&tlt->read_lock);

	if (was_throttled)
		tlt_submit_read_urbs(tlt, GFP_KERNEL);
}

static int tlt_tty_tiocmget(struct tty_struct *tty)
{
	return -EPERM;
}

static int tlt_tty_tiocmset(struct tty_struct *tty,
			    unsigned int set, unsigned int clear)
{
	return -EPERM;
}

static int tlt_tty_ioctl(struct tty_struct *tty,
					unsigned int cmd, unsigned long arg)
{
	struct tlt *tlt = tty->driver_data;

	if (!TLT_READY(tlt))
		return -EINVAL;

	return -ENOIOCTLCMD;
}

static void tlt_tty_set_termios(struct tty_struct *tty,
						struct ktermios *termios_old)
{
	return;
}

/*
 * USB probe and disconnect routines.
 */

/* Little helpers: write/read buffers free */
static void tlt_write_buffers_free(struct tlt *tlt)
{
	int i;
	struct tlt_wb *wb;
	
	struct usb_device *usb_dev = interface_to_usbdev(tlt->control);

	for (wb = &tlt->wb[0], i = 0; i < TLT_NW; i++, wb++)
		usb_free_coherent(usb_dev, tlt->writesize, wb->buf, wb->dmah);
}

static void tlt_read_buffers_free(struct tlt *tlt)
{
	struct usb_device *usb_dev = interface_to_usbdev(tlt->control);
	int i;

	for (i = 0; i < tlt->rx_buflimit; i++)
		usb_free_coherent(usb_dev, tlt->readsize,
			  tlt->read_buffers[i].base, tlt->read_buffers[i].dma);
}

/* Little helper: write buffers allocate */
static int tlt_write_buffers_alloc(struct tlt *tlt)
{
	int i;
	struct tlt_wb *wb;

	for (wb = &tlt->wb[0], i = 0; i < TLT_NW; i++, wb++) {
		wb->buf = usb_alloc_coherent(tlt->dev, tlt->writesize, GFP_KERNEL,
		    &wb->dmah);
		if (!wb->buf) {
			while (i != 0) {
				--i;
				--wb;
				usb_free_coherent(tlt->dev, tlt->writesize,
				    wb->buf, wb->dmah);
			}
			return -ENOMEM;
		}
	}
	return 0;
}

static int tlt_probe(struct usb_interface *intf,
		     const struct usb_device_id *id)
{
	struct usb_cdc_union_desc *union_header = NULL;
	unsigned char *buffer = intf->altsetting->extra;
	int buflen = intf->altsetting->extralen;
	struct usb_interface *control_interface;
	struct usb_interface *data_interface;
	struct usb_endpoint_descriptor *epctrl = NULL;
	struct usb_endpoint_descriptor *epread = NULL;
	struct usb_endpoint_descriptor *epwrite = NULL;
	struct usb_device *usb_dev = interface_to_usbdev(intf);
	struct tlt *tlt;
	int minor;
	int ctrlsize, readsize;
	u8 *buf;
	u8 ac_management_function = 0;
	int data_interface_num = -1;
	int num_rx_buf;
	int i;
	int combined_interfaces = 0;

	num_rx_buf = TLT_NR;

	/* normal probing*/
	if (!buffer) {
		dev_err(&intf->dev, "Weird descriptor references\n");
		return -EINVAL;
	}

	if (!buflen) {
		if (intf->cur_altsetting->endpoint &&
				intf->cur_altsetting->endpoint->extralen &&
				intf->cur_altsetting->endpoint->extra) {
			dev_dbg(&intf->dev,
				"Seeking extra descriptors on endpoint\n");
			buflen = intf->cur_altsetting->endpoint->extralen;
			buffer = intf->cur_altsetting->endpoint->extra;
		} else {
			dev_err(&intf->dev,
				"Zero length descriptor references\n");
			return -EINVAL;
		}
	}

	while (buflen > 0) {
		if (buffer[1] != USB_DT_CS_INTERFACE) {
			dev_err(&intf->dev, "skipping garbage\n");
			goto next_desc;
		}

		switch (buffer[2]) {
		case USB_CDC_UNION_TYPE: /* we've found it */
			if (union_header) {
				dev_err(&intf->dev, "More than one "
					"union descriptor, skipping ...\n");
				goto next_desc;
			}
			union_header = (struct usb_cdc_union_desc *)buffer;
			break;
		case USB_CDC_ACM_TYPE:
			ac_management_function = buffer[3];
			break;
		default:
			/* there are LOTS more CDC descriptors that
			 * could legitimately be found here.
			 */
			dev_dbg(&intf->dev, "Ignoring descriptor: "
					"type %02x, length %d\n",
					buffer[2], buffer[0]);
			break;
		}
next_desc:
		buflen -= buffer[0];
		buffer += buffer[0];
	}

	control_interface = usb_ifnum_to_if(usb_dev, union_header->bMasterInterface0);
	data_interface = usb_ifnum_to_if(usb_dev, (data_interface_num = union_header->bSlaveInterface0));
	if (!control_interface || !data_interface) {
		dev_dbg(&intf->dev, "no interfaces\n");
		return -ENODEV;
	}

	/* Accept probe requests only for the control interface */
	if (!combined_interfaces && intf != control_interface)
		return -ENODEV;

	if (!combined_interfaces && usb_interface_claimed(data_interface)) {
		/* valid in this context */
		dev_dbg(&intf->dev, "The data interface isn't available\n");
		return -EBUSY;
	}


	if (data_interface->cur_altsetting->desc.bNumEndpoints < 2)
		return -EINVAL;

	epctrl = &control_interface->cur_altsetting->endpoint[0].desc;
	epread = &data_interface->cur_altsetting->endpoint[0].desc;
	epwrite = &data_interface->cur_altsetting->endpoint[1].desc;

	dev_dbg(&intf->dev, "interfaces are valid\n");
	for (minor = 0; minor < TLT_TTY_MINORS && tlt_table[minor]; minor++);

	if (minor == TLT_TTY_MINORS) {
		dev_err(&intf->dev, "no more free tlt devices\n");
		return -ENODEV;
	}

	tlt = kzalloc(sizeof(struct tlt), GFP_KERNEL);
	if (tlt == NULL) {
		dev_err(&intf->dev, "out of memory (tlt kzalloc)\n");
		goto alloc_fail;
	}

	ctrlsize = le16_to_cpu(epctrl->wMaxPacketSize);
	readsize = le16_to_cpu(epread->wMaxPacketSize) * 2;
	tlt->combined_interfaces = combined_interfaces;
	tlt->writesize = le16_to_cpu(epwrite->wMaxPacketSize) * 20;
	tlt->control = control_interface;
	tlt->data = data_interface;
	tlt->minor = minor;
	tlt->dev = usb_dev;
	tlt->ctrl_caps = ac_management_function;
	tlt->ctrlsize = ctrlsize;
	tlt->readsize = readsize;
	tlt->rx_buflimit = num_rx_buf;
	INIT_WORK(&tlt->work, tlt_softint);
	spin_lock_init(&tlt->write_lock);
	spin_lock_init(&tlt->read_lock);
	mutex_init(&tlt->mutex);
	tlt->rx_endpoint = usb_rcvbulkpipe(usb_dev, epread->bEndpointAddress);
	tlt->is_int_ep = usb_endpoint_xfer_int(epread);
	if (tlt->is_int_ep)
		tlt->bInterval = epread->bInterval;
	tty_port_init(&tlt->port);
	tlt->port.ops = &tlt_port_ops;

	buf = usb_alloc_coherent(usb_dev, ctrlsize, GFP_KERNEL, &tlt->ctrl_dma);
	if (!buf) {
		dev_err(&intf->dev, "out of memory (ctrl buffer alloc)\n");
		goto alloc_fail2;
	}
	tlt->ctrl_buffer = buf;

	if (tlt_write_buffers_alloc(tlt) < 0) {
		dev_err(&intf->dev, "out of memory (write buffer alloc)\n");
		goto alloc_fail4;
	}

	for (i = 0; i < num_rx_buf; i++) {
		struct tlt_rb *rb = &(tlt->read_buffers[i]);
		struct urb *urb;

		rb->base = usb_alloc_coherent(tlt->dev, readsize, GFP_KERNEL,
								&rb->dma);
		if (!rb->base) {
			dev_err(&intf->dev, "out of memory "
					"(read bufs usb_alloc_coherent)\n");
			goto alloc_fail6;
		}
		rb->index = i;
		rb->instance = tlt;

		urb = usb_alloc_urb(0, GFP_KERNEL);
		if (!urb) {
			dev_err(&intf->dev,
				"out of memory (read urbs usb_alloc_urb)\n");
			goto alloc_fail6;
		}
		urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
		urb->transfer_dma = rb->dma;
		if (tlt->is_int_ep) {
			usb_fill_int_urb(urb, tlt->dev,
					 tlt->rx_endpoint,
					 rb->base,
					 tlt->readsize,
					 tlt_read_bulk_callback, rb,
					 tlt->bInterval);
		} else {
			usb_fill_bulk_urb(urb, tlt->dev,
					  tlt->rx_endpoint,
					  rb->base,
					  tlt->readsize,
					  tlt_read_bulk_callback, rb);
		}

		tlt->read_urbs[i] = urb;
		__set_bit(i, &tlt->read_urbs_free);
	}
	for (i = 0; i < TLT_NW; i++) {
		struct tlt_wb *snd = &(tlt->wb[i]);

		snd->urb = usb_alloc_urb(0, GFP_KERNEL);
		if (snd->urb == NULL) {
			dev_err(&intf->dev,
				"out of memory (write urbs usb_alloc_urb)\n");
			goto alloc_fail7;
		}

		if (usb_endpoint_xfer_int(epwrite))
			usb_fill_int_urb(snd->urb, usb_dev,
				usb_sndbulkpipe(usb_dev, epwrite->bEndpointAddress),
				NULL, tlt->writesize, tlt_write_bulk, snd, epwrite->bInterval);
		else
			usb_fill_bulk_urb(snd->urb, usb_dev,
				usb_sndbulkpipe(usb_dev, epwrite->bEndpointAddress),
				NULL, tlt->writesize, tlt_write_bulk, snd);
		snd->urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
		snd->instance = tlt;
	}

	usb_set_intfdata(intf, tlt);

	dev_info(&intf->dev, "ttyACM%d: USB ACM device for Telit modems\n", minor);

	tlt->line.dwDTERate = cpu_to_le32(9600);
	tlt->line.bDataBits = 8;

	usb_driver_claim_interface(&tlt_driver, data_interface, tlt);
	usb_set_intfdata(data_interface, tlt);
	usb_get_intf(control_interface);
	
	ports[0] = &tlt->port;
	tlt_tty_driver->ports = ports;
	
	tty_register_device(tlt_tty_driver, minor, &control_interface->dev);

	tlt_table[minor] = tlt;

	return 0;
alloc_fail7:
	for (i = 0; i < TLT_NW; i++)
		usb_free_urb(tlt->wb[i].urb);
alloc_fail6:
	for (i = 0; i < num_rx_buf; i++)
		usb_free_urb(tlt->read_urbs[i]);
	tlt_read_buffers_free(tlt);
	tlt_write_buffers_free(tlt);
alloc_fail4:
	usb_free_coherent(usb_dev, ctrlsize, tlt->ctrl_buffer, tlt->ctrl_dma);
alloc_fail2:
	kfree(tlt);
alloc_fail:
	return -ENOMEM;
}

static void stop_data_traffic(struct tlt *tlt)
{
	int i;

	dev_dbg(&tlt->control->dev, "%s\n", __func__);

	for (i = 0; i < TLT_NW; i++)
		usb_kill_urb(tlt->wb[i].urb);
	for (i = 0; i < tlt->rx_buflimit; i++)
		usb_kill_urb(tlt->read_urbs[i]);

	cancel_work_sync(&tlt->work);
}

static void tlt_disconnect(struct usb_interface *intf)
{
	struct tlt *tlt = usb_get_intfdata(intf);
	struct usb_device *usb_dev = interface_to_usbdev(intf);
	struct tty_struct *tty;

	/* sibling interface is already cleaning up */
	if (!tlt)
		return;

	mutex_lock(&open_mutex);
	tlt->dev = NULL;
	usb_set_intfdata(tlt->control, NULL);
	usb_set_intfdata(tlt->data, NULL);

	stop_data_traffic(tlt);

	tlt_write_buffers_free(tlt);
	usb_free_coherent(usb_dev, tlt->ctrlsize, tlt->ctrl_buffer,
			  tlt->ctrl_dma);
	tlt_read_buffers_free(tlt);

	if (!tlt->combined_interfaces)
		usb_driver_release_interface(&tlt_driver, intf == tlt->control ?
					tlt->data : tlt->control);

	if (tlt->port.count == 0) {
		tlt_tty_unregister(tlt);
		mutex_unlock(&open_mutex);
		return;
	}

	mutex_unlock(&open_mutex);
	tty = tty_port_tty_get(&tlt->port);
	if (tty) {
		tty_hangup(tty);
		tty_kref_put(tty);
	}
}

/*
 * USB driver structure.
 */

static const struct usb_device_id tlt_ids[] = {
	{ USB_DEVICE(0x058b, 0x0041), },
	{ }
};

MODULE_DEVICE_TABLE(usb, tlt_ids);

static struct usb_driver tlt_driver = {
	.name =		"tltflash",
	.probe =	tlt_probe,
	.disconnect =	tlt_disconnect,
	.id_table =	tlt_ids,
#ifdef CONFIG_PM
	.supports_autosuspend = 1,
#endif
};

/*
 * TTY driver structures.
 */

static const struct tty_operations tlt_ops = {
	.open =			tlt_tty_open,
	.close =		tlt_tty_close,
	.write =		tlt_tty_write,
	.write_room =		tlt_tty_write_room,
	.ioctl =		tlt_tty_ioctl,
	.throttle =		tlt_tty_throttle,
	.unthrottle =		tlt_tty_unthrottle,
	.chars_in_buffer =	tlt_tty_chars_in_buffer,
	.set_termios =		tlt_tty_set_termios,
	.tiocmget =		tlt_tty_tiocmget,
	.tiocmset =		tlt_tty_tiocmset,
};

/*
 * Init / exit.
 */

static int __init tlt_init(void)
{
	int retval;
	tlt_tty_driver = alloc_tty_driver(TLT_TTY_MINORS);
	if (!tlt_tty_driver)
		return -ENOMEM;
	tlt_tty_driver->owner = THIS_MODULE,
	tlt_tty_driver->driver_name = "tlt",
	tlt_tty_driver->name = "ttyTLT",
	tlt_tty_driver->major = TLT_TTY_MAJOR,
	tlt_tty_driver->minor_start = 0,
	tlt_tty_driver->type = TTY_DRIVER_TYPE_SERIAL,
	tlt_tty_driver->subtype = SERIAL_TYPE_NORMAL,
	tlt_tty_driver->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
	tlt_tty_driver->init_termios = tty_std_termios;
	tlt_tty_driver->init_termios.c_cflag = B9600 | CS8 | CREAD |
								HUPCL | CLOCAL;
	tty_set_operations(tlt_tty_driver, &tlt_ops);

	retval = tty_register_driver(tlt_tty_driver);
	if (retval) {
		put_tty_driver(tlt_tty_driver);
		return retval;
	}

	retval = usb_register(&tlt_driver);
	if (retval) {
		tty_unregister_driver(tlt_tty_driver);
		put_tty_driver(tlt_tty_driver);
		return retval;
	}

	printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_DESC "\n");

	return 0;
}

static void __exit tlt_exit(void)
{
	usb_deregister(&tlt_driver);
	tty_unregister_driver(tlt_tty_driver);
	put_tty_driver(tlt_tty_driver);
}

module_init(tlt_init);
module_exit(tlt_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
MODULE_ALIAS_CHARDEV_MAJOR(TLT_TTY_MAJOR);
