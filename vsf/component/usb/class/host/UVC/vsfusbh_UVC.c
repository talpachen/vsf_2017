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
#include "vsfusbh_UVC.h"

enum uav_evt_t
{
	UAV_RESET_STREAM_PARAM = VSFSM_EVT_USER_LOCAL + 100,
	UAV_ISO_ENABLE,
	UAV_ISO_DISABLE,
};

enum uav_request_t
{
	RC_UNDEFINED = 0x00,
	SET_CUR = 0x01,
	GET_CUR = 0x81,
	GET_MIN = 0x82,
	GET_MAX = 0x83,
	GET_RES = 0x84,
	GET_LEN = 0x85,
	GET_INFO = 0x86,
	GET_DEF = 0x87,
};

struct vsfusbh_uvc_t
{
	struct vsfusbh_t *usbh;
	struct vsfusbh_device_t *dev;

	struct vsfsm_t ctrl_sm;
	struct vsfsm_pt_t ctrl_pt;
	struct vsfsm_t video_sm;
	//struct vsfsm_t audio_sm;

	struct vsfhcd_urb_t *ctrl_urb;
	struct vsfhcd_urb_t *video_urb;
	//struct vsfhcd_urb_t *audio_urb;
		
	struct vsfusbh_uvc_param_t set_param;
};

void (*vsfusbh_uvc_report)(void *dev, struct vsfusbh_uvc_param_t *param,
		uint8_t type, uint8_t *data, uint32_t size) = NULL;

static vsf_err_t uvc_ctrl_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_uvc_t *uvc = pt->user_data;
	struct vsfhcd_urb_t *urb = uvc->ctrl_urb;

	vsfsm_pt_begin(pt);

	if (uvc->set_param.video_enable)
	{
		// commit param
		if (!vsfusbh_alloc_urb_buffer(urb, sizeof(struct video_probe_commit_ctrl_t))) goto ret_fail;
		memcpy(urb->transfer_buffer, &uvc->set_param.video_ctrl, sizeof(struct video_probe_commit_ctrl_t));
		urb->pipe = usb_sndctrlpipe(urb->hcddev, 0);
#if 1
		err =  vsfusbh_control_msg(uvc->usbh, urb, USB_TYPE_CLASS |USB_RECIP_INTERFACE | USB_DIR_OUT, SET_CUR,
				0x0100, 0x0001);
		if (err) goto ret_fail;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (urb->status != URB_OK) goto ret_fail;
#endif
		err =  vsfusbh_control_msg(uvc->usbh, urb, USB_TYPE_CLASS |USB_RECIP_INTERFACE | USB_DIR_OUT, SET_CUR,
				0x0200, 0x0001);
		if (err) goto ret_fail;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (urb->status != URB_OK) goto ret_fail;
		
		vsfusbh_free_urb_buffer(urb);
		
		// set interfaces
		urb->transfer_buffer = NULL;
		urb->transfer_length = 0;
		err = vsfusbh_set_interface(uvc->usbh, urb, uvc->set_param.video_interface,
				uvc->set_param.video_interface_altr_setting);
		if (err != VSFERR_NONE)
			return err;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (urb->status != URB_OK)
			return VSFERR_FAIL;

		// enable video
		vsfsm_post_evt_pending(&uvc->video_sm, UAV_ISO_ENABLE);
	}
	else
	{
		urb->transfer_buffer = NULL;
		urb->transfer_length = 0;
		err = vsfusbh_set_interface(uvc->usbh, urb, uvc->set_param.video_interface, 0);
		if (err != VSFERR_NONE)
			return err;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (urb->status != URB_OK)
			return VSFERR_FAIL;
	}
	vsfsm_pt_end(pt);

	return VSFERR_NONE;

ret_fail:
	vsfusbh_free_urb_buffer(urb);
	return VSFERR_FAIL;
}

static struct vsfsm_state_t *uvc_evt_handler_video(struct vsfsm_t *sm,
		vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_uvc_t *uvc = sm->user_data;
	struct vsfhcd_urb_t *urb = uvc->video_urb;

	switch (evt)
	{
	case VSFSM_EVT_INIT:
		break;
	case UAV_ISO_ENABLE:
		if (!urb->transfer_buffer)
		{
			if (uvc->dev->ep_mps_in[uvc->set_param.video_iso_ep] > uvc->set_param.video_iso_packet_len)
				uvc->dev->ep_mps_in[uvc->set_param.video_iso_ep] = uvc->set_param.video_iso_packet_len;			
			
			if (!vsfusbh_alloc_urb_buffer(urb, uvc->set_param.video_iso_packet_len * 3))
				break;
			urb->pipe = usb_rcvisocpipe(urb->hcddev, uvc->set_param.video_iso_ep);
			urb->transfer_flags |= USB_ISO_ASAP;
			urb->number_of_packets = 1;
			urb->iso_frame_desc[0].offset = 0;
			urb->iso_frame_desc[0].length = uvc->set_param.video_iso_packet_len * 3;
			err = vsfusbh_submit_urb(uvc->usbh, urb);
			if (err != VSFERR_NONE)
				break;
		}
		else
			vsfsm_post_evt_pending(sm, VSFSM_EVT_URB_COMPLETE);
		break;
	case VSFSM_EVT_URB_COMPLETE:
		if (uvc->set_param.video_enable)
		{
			if (urb->status == URB_OK)
				vsfusbh_uvc_report(uvc, &uvc->set_param, VSFUSBH_UVC_REPORT_TYPE_VIDEO,
						urb->transfer_buffer, urb->actual_length);
			err = vsfusbh_relink_urb(uvc->usbh, urb);
			if (err != VSFERR_NONE)
				break;
		}
		break;
	default:
		break;
	}
	return NULL;
}

static void *vsfusbh_uvc_probe(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, struct vsfusbh_ifs_t *ifs, 
		const struct vsfusbh_device_id_t *id)
{
	struct usb_interface_desc_t *ifs_desc = ifs->alt[ifs->cur_alt].ifs_desc;
	struct vsfusbh_uvc_t *uvc;

	// only probe interface 0
	if ((ifs_desc->bInterfaceNumber != 0) || !vsfusbh_uvc_report)
		return NULL;
	
	uvc = vsf_bufmgr_malloc(sizeof(struct vsfusbh_uvc_t));
	if (!uvc)
		return NULL;
	memset(uvc, 0, sizeof(struct vsfusbh_uvc_t));

	uvc->ctrl_urb = vsfusbh_alloc_urb(usbh);
	uvc->video_urb = vsfusbh_alloc_urb(usbh);
	//uvc->audio_urb = vsfusbh_alloc_urb(usbh);
	if (!uvc->ctrl_urb || !uvc->video_urb)
		goto free_and_fail;

	uvc->set_param.vid = dev->device_desc->idVendor;
	uvc->set_param.pid = dev->device_desc->idProduct;
	vsfusbh_uvc_report(uvc, &uvc->set_param, 0, NULL, 0);

	uvc->usbh = usbh;
	uvc->dev = dev;
	
	uvc->ctrl_urb->hcddev = &dev->hcddev;
	uvc->ctrl_urb->notifier_sm = &uvc->ctrl_sm;
	uvc->ctrl_urb->timeout = 200;
	
	uvc->video_urb->hcddev = &dev->hcddev;
	uvc->video_urb->timeout = 200;
	uvc->video_urb->notifier_sm = &uvc->video_sm;

	uvc->ctrl_pt.thread = uvc_ctrl_thread;
	uvc->ctrl_pt.user_data = uvc;
	uvc->ctrl_pt.sm = &uvc->ctrl_sm;
	uvc->ctrl_pt.state = 0;

	uvc->video_sm.init_state.evt_handler = uvc_evt_handler_video;
	uvc->video_sm.user_data = uvc;

	vsfsm_pt_init(&uvc->ctrl_sm, &uvc->ctrl_pt);
	vsfsm_init(&uvc->video_sm);
	return uvc;
	
free_and_fail:
	if (uvc->ctrl_urb != NULL)
		vsfusbh_free_urb(usbh, &uvc->ctrl_urb);
	if (uvc->video_urb != NULL)
		vsfusbh_free_urb(usbh, &uvc->video_urb);
	vsf_bufmgr_free(uvc);
	return NULL;
}

static void vsfusbh_uvc_disconnect(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, void *priv)
{
	struct vsfusbh_uvc_t *uvc = priv;

	if (uvc->ctrl_urb)
		vsfusbh_free_urb(usbh, &uvc->ctrl_urb);
	if (uvc->video_urb)
		vsfusbh_free_urb(usbh, &uvc->video_urb);

	vsfsm_fini(&uvc->ctrl_sm);
	vsfsm_fini(&uvc->video_sm);
	vsf_bufmgr_free(uvc);
}

vsf_err_t vsfusbh_uvc_set(void *dev_data, struct vsfusbh_uvc_param_t *param)
{
	//struct vsfusbh_uvc_t *uvc = (struct vsfusbh_uvc_t *)dev_data;

	//uvc->set_param = *param;
	//vsfsm_post_evt_pending(&uvc->ctrl_sm, UAV_RESET_STREAM_PARAM);
	return VSFERR_NONE;
}

const struct vsfusbh_device_id_t vsfusbh_uvc_id_table[] =
{
	{
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT,
		.idVendor = 0x041e,
		.idProduct = 0x4087,
	},
	{
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT,
		.idVendor = 0x0c45,
		.idProduct = 0x6341,
	},
	{
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT,
		.idVendor = 0x5149,
		.idProduct = 0x13d3,
	},
	{0},
};

const struct vsfusbh_class_drv_t vsfusbh_uvc_drv =
{
	.name = "uvc",
	.id_table = vsfusbh_uvc_id_table,
	.probe = vsfusbh_uvc_probe,
	.disconnect = vsfusbh_uvc_disconnect,
};

