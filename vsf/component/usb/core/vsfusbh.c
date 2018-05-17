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

#define USB_MAX_DEVICE				127

struct vsfusbh_class_drv_list
{
	const struct vsfusbh_class_drv_t *drv;
	struct sllist list;
};

struct vsfusbh_device_t *vsfusbh_alloc_device(struct vsfusbh_t *usbh)
{
	struct vsfusbh_device_t *dev;

	dev = vsf_bufmgr_malloc(sizeof(struct vsfusbh_device_t));
	if (NULL == dev)
		return NULL;
	memset(dev, 0, sizeof(struct vsfusbh_device_t));

	if (usbh->hcd.drv->alloc_device &&
		usbh->hcd.drv->alloc_device(&usbh->hcd, &dev->hcddev))
		goto free_dev;

#ifndef VSFCFG_FUNC_USBH_TINY
	dev->hcddev.devnum = mskarr_ffz(usbh->device_bitmap, USB_MAX_DEVICE);
	if (dev->hcddev.devnum == 0)
		goto free_dev;
	mskarr_set(usbh->device_bitmap, dev->hcddev.devnum);
#else
	dev->devnum = 2;
#endif
	return dev;
free_dev:
	vsf_bufmgr_free(dev);
	return NULL;
}

static void vsfusbh_free_desc(struct vsfusbh_device_t *dev)
{
	if (dev->device_desc != NULL)
	{
		vsf_bufmgr_free(dev->device_desc);
		dev->device_desc = NULL;
	}
	if (dev->config_desc != NULL)
	{
		vsf_bufmgr_free(dev->config_desc);
		dev->config_desc = NULL;
	}
}

void vsfusbh_clean_device(struct vsfusbh_t *usbh, struct vsfusbh_device_t *dev)
{
	struct vsfusbh_cfg_t *config = &dev->config;
	struct vsfusbh_ifs_t *ifs = config->ifs;

	if (usbh->hcd.drv->free_device)
		usbh->hcd.drv->free_device(&usbh->hcd, &dev->hcddev);
	for (uint8_t i = 0; i < config->num_of_ifs; i++, ifs++)
		vsf_bufmgr_free(ifs->alt);
	vsf_bufmgr_free(config->ifs);
	vsfusbh_free_desc(dev);
}

struct vsfhcd_urb_t *vsfusbh_alloc_urb(struct vsfusbh_t *usbh)
{
	return usbh->hcd.drv->alloc_urb(&usbh->hcd);
}

void vsfusbh_free_urb(struct vsfusbh_t *usbh, struct vsfhcd_urb_t **purb)
{
	usbh->hcd.drv->free_urb(&usbh->hcd, *purb);
	*purb = NULL;
}

void vsfusbh_free_urb_buffer(struct vsfhcd_urb_t *urb)
{
	if (urb->transfer_buffer && (urb->transfer_flags & URB_BUFFER_DYNALLOC))
		vsf_bufmgr_free(urb->transfer_buffer);
	urb->transfer_buffer = NULL;
	urb->transfer_length = 0;
	urb->transfer_flags &= ~URB_BUFFER_DYNALLOC;
}

uint8_t *vsfusbh_alloc_urb_buffer(struct vsfhcd_urb_t *urb, uint16_t size)
{
	vsfusbh_free_urb_buffer(urb);
	urb->transfer_buffer = vsf_bufmgr_malloc(size);
	urb->transfer_length = size;
	urb->transfer_flags |= URB_BUFFER_DYNALLOC;
	return urb->transfer_buffer;
}

static const struct vsfusbh_device_id_t *vsfusbh_match_id(
		struct vsfusbh_device_t *dev, struct vsfusbh_ifs_t *ifs,
		const struct vsfusbh_device_id_t *id)
{
	struct usb_interface_desc_t *ifs_desc;

	if (id == NULL)
		return NULL;

	for (; id->idVendor || id->bDeviceClass || id->bInterfaceClass; id++)
	{
#ifndef VSFCFG_FUNC_USBH_TINY
		if ((id->match_flags & USB_DEVICE_ID_MATCH_VENDOR) &&
				id->idVendor != dev->device_desc->idVendor)
			continue;
		if ((id->match_flags & USB_DEVICE_ID_MATCH_PRODUCT) &&
				id->idProduct != dev->device_desc->idProduct)
			continue;
		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_LO) &&
				(id->bcdDevice_lo > dev->device_desc->bcdDevice))
			continue;
		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_HI) &&
				(id->bcdDevice_hi < dev->device_desc->bcdDevice))
			continue;
		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_CLASS) &&
				(id->bDeviceClass != dev->device_desc->bDeviceClass))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_SUBCLASS) &&
				(id->bDeviceSubClass!= dev->device_desc->bDeviceSubClass))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_PROTOCOL) &&
				(id->bDeviceProtocol != dev->device_desc->bDeviceProtocol))
			continue;
#endif
		ifs_desc = ifs->alt[ifs->cur_alt].ifs_desc;
		if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_CLASS) &&
				(id->bInterfaceClass != ifs_desc->bInterfaceClass))
			continue;
		if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_SUBCLASS) &&
				(id->bInterfaceSubClass != ifs_desc->bInterfaceSubClass))
			continue;
		if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_PROTOCOL) &&
				(id->bInterfaceProtocol != ifs_desc->bInterfaceProtocol))
			continue;
		return id;
	}
	return NULL;
}

static const struct vsfusbh_class_drv_t *vsfusbh_match_intrface_driver(
		struct vsfusbh_t *usbh, struct vsfusbh_device_t *dev,
		struct vsfusbh_ifs_t *ifs, const struct vsfusbh_device_id_t **id)
{
	struct sllist *list = &usbh->drv_list;
	struct vsfusbh_class_drv_list *drv_list;
	const struct vsfusbh_class_drv_t *drv;

	while (list->next)
	{
		list = list->next;
		drv_list = sllist_get_container(list, struct vsfusbh_class_drv_list, list);
		drv = drv_list->drv;

		*id = drv->id_table;
		if (*id)
		{
			for (uint8_t i = 0; i < ifs->num_of_alt; i++)
			{
				ifs->cur_alt = i;
				*id = vsfusbh_match_id(dev, ifs, *id);
				if (*id) goto end;
			}
		}
	}
	drv = NULL;
end:
	ifs->cur_alt = 0;
	return drv;
}

static vsf_err_t vsfusbh_find_intrface_driver(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, struct vsfusbh_ifs_t *ifs)
{
	const struct vsfusbh_class_drv_t *drv;
	const struct vsfusbh_device_id_t *id;
	void *priv = NULL;

	do
	{
		drv = vsfusbh_match_intrface_driver(usbh, dev, ifs, &id);
		if ((drv != NULL) && (id != NULL))
		{
			priv = drv->probe(usbh, dev, ifs, id);
			if (priv)
			{
				vsfdbg_printf("%s: vid%04X pid%04X interface%d" VSFCFG_DEBUG_LINEEND,
							drv->name,
							dev->device_desc->idVendor, dev->device_desc->idProduct,
							ifs->alt[ifs->cur_alt].ifs_desc->bInterfaceNumber);
				ifs->priv = priv;
				ifs->drv = drv;
				return VSFERR_NONE;
			}
			return VSFERR_FAIL;
		}
	} while (drv != NULL);
	return VSFERR_FAIL;
}

void vsfusbh_remove_interface(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, struct vsfusbh_ifs_t *ifs)
{
	const struct vsfusbh_class_drv_t *drv = ifs->drv;
	if (drv)
	{
		vsfdbg_printf("%s: remove interface" VSFCFG_DEBUG_LINEEND, drv->name);
		drv->disconnect(usbh, dev, ifs->priv);
		ifs->drv = NULL;
		ifs->priv = NULL;
		ifs->cur_alt = 0;
	}
}

void vsfusbh_disconnect_device(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t **pdev)
{
	struct vsfusbh_device_t *dev = *pdev;
	uint8_t i;

	if (!dev)
		return;

	*pdev = NULL;

	if (dev->config.num_of_ifs)
	{
		struct vsfusbh_ifs_t *ifs = dev->config.ifs;
		for (i = 0; i < dev->config.num_of_ifs; i++, ifs++)
			vsfusbh_remove_interface(usbh, dev, ifs);
	}
#ifndef VSFCFG_FUNC_USBH_TINY
	for (i = 0; i < USB_MAX_CHILDREN; i++)
	{
		if (dev->children[i])
			vsfusbh_disconnect_device(usbh, &dev->children[i]);
	}
#endif
	vsfusbh_clean_device(usbh, dev);
	
#ifndef VSFCFG_FUNC_USBH_TINY
	if (dev->hcddev.devnum != 0)
		mskarr_clr(usbh->device_bitmap, dev->hcddev.devnum);
#endif
	vsf_bufmgr_free(dev);
}

#ifndef VSFCFG_FUNC_USBH_TINY
#define KERNEL_REL	0
#define KERNEL_VER	0

/* usb 2.0 root hub device descriptor */
static const uint8_t usb_rh_dev_descriptor[18] =
{
	0x12,       /*  __u8  bLength; */
	0x01,       /*  __u8  bDescriptorType; Device */
	0x00, 0x00, /*  __le16 bcdUSB: to be patched */

	0x09,	    /*  __u8  bDeviceClass; HUB_CLASSCODE */
	0x00,	    /*  __u8  bDeviceSubClass; */
	0x00,       /*  __u8  bDeviceProtocol; [ usb 2.0 no TT ] */
	0x40,       /*  __u8  bMaxPacketSize0; 64 Bytes */

	0x6b, 0x1d, /*  __le16 idVendor; Linux Foundation 0x1d6b */
	0x02, 0x00, /*  __le16 idProduct; device 0x0002 */
	KERNEL_VER, KERNEL_REL, /*  __le16 bcdDevice */

	0x00,       /*  __u8  iManufacturer; */
	0x00,       /*  __u8  iProduct; */
	0x00,       /*  __u8  iSerialNumber; */
	0x01        /*  __u8  bNumConfigurations; */
};

/* no usb 2.0 root hub "device qualifier" descriptor: one speed only */

static const uint8_t rh_config_descriptor[] =
{
	/* one configuration */
	0x09,       /*  __u8  bLength; */
	0x02,       /*  __u8  bDescriptorType; Configuration */
	0x19, 0x00, /*  __le16 wTotalLength; */
	0x01,       /*  __u8  bNumInterfaces; (1) */
	0x01,       /*  __u8  bConfigurationValue; */
	0x00,       /*  __u8  iConfiguration; */
	0xc0,       /*  __u8  bmAttributes;
				 Bit 7: must be set,
				     6: Self-powered,
				     5: Remote wakeup,
				     4..0: resvd */
	0x00,       /*  __u8  MaxPower; */

	/* USB 1.1:
	 * USB 2.0, single TT organization (mandatory):
	 *	one interface, protocol 0
	 *
	 * USB 2.0, multiple TT organization (optional):
	 *	two interfaces, protocols 1 (like single TT)
	 *	and 2 (multiple TT mode) ... config is
	 *	sometimes settable
	 *	NOT IMPLEMENTED
	 */

	/* one interface */
	0x09,       /*  __u8  if_bLength; */
	0x04,       /*  __u8  if_bDescriptorType; Interface */
	0x00,       /*  __u8  if_bInterfaceNumber; */
	0x00,       /*  __u8  if_bAlternateSetting; */
	0x01,       /*  __u8  if_bNumEndpoints; */
	0x09,       /*  __u8  if_bInterfaceClass; HUB_CLASSCODE */
	0x00,       /*  __u8  if_bInterfaceSubClass; */
	0x00,       /*  __u8  if_bInterfaceProtocol; [usb1.1 or single tt] */
	0x00,       /*  __u8  if_iInterface; */

	/* one endpoint (status change endpoint) */
	0x07,       /*  __u8  ep_bLength; */
	0x05,       /*  __u8  ep_bDescriptorType; Endpoint */
	0x81,       /*  __u8  ep_bEndpointAddress; IN Endpoint 1 */
	0x03,       /*  __u8  ep_bmAttributes; Interrupt */
	(USB_MAX_CHILDREN + 1 + 7) / 8, 0x00, /*  __le16 ep_wMaxPacketSize; 1 + (MAX_ROOT_PORTS / 8) */
	0xff        /*  __u8  ep_bInterval; (255ms -- usb 2.0 spec) */
};

static void vsfusbh_rh_complete_event(void *param)
{
	vsfsm_post_evt_pending((struct vsfsm_t *)param, VSFSM_EVT_URB_COMPLETE);
}

static void vsfusbh_rh_urb_complete(struct vsfhcd_urb_t *urb, int16_t result)
{
	urb->status = result;
	vsftimer_create_cb(1, 1, vsfusbh_rh_complete_event, urb->notifier_sm);
}

static vsf_err_t vsfusbh_rh_submit_urb(struct vsfusbh_t *usbh,
		struct vsfhcd_urb_t *urb)
{
	uint16_t typeReq, wValue, wLength;
	struct usb_ctrlrequest_t *cmd = &urb->setup_packet;
	uint32_t pipe = urb->pipe;
	uint8_t data[32];
	int16_t len = 0;

	if (usb_pipeint(pipe))
	{
		// WARNING: not support int transfer for HUB
		return VSFERR_NOT_SUPPORT;
	}

	urb->actual_length = 0;

	typeReq = (cmd->bRequestType << 8) | cmd->bRequest;
	wValue = cmd->wValue;
	wLength = cmd->wLength;

	if (wLength > urb->transfer_length)
		goto error;

	switch (typeReq)
	{
	case DeviceRequest | USB_REQ_GET_STATUS:
		data[0] = 1;
		data[1] = 0;
		len = 2;
		break;
	case DeviceOutRequest | USB_REQ_CLEAR_FEATURE:
	case DeviceOutRequest | USB_REQ_SET_FEATURE:
		break;
	case DeviceRequest | USB_REQ_GET_CONFIGURATION:
		data[0] = 1;
		len = 1;
		break;
	case DeviceOutRequest | USB_REQ_SET_CONFIGURATION:
		break;
	case DeviceRequest | USB_REQ_GET_DESCRIPTOR:
		switch ((wValue & 0xff00) >> 8)
		{
		case USB_DT_DEVICE:
			memcpy(data, usb_rh_dev_descriptor, sizeof(usb_rh_dev_descriptor));
			switch (urb->hcddev->speed)
			{
			case USB_SPEED_LOW:
			case USB_SPEED_FULL:
				((struct usb_device_descriptor_t *)data)->bcdUSB = SYS_TO_LE_U16(0x0110);
				break;
			case USB_SPEED_HIGH:
				((struct usb_device_descriptor_t *)data)->bcdUSB = SYS_TO_LE_U16(0x0200);
				break;
			//case USB_SPEED_VARIABLE:
			//	break;
			//case USB_SPEED_SUPER:
			//	break;
			default:
				goto error;
			}
			len = sizeof(usb_rh_dev_descriptor);
			break;
		case USB_DT_CONFIG:
			len = sizeof(rh_config_descriptor);
			memcpy(data, rh_config_descriptor, len);
			break;
		default:
			goto error;
		}
		break;
	case DeviceRequest | USB_REQ_GET_INTERFACE:
		data[0] = 0;
		len = 1;
		break;
	case DeviceOutRequest | USB_REQ_SET_INTERFACE:
		break;
	case DeviceOutRequest | USB_REQ_SET_ADDRESS:
		break;
	case EndpointRequest | USB_REQ_GET_STATUS:
		data[0] = 0;
		data[1] = 0;
		len = 2;
		break;
	case EndpointOutRequest | USB_REQ_CLEAR_FEATURE:
	case EndpointOutRequest | USB_REQ_SET_FEATURE:
		break;
	default:
		len = usbh->hcd.drv->rh_control(&usbh->hcd, urb);
		if (len < 0)
			goto error;
		goto complete;
	}

	if (len > 0)
	{
		if (urb->transfer_length < len)
			len = urb->transfer_length;
		urb->actual_length = len;

		memcpy(urb->transfer_buffer, data, len);
	}

complete:
	vsfusbh_rh_urb_complete(urb, URB_OK);
	return VSFERR_NONE;

error:
	urb->status = URB_FAIL;
	return VSFERR_FAIL;
}
#endif

vsf_err_t vsfusbh_submit_urb(struct vsfusbh_t *usbh, struct vsfhcd_urb_t *urb)
{
	struct vsfusbh_device_t *dev =
				container_of(urb->hcddev, struct vsfusbh_device_t, hcddev);

	if (usb_pipein(urb->pipe))
		urb->packet_size = dev->ep_mps_in[usb_pipeendpoint(urb->pipe)];
	else
		urb->packet_size = dev->ep_mps_out[usb_pipeendpoint(urb->pipe)];
	
#ifndef VSFCFG_FUNC_USBH_TINY
	if (dev == usbh->rh_dev)
		return vsfusbh_rh_submit_urb(usbh, urb);
	else
#endif
		return usbh->hcd.drv->submit_urb(&usbh->hcd, urb);
}

vsf_err_t vsfusbh_relink_urb(struct vsfusbh_t *usbh, struct vsfhcd_urb_t *urb)
{
	return usbh->hcd.drv->relink_urb(&usbh->hcd, urb);
}

vsf_err_t vsfusbh_control_msg(struct vsfusbh_t *usbh, struct vsfhcd_urb_t *urb,
		uint8_t bRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex)
{
	urb->timeout = 200;

	urb->setup_packet.bRequestType = bRequestType;
	urb->setup_packet.bRequest = bRequest;
	urb->setup_packet.wValue = wValue;
	urb->setup_packet.wIndex = wIndex;
	urb->setup_packet.wLength = urb->transfer_length;

	return vsfusbh_submit_urb(usbh, urb);
}

vsf_err_t vsfusbh_set_address(struct vsfusbh_t *usbh,
		struct vsfhcd_urb_t *urb)
{
	urb->pipe = usb_snddefctrl(urb->hcddev);
	return vsfusbh_control_msg(usbh, urb, USB_DIR_OUT, USB_REQ_SET_ADDRESS,
			urb->hcddev->devnum, 0);
}
vsf_err_t vsfusbh_get_descriptor(struct vsfusbh_t *usbh,
		struct vsfhcd_urb_t *urb, uint8_t type, uint8_t index)
{
	urb->pipe = usb_rcvctrlpipe(urb->hcddev, 0);
	return vsfusbh_control_msg(usbh, urb, USB_DIR_IN, USB_REQ_GET_DESCRIPTOR,
			(type << 8) + index, index);
}
vsf_err_t vsfusbh_get_class_descriptor(struct vsfusbh_t *usbh,
		struct vsfhcd_urb_t *urb, uint16_t ifnum, uint8_t type, uint8_t id)
{
	urb->pipe = usb_rcvctrlpipe(urb->hcddev, 0);
	return vsfusbh_control_msg(usbh, urb, USB_RECIP_INTERFACE | USB_DIR_IN,
			USB_REQ_GET_DESCRIPTOR, (type << 8) + id, ifnum);
}
vsf_err_t vsfusbh_set_configuration(struct vsfusbh_t *usbh,
		struct vsfhcd_urb_t *urb, uint8_t configuration)
{
	urb->pipe = usb_sndctrlpipe(urb->hcddev, 0);
	return vsfusbh_control_msg(usbh, urb, USB_DIR_OUT,
			USB_REQ_SET_CONFIGURATION, configuration, 0);
}
vsf_err_t vsfusbh_set_interface(struct vsfusbh_t *usbh,
		struct vsfhcd_urb_t *urb, uint16_t interface, uint16_t alternate)
{
	urb->pipe = usb_sndctrlpipe(urb->hcddev, 0);
	return vsfusbh_control_msg(usbh, urb, USB_RECIP_INTERFACE,
			USB_REQ_SET_INTERFACE, alternate, interface);
}

static vsf_err_t vsfusbh_parse_config(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev)
{
	struct usb_config_descriptor_t *config_desc = dev->config_desc;
	struct usb_interface_desc_t *ifs_desc;
	struct usb_endpoint_desc_t *ep_desc;
	struct vsfusbh_ifs_alt_t *alt;
	struct usb_descriptor_header_t *header =
				(struct usb_descriptor_header_t *)config_desc, *tmpheader;
	uint16_t size = config_desc->wTotalLength, len, tmpsize;
	struct vsfusbh_ifs_t *ifs;
	uint8_t ifnum, claiming;
	uint8_t epaddr;
	uint16_t epsize;

	if (header->bDescriptorType != USB_DT_CONFIG)
		return VSFERR_NONE;

	dev->config.num_of_ifs = config_desc->bNumInterfaces;
	len = dev->config.num_of_ifs * sizeof(*ifs);
	dev->config.ifs = vsf_bufmgr_malloc(len);
	if (!dev->config.ifs) return VSFERR_NOT_ENOUGH_RESOURCES;
	memset(dev->config.ifs, 0, len);
	ifs = dev->config.ifs;

	size -= header->bLength;
	header = (struct usb_descriptor_header_t *)((uint8_t *)header + header->bLength);

	alt = NULL;
	claiming = header->bDescriptorType == USB_DT_INTERFACE ? 1 : 0;
	ifnum = 0;
	tmpsize = size;
	tmpheader = header;
	while ((size > 0) && (size >= header->bLength))
	{
		switch (header->bDescriptorType)
		{
		case USB_DT_INTERFACE:
			ifs_desc = (struct usb_interface_desc_t *)header;
			if (!ifs->alt)
			{
				if (ifs_desc->bInterfaceNumber == ifnum)
					ifs->num_of_alt++;
				else
				{
				alloc_alt:
					len = ifs->num_of_alt * sizeof(*ifs->alt);
					ifs->alt = vsf_bufmgr_malloc(len);
					if (!ifs->alt) return VSFERR_NOT_ENOUGH_RESOURCES;
					memset(ifs->alt, 0, len);

					claiming = 2;
					alt = ifs->alt - 1;
					size = tmpsize;
					header = tmpheader;
					continue;
				}
			}
			else
			{
				if (ifs_desc->bInterfaceNumber == ifnum)
				{
					if (alt > ifs->alt)
						alt[-1].desc_size = (uint32_t)ifs_desc - (uint32_t)alt[-1].ifs_desc;
					(++alt)->ifs_desc = ifs_desc;
				}
				else
				{
				probe_alt:
					alt->desc_size = (uint32_t)ifs_desc - (uint32_t)alt->ifs_desc;
					alt = NULL;
					claiming = size > 0 ? 1 : 0;
					ifs++;
					ifnum++;
					tmpsize = size;
					tmpheader = header;
					continue;
				}
			}
			break;
		case USB_DT_ENDPOINT:
			ep_desc = (struct usb_endpoint_desc_t *)header;
			if (alt && !alt->ep_desc)
				alt->ep_desc = ep_desc;
			epaddr = ep_desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
			epsize = GET_LE_U16(&ep_desc->wMaxPacketSize_Lo);
			if (ep_desc->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
			{
				if (epsize > dev->ep_mps_in[epaddr])
					dev->ep_mps_in[epaddr] = epsize;
			}
			else
			{
				if (epsize > dev->ep_mps_out[epaddr])
					dev->ep_mps_out[epaddr] = epsize;
			}
			break;
		}

		size -= header->bLength;
		header = (struct usb_descriptor_header_t *)((uint8_t *)header + header->bLength);
		if (!size && claiming)
		{
			if (claiming == 1)
				goto alloc_alt;
			else
			{
				ifs_desc = (struct usb_interface_desc_t *)header;
				goto probe_alt;
			}
		}
	}

	// probe
	claiming = 0;
	ifs = dev->config.ifs;
	for (ifnum = 0; ifnum < dev->config.num_of_ifs; ifnum++, ifs++)
	{
		if (!vsfusbh_find_intrface_driver(usbh, dev, ifs))
			claiming++;
	}
	return claiming > 0 ? VSFERR_NONE : VSFERR_FAIL;
}

vsf_err_t vsfusbh_probe_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	struct vsfusbh_t *usbh = (struct vsfusbh_t *)pt->user_data;
	struct vsfhcd_urb_t *probe_urb = usbh->probe_urb;
	struct vsfusbh_device_t *dev = usbh->new_dev;
	vsf_err_t err;
	uint32_t len;

	vsfsm_pt_begin(pt);

	dev->devnum_temp = dev->hcddev.devnum;
	dev->hcddev.devnum = 0;
	dev->ep_mps_in[0] = 64;
	dev->ep_mps_out[0] = 64;

	probe_urb->hcddev = &dev->hcddev;
	probe_urb->notifier_sm = &usbh->sm;
	probe_urb->timeout = DEFAULT_TIMEOUT;

	// get 64 bytes device descriptor
	if (!vsfusbh_alloc_urb_buffer(probe_urb, 64)) goto ret_failure;
	err = vsfusbh_get_descriptor(usbh, probe_urb, USB_DT_DEVICE, 0);
	if (err) goto ret_error;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (probe_urb->status != URB_OK) goto ret_failure;
	dev->ep_mps_in[0] = dev->ep_mps_out[0] = ((uint8_t *)probe_urb->transfer_buffer)[7];
	vsfusbh_free_urb_buffer(probe_urb);

	if (vsfusbh_hub_reset_dev(dev) == VSFERR_NONE)
	{
		do
		{
			vsfsm_pt_delay(pt, 20);
		} while (vsfusbh_hub_dev_is_reset(dev));
	}

	// set address
	dev->hcddev.devnum = dev->devnum_temp;
	err = vsfusbh_set_address(usbh, probe_urb);
	if (err) goto ret_error;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (probe_urb->status != URB_OK) goto ret_failure;

	vsfsm_pt_delay(pt, 10);

	// get full device descriptor
	if (!vsfusbh_alloc_urb_buffer(probe_urb, sizeof(*dev->device_desc))) goto ret_failure;
	err = vsfusbh_get_descriptor(usbh, probe_urb, USB_DT_DEVICE, 0);
	if (err) goto ret_error;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (probe_urb->status != URB_OK) goto ret_failure;
	dev->device_desc = probe_urb->transfer_buffer;
	if (dev->device_desc->bNumConfigurations < 1) goto ret_failure;
	probe_urb->transfer_buffer = NULL;
	probe_urb->transfer_length = 0;
	probe_urb->transfer_flags = 0;

	for (dev->cur_config = 0;
			dev->cur_config < dev->device_desc->bNumConfigurations;
			dev->cur_config++)
	{
		// get 9 bytes configuration
		if (!vsfusbh_alloc_urb_buffer(probe_urb, 9)) goto ret_failure;
		err = vsfusbh_get_descriptor(usbh, probe_urb, USB_DT_CONFIG, dev->cur_config);
		if (err) goto ret_error;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (probe_urb->status != URB_OK) goto ret_failure;
		// get wTotalLength
		len = GET_U16_LSBFIRST(&((uint8_t *)(probe_urb->transfer_buffer))[2]);

		// get full configuation
		if (!vsfusbh_alloc_urb_buffer(probe_urb, len)) goto ret_failure;
		err = vsfusbh_get_descriptor(usbh, probe_urb, USB_DT_CONFIG, dev->cur_config);
		if (err) goto ret_error;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (probe_urb->status != URB_OK) goto ret_failure;
		// check wTotalLength
		len = GET_U16_LSBFIRST(&((uint8_t *)(probe_urb->transfer_buffer))[2]);
		if (probe_urb->actual_length != len) goto ret_failure;
		dev->config_desc = probe_urb->transfer_buffer;
		probe_urb->transfer_buffer = NULL;
		probe_urb->transfer_length = 0;
		probe_urb->transfer_flags = 0;

		err = vsfusbh_set_configuration(usbh, probe_urb, dev->config_desc->bConfigurationValue);
		if (err) goto ret_error;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (probe_urb->status != URB_OK) goto ret_failure;

		vsfsm_crit_init(&dev->ep0_crit, VSFSM_EVT_EP0_CRIT);
		err = vsfusbh_parse_config(usbh, dev);
		if (err < 0)
		{
			vsf_bufmgr_free(dev->config_desc);
			dev->config_desc = NULL;
			continue;
		}
		return VSFERR_NONE;
	}

	vsfsm_pt_end(pt);

ret_failure:
	err = VSFERR_FAIL;
ret_error:
	vsfusbh_free_urb_buffer(probe_urb);
	vsfusbh_free_desc(dev);
	return err;
}

static struct vsfsm_state_t *vsfusbh_probe_evt_handler(struct vsfsm_t *sm,
		vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_t *usbh = (struct vsfusbh_t *)sm->user_data;

	switch (evt)
	{
	case VSFSM_EVT_NEW_DEVICE:
		usbh->pt.thread = vsfusbh_probe_thread;
		usbh->pt.user_data = usbh;
		usbh->pt.state = 0;
		usbh->pt.sm = sm;
	default:
		err = usbh->pt.thread(&usbh->pt, evt);
		if (!err)
			usbh->new_dev = NULL;
		else if (err < 0)
		{
			vsfusbh_clean_device(usbh, usbh->new_dev);
			usbh->new_dev = NULL;
		}
		break;
	}
	return NULL;
}

static struct vsfsm_state_t *vsfusbh_init_evt_handler(struct vsfsm_t *sm,
		vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_t *usbh = (struct vsfusbh_t *)sm->user_data;

	switch (evt)
	{
	case VSFSM_EVT_ENTER:
	case VSFSM_EVT_EXIT:
		break;
	case VSFSM_EVT_INIT:
		sllist_init_node(usbh->drv_list);
		usbh->pt.thread = usbh->hcd.drv->init_thread;
		usbh->pt.user_data = usbh;
		usbh->pt.state = 0;
		usbh->pt.sm = sm;
	default:
		err = usbh->pt.thread(&usbh->pt, evt);
		if (VSFERR_NONE == err)
		{
			usbh->probe_urb = vsfusbh_alloc_urb(usbh);
			if (usbh->probe_urb != NULL)
			{
				sm->init_state.evt_handler = vsfusbh_probe_evt_handler;
#ifndef VSFCFG_FUNC_USBH_TINY
				usbh->rh_dev = vsfusbh_alloc_device(usbh);
				if (usbh->rh_dev != NULL)
				{
					usbh->rh_dev->hcddev.speed = usbh->hcd.rh_speed;
					usbh->new_dev = usbh->rh_dev;
					vsfsm_post_evt_pending(&usbh->sm, VSFSM_EVT_NEW_DEVICE);
					break;
				}
#else
				break;
#endif
			}
			usbh->pt.thread = NULL;
		}
		else if (err < 0)
			usbh->pt.thread = NULL;
		break;
	}
	return NULL;
}

vsf_err_t vsfusbh_init(struct vsfusbh_t *usbh)
{
	if (NULL == usbh->hcd.drv)
		return VSFERR_INVALID_PARAMETER;

	memset(&usbh->sm, 0, sizeof(usbh->sm));
#ifndef VSFCFG_FUNC_USBH_TINY
	usbh->device_bitmap[0] = 1;		// remove address 0
	usbh->device_bitmap[1] = 0;
	usbh->device_bitmap[2] = 0;
	usbh->device_bitmap[3] = 0;
#endif
	usbh->sm.init_state.evt_handler = vsfusbh_init_evt_handler;
	usbh->sm.user_data = (void*)usbh;
	return vsfsm_init(&usbh->sm);
}

vsf_err_t vsfusbh_fini(struct vsfusbh_t *usbh)
{
	// TODO
	return VSFERR_NONE;
}

vsf_err_t vsfusbh_register_driver(struct vsfusbh_t *usbh,
		const struct vsfusbh_class_drv_t *drv)
{
#ifndef VSFCFG_FUNC_USBH_TINY
	struct vsfusbh_class_drv_list *drv_list;

	drv_list = (struct vsfusbh_class_drv_list *)\
			vsf_bufmgr_malloc(sizeof(struct vsfusbh_class_drv_list));
	if (drv_list == NULL)
	{
		return VSFERR_FAIL;
	}
	memset(drv_list, 0, sizeof(struct vsfusbh_class_drv_list));
	drv_list->drv = drv;
	sllist_append(&usbh->drv_list, &drv_list->list);
#else
	usbh->drv = drv;
#endif
	return VSFERR_NONE;
}

vsf_err_t vsfusbh_get_extra_descriptor(uint8_t *buf, uint16_t size,
		uint8_t type, void **ptr)
{
	struct usb_descriptor_header_t *header;

	if ((buf == NULL) || (ptr == NULL))
		return VSFERR_FAIL;

	while (size >= sizeof(struct usb_descriptor_header_t))
	{
		header = (struct usb_descriptor_header_t *)buf;

		if (header->bLength < 2)
			break;

		if (header->bDescriptorType == type)
		{
			*ptr = header;
			return VSFERR_NONE;
		}

		if (size < header->bLength)
			break;

		buf += header->bLength;
		size -= header->bLength;
	}
	return VSFERR_FAIL;
}

