/***************************************************************************
 *   Copyright (C) 2009 - 2010 by Simon Qian <SimonQian@SimonQian.com>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "vsf.h"

struct vsfusbh_hub_t
{
	struct vsfsm_t sm;
	struct vsfsm_pt_t pt;
	struct vsfsm_pt_t connect_pt;
	struct vsfsm_pt_t reset_pt;

	struct vsfusbh_t *usbh;
	struct vsfusbh_device_t *dev;
	struct vsfusbh_ifs_t *ifs;

	struct vsfhcd_urb_t *urb;
	struct vsftimer_t *polltimer;

	struct usb_hub_descriptor_t hub_desc;
	//struct usb_hub_status_t hub_status;
	struct usb_port_status_t hub_portsts;
	struct vsfsm_sem_t poll_sem;

	uint16_t reset_mask;
	uint16_t reconnect_mask;
	int16_t counter;
	int16_t retry;
	int16_t inited;
};

static vsf_err_t hub_set_port_feature(struct vsfusbh_t *usbh,
		struct vsfhcd_urb_t *urb, uint16_t port, uint16_t feature)
{
	urb->pipe = usb_sndctrlpipe(urb->hcddev, 0);
	return vsfusbh_control_msg(usbh, urb, USB_RT_PORT, USB_REQ_SET_FEATURE,
			feature, port);
}
static vsf_err_t hub_get_port_status(struct vsfusbh_t *usbh,
		struct vsfhcd_urb_t *urb, uint16_t port)
{
	urb->pipe = usb_rcvctrlpipe(urb->hcddev, 0);
	return vsfusbh_control_msg(usbh, urb, USB_DIR_IN | USB_RT_PORT,
			USB_REQ_GET_STATUS, 0, port);
}
static vsf_err_t hub_clear_port_feature(struct vsfusbh_t *usbh,
		struct vsfhcd_urb_t *urb, int port, int feature)
{
	urb->pipe = usb_sndctrlpipe(urb->hcddev, 0);
	return vsfusbh_control_msg(usbh, urb, USB_RT_PORT, USB_REQ_CLEAR_FEATURE,
			feature, port);
}
#if 0
static vsf_err_t hub_get_status(struct vsfusbh_t *usbh, struct vsfhcd_urb_t *urb)
{
	urb->pipe = usb_rcvctrlpipe(urb->hcddev, 0);
	return vsfusbh_control_msg(usbh, urb, USB_DIR_IN | USB_RT_HUB,
			USB_REQ_GET_STATUS, 0, 0);
}
#endif

static vsf_err_t hub_reset_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfusbh_hub_t *hub = (struct vsfusbh_hub_t *)pt->user_data;
	struct vsfhcd_urb_t *urb = hub->urb;
	vsf_err_t err;

	vsfsm_pt_begin(pt);

	hub->retry = 0;
	vsfsm_pt_delay(pt, 10);
	
	do
	{
		/* send command to reset port */
		err = hub_set_port_feature(hub->usbh, urb, hub->counter,
				USB_PORT_FEAT_RESET);
		if (err) goto ret_error;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (urb->status != URB_OK) goto ret_failure;

		/* delay 10ms after port reset*/
		vsfsm_pt_delay(pt, 10);

		// clear reset
		err = hub_clear_port_feature(hub->usbh, urb,
				hub->counter, USB_PORT_FEAT_C_RESET);
		if (err) goto ret_error;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (urb->status != URB_OK) goto ret_failure;

		if (hub->reconnect_mask & (1 << (hub->counter - 1)))
			vsfsm_pt_delay(pt, 1000);
		else
			vsfsm_pt_delay(pt, 20);

		/* get port status for check */
		urb->transfer_buffer = &hub->hub_portsts;
		urb->transfer_length = sizeof(hub->hub_portsts);
		err = hub_get_port_status(hub->usbh, urb, hub->counter);
		if (err) goto ret_error;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (urb->status != URB_OK) goto ret_failure;
		urb->transfer_buffer = NULL;
		urb->transfer_length = 0;

		/* check port status after reset */
		if (hub->hub_portsts.wPortStatus & USB_PORT_STAT_ENABLE)
		{
			return VSFERR_NONE;
		}
		else
		{
			/* delay 200ms for next reset*/
			vsfsm_pt_delay(pt, 200);
		}
	} while (hub->retry++ <= 3);

	vsfsm_pt_end(pt);
	return VSFERR_FAIL;

ret_failure:
	err = VSFERR_FAIL;
ret_error:
	return err;
}

static vsf_err_t hub_connect_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_hub_t *hub = (struct vsfusbh_hub_t *)pt->user_data;
	struct vsfhcd_urb_t *urb = hub->urb;
	struct vsfusbh_device_t *dev;

	vsfsm_pt_begin(pt);

	/* clear the cnnection change state */
	err = hub_clear_port_feature(hub->usbh, urb, hub->counter,
			USB_PORT_FEAT_C_CONNECTION);
	if (err) goto ret_error;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (urb->status != URB_OK) goto ret_failure;

	if (hub->dev->children[hub->counter - 1] != NULL)
	{
		vsfusbh_disconnect_device(hub->usbh,
				&hub->dev->children[hub->counter - 1]);
	}

	if (!(hub->hub_portsts.wPortStatus & USB_PORT_STAT_CONNECTION))
	{
		if (hub->hub_portsts.wPortStatus & USB_PORT_STAT_ENABLE)
		{
			err = hub_clear_port_feature(hub->usbh, urb,
					hub->counter, USB_PORT_FEAT_ENABLE);
			if (err) goto ret_error;
			vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
			if (urb->status != URB_OK) goto ret_failure;
		}

		return VSFERR_NONE;
	}

	if (hub->hub_portsts.wPortStatus & USB_PORT_STAT_LOW_SPEED)
		vsfsm_pt_delay(pt, 200);

	vsfsm_pt_wfpt(pt, &hub->reset_pt);

	vsfsm_pt_delay(pt, 200);

	// wait for new_dev free
	while (hub->usbh->new_dev != NULL)
	{
		vsfsm_pt_delay(pt, 200);
	}

	dev = vsfusbh_alloc_device(hub->usbh);
	if (NULL == dev) goto ret_failure;

	dev->hcddev.speed = (hub->hub_portsts.wPortStatus & USB_PORT_STAT_LOW_SPEED) ? USB_SPEED_LOW :
		(hub->hub_portsts.wPortStatus & USB_PORT_STAT_HIGH_SPEED) ? USB_SPEED_HIGH : USB_SPEED_FULL;

	hub->dev->children[hub->counter - 1] = dev;
	dev->parent = hub->dev;

	hub->usbh->new_dev = dev;
	vsfsm_post_evt_pending(&hub->usbh->sm, VSFSM_EVT_NEW_DEVICE);
	
	vsfsm_pt_end(pt);
	return VSFERR_NONE;

ret_failure:
	err = VSFERR_FAIL;
ret_error:
	return err;
}

static void hub_poll_timer_cb(void *param)
{
	vsfsm_sem_post((struct vsfsm_sem_t *)param);
}

static vsf_err_t hub_scan_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_hub_t *hub = (struct vsfusbh_hub_t *)pt->user_data;
	struct vsfhcd_urb_t *urb = hub->urb;

	vsfsm_pt_begin(pt);

	do
	{
		hub->counter = 1;
		do
		{
			if (hub->reset_mask & (1 << (hub->counter - 1)))
			{
				vsfsm_pt_wfpt(pt, &hub->reset_pt);
				hub->reset_mask &= ~(1 << (hub->counter - 1));
			}

			// get port status
			urb->transfer_buffer = &hub->hub_portsts;
			urb->transfer_length = sizeof(hub->hub_portsts);
			err = hub_get_port_status(hub->usbh, urb, hub->counter);
			if (err) goto ret_error;
			vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
			if (urb->status != URB_OK) goto ret_failure;
			urb->transfer_buffer = NULL;
			urb->transfer_length = 0;

			if (hub->reconnect_mask & (1 << (hub->counter - 1)))
			{
				if (!hub->usbh->new_dev)
				{
					vsfsm_pt_wfpt(pt, &hub->connect_pt);
					hub->reconnect_mask &= ~(0x1ul << (hub->counter - 1));
				}
			}
			else if (hub->hub_portsts.wPortChange & USB_PORT_STAT_C_CONNECTION)
			{
				if (!hub->usbh->new_dev)
				{
					// try to connect
					vsfsm_pt_wfpt(pt, &hub->connect_pt);
				}
			}
			else if (hub->hub_portsts.wPortChange & USB_PORT_STAT_C_ENABLE)
			{
				err = hub_clear_port_feature(hub->usbh, urb,
						hub->counter, USB_PORT_FEAT_C_ENABLE);
				if (err) goto ret_error;
				vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
				if (urb->status != URB_OK) goto ret_failure;
				hub->hub_portsts.wPortChange &= ~USB_PORT_STAT_C_ENABLE;
			}

			if (hub->hub_portsts.wPortChange & USB_PORT_STAT_C_SUSPEND)
			{
				err = hub_clear_port_feature(hub->usbh, urb,
						hub->counter, USB_PORT_FEAT_C_SUSPEND);
				if (err) goto ret_error;
				vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
				if (urb->status != URB_OK) goto ret_failure;
				hub->hub_portsts.wPortChange &= ~USB_PORT_STAT_C_SUSPEND;
			}
			if (hub->hub_portsts.wPortChange & USB_PORT_STAT_C_OVERCURRENT)
			{
				err = hub_clear_port_feature(hub->usbh, urb,
						hub->counter, USB_PORT_FEAT_C_OVER_CURRENT);
				if (err) goto ret_error;
				vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
				if (urb->status != URB_OK) goto ret_failure;
				hub->hub_portsts.wPortChange &= ~USB_PORT_FEAT_C_OVER_CURRENT;

				// TODO : power every port
			}
			if (hub->hub_portsts.wPortChange & USB_PORT_STAT_C_RESET)
			{
				err = hub_clear_port_feature(hub->usbh, urb,
						hub->counter, USB_PORT_FEAT_C_RESET);
				if (err) goto ret_error;
				vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
				if (urb->status != URB_OK) goto ret_failure;
				hub->hub_portsts.wPortChange &= ~USB_PORT_FEAT_C_RESET;
			}
		} while (hub->counter++ < hub->dev->maxchild);

		// TODO : poll hub status

		if (vsfsm_sem_pend(&hub->poll_sem, pt->sm))
			vsfsm_pt_wfe(pt, hub->poll_sem.evt);
	} while (1);

	vsfsm_pt_end(pt);
	return VSFERR_NONE;

ret_failure:
	err = VSFERR_FAIL;
ret_error:
	return err;
}

static vsf_err_t hub_class_check(struct vsfusbh_device_t *dev)
{
	struct usb_interface_desc_t *ifs_desc;
	struct usb_endpoint_desc_t *ep_desc;

	if (dev == NULL)
		return VSFERR_FAIL;

	ifs_desc = dev->config.ifs->alt->ifs_desc;
	ep_desc = dev->config.ifs->alt->ep_desc;
	if ((ifs_desc->bInterfaceClass != USB_CLASS_HUB) ||
			(ifs_desc->bInterfaceSubClass > 1) ||
			(ifs_desc->bNumEndpoints != 1) ||
			(!(ep_desc->bEndpointAddress & USB_DIR_IN)) ||
			((ep_desc->bmAttributes & USB_ENDPOINT_XFER_INT) != USB_ENDPOINT_XFER_INT))
		return VSFERR_FAIL;

	return VSFERR_NONE;
}

static vsf_err_t hub_get_descriptor(struct vsfusbh_t *usbh,
		struct vsfhcd_urb_t *urb)
{
	urb->pipe = usb_rcvctrlpipe(urb->hcddev, 0);
	return vsfusbh_control_msg(usbh, urb, USB_DIR_IN | USB_RT_HUB,
			USB_REQ_GET_DESCRIPTOR, (USB_DT_HUB << 8), 0);
}

static vsf_err_t hub_init_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_hub_t *hub = (struct vsfusbh_hub_t *)pt->user_data;
	struct vsfhcd_urb_t *urb = hub->urb;

	vsfsm_pt_begin(pt);
	urb->notifier_sm = &hub->sm;

	urb->transfer_buffer = &hub->hub_desc;
	urb->transfer_length = 4;
	err = hub_get_descriptor(hub->usbh, urb);
	if (err) goto ret_error;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (urb->status != URB_OK) goto ret_failure;
	if (hub->hub_desc.bDescLength > sizeof(hub->hub_desc)) goto ret_failure;

	urb->transfer_length = hub->hub_desc.bDescLength;
	err = hub_get_descriptor(hub->usbh, urb);
	if (err) goto ret_error;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (urb->status != URB_OK) goto ret_failure;
	hub->urb->transfer_buffer = NULL;
	hub->urb->transfer_length = 0;

	hub->dev->maxchild = min(hub->hub_desc.bNbrPorts, USB_MAXCHILDREN);
	hub->counter = 0;

	do
	{
		hub->counter++;
		err = hub_set_port_feature(hub->usbh, hub->urb, hub->counter,
				USB_PORT_FEAT_POWER);
		if (err) goto ret_error;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (urb->status != URB_OK) goto ret_failure;
		vsfsm_pt_delay(pt, hub->hub_desc.bPwrOn2PwrGood * 2);
	} while (hub->counter < hub->dev->maxchild);

	vsfsm_pt_end(pt);
	return VSFERR_NONE;

ret_failure:
	err = VSFERR_FAIL;
ret_error:
	return err;
}

static struct vsfsm_state_t *vsfusbh_hub_evt_handler_init(struct vsfsm_t *sm,
		vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_hub_t *hub = (struct vsfusbh_hub_t *)sm->user_data;

	switch (evt)
	{
	case VSFSM_EVT_INIT:
		hub->pt.thread = hub_init_thread;
		hub->pt.user_data = hub;
		hub->pt.sm = sm;
		hub->pt.state = 0;
		hub->connect_pt.thread = hub_connect_thread;
		hub->connect_pt.user_data = hub;
		hub->connect_pt.sm = sm;
		hub->connect_pt.state = 0;
		hub->reset_pt.thread = hub_reset_thread;
		hub->reset_pt.user_data = hub;
		hub->reset_pt.sm = sm;
		hub->reset_pt.state = 0;
		hub->inited = false;

	case VSFSM_EVT_URB_COMPLETE:
	case VSFSM_EVT_DELAY_DONE:
		if (!hub->inited)
		{
			err = hub->pt.thread(&hub->pt, evt);
			if (!err)
			{
				vsfsm_sem_init(&hub->poll_sem, 0, VSFSM_EVT_DELAY_DONE);
				hub->polltimer = vsftimer_create_cb(200, -1, hub_poll_timer_cb, &hub->poll_sem);
				hub->inited = true;
				hub->pt.thread = hub_scan_thread;
				hub->pt.user_data = hub;
				hub->pt.sm = sm;
				hub->pt.state = 0;
				vsfsm_post_evt_pending(sm, VSFSM_EVT_DELAY_DONE);
			}
			else if (err < 0)
				vsfusbh_remove_interface(hub->usbh, hub->dev, hub->ifs);
		}
		else
		{
			err = hub->pt.thread(&hub->pt, evt);
			if (err < 0)
			{
				hub->pt.state = 0;
				vsfsm_post_evt_pending(sm, VSFSM_EVT_DELAY_DONE);
			}
		}
		break;
	default:
		break;
	}
	return NULL;
}

static void *vsfusbh_hub_probe(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, struct vsfusbh_ifs_t *ifs,
		const struct vsfusbh_device_id_t *id)
{
	struct vsfusbh_hub_t *hub;

	if (hub_class_check(dev))
		return NULL;

	hub = vsf_bufmgr_malloc(sizeof(struct vsfusbh_hub_t));
	if (NULL == hub)
		return NULL;
	memset(hub, 0, sizeof(struct vsfusbh_hub_t));

	hub->urb = vsfusbh_alloc_urb(usbh);
	if (hub->urb == NULL)
	{
		vsf_bufmgr_free(hub);
		return NULL;
	}

	hub->usbh = usbh;
	hub->dev = dev;
	hub->ifs = ifs;
	hub->urb->hcddev = &dev->hcddev;
	hub->urb->timeout = 200;
	hub->sm.init_state.evt_handler = vsfusbh_hub_evt_handler_init;
	hub->sm.user_data = hub;
	vsfsm_init(&hub->sm);
	return hub;
}

static void vsfusbh_hub_disconnect(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, void *priv)
{
	struct vsfusbh_hub_t *hub = priv;

	if (hub->urb != NULL)
		vsfusbh_free_urb(usbh, &hub->urb);

	if (hub->polltimer != NULL)
		vsftimer_free(hub->polltimer);
	vsfsm_fini(&hub->sm);
	vsf_bufmgr_free(hub);
}

static const struct vsfusbh_device_id_t vsfusbh_hub_id_table[] =
{
	{
		.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		.bInterfaceClass = USB_CLASS_HUB,
	},
	{0},
};

const struct vsfusbh_class_drv_t vsfusbh_hub_drv =
{
	.name = "hub",
	.id_table = vsfusbh_hub_id_table,
	.probe = vsfusbh_hub_probe,
	.disconnect = vsfusbh_hub_disconnect,
};

#ifndef VSFCFG_FUNC_USBH_TINY
static struct vsfusbh_hub_t *
vsfusbh_hub_dev_gethub(struct vsfusbh_device_t *hub_dev)
{
	struct vsfusbh_hub_t *hub = NULL;
	struct vsfusbh_ifs_t *ifs = hub_dev->config.ifs;

	for (int i = 0; i < hub_dev->config.num_of_ifs; i++, ifs++)
	{
		if (ifs->drv == &vsfusbh_hub_drv)
		{
			hub = ifs->priv;
			break;
		}
	}
	return hub;
}

bool vsfusbh_hub_dev_is_reset(struct vsfusbh_device_t *dev)
{
	struct vsfusbh_device_t *hub_dev = dev->parent;
	struct vsfusbh_hub_t *hub = vsfusbh_hub_dev_gethub(hub_dev);

	if (hub != NULL)
	{
		for (uint16_t count = 0; count < dimof(hub_dev->children); count++)
		{
			if (hub_dev->children[count] == dev)
			{
				return (hub->reset_mask & (1 << count)) != 0;
			}
		}
	}
	return false;
}

vsf_err_t vsfusbh_hub_reset_dev(struct vsfusbh_device_t *dev)
{
	struct vsfusbh_device_t *hub_dev = dev->parent;
	struct vsfusbh_hub_t *hub = vsfusbh_hub_dev_gethub(hub_dev);

	if (hub != NULL)
	{
		for (uint16_t count = 0; count < dimof(hub_dev->children); count++)
		{
			if (hub_dev->children[count] == dev)
			{
				if (!(hub->reset_mask & (1 << count)))
				{
					hub->reset_mask |= 1 << count;
					vsfsm_sem_post(&hub->poll_sem);
				}
				break;
			}
		}
		return VSFERR_NONE;
	}
	return VSFERR_FAIL;
}

vsf_err_t vsfusbh_hub_reconnect_dev(struct vsfusbh_device_t *dev)
{
	struct vsfusbh_device_t *hub_dev = dev->parent;
	struct vsfusbh_hub_t *hub = vsfusbh_hub_dev_gethub(hub_dev);

	if (hub != NULL)
	{
		for (uint16_t count = 0; count < dimof(hub_dev->children); count++)
		{
			if (hub_dev->children[count] == dev)
			{
				if (!(hub->reconnect_mask & (1 << count)))
				{
					hub->reconnect_mask |= 1 << count;
					vsfsm_sem_post(&hub->poll_sem);
				}
				break;
			}
		}
		return VSFERR_NONE;
	}
	return VSFERR_FAIL;
}

#endif

