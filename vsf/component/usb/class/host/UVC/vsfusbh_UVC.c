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

#define UVC_PROBE_CRTL_DATA_SIZE 36

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
	struct vsfusbh_ifs_t *ifs;

	struct vsfsm_t init_sm;
	struct vsfsm_pt_t init_pt;
	struct vsfsm_t ctrl_sm;
	struct vsfsm_pt_t ctrl_pt;
	struct vsfsm_t video_sm;
	//struct vsfsm_t audio_sm;

	struct vsfhcd_urb_t *ctrl_urb;
	struct vsfhcd_urb_t *video_urb;
	//struct vsfhcd_urb_t *audio_urb;

	uint8_t *ctrl_urb_buf;
	uint8_t *video_urb_buf;
	//uint8_t *audio_urb_buf;

	struct vsfusbh_uvc_payload_t video_payload;
	//struct vsfusbh_uvc_payload_t audio_payload;

	struct vsfusbh_uvc_param_t set_param;
	struct vsfusbh_uvc_param_t cur_param;

	uint16_t video_iso_packet_len;
	//uint16_t audio_iso_packet_len;
	uint8_t video_iso_ep;
	//uint8_t audio_iso_ep;

};

void (*vsfusbh_uvc_report)(void *dev_data, struct vsfusbh_uvc_param_t *param,
		struct vsfusbh_uvc_payload_t *payload) = NULL;

static vsf_err_t uvc_init_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_uvc_t *uvc = pt->user_data;
	struct vsfhcd_urb_t *urb = uvc->ctrl_urb;

	vsfsm_pt_begin(pt);

	// reset interfaces 1 (video)
	urb->transfer_buffer = NULL;
	urb->transfer_length = 0;
	err = vsfusbh_set_interface(uvc->usbh, urb, 1, 0);
	if (err != VSFERR_NONE)
		return err;
	vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
	if (urb->status != URB_OK)
		return VSFERR_FAIL;

	vsfsm_pt_end(pt);

	return VSFERR_NONE;
}

static struct vsfsm_state_t *uvc_evt_handler_init(struct vsfsm_t *sm,
		vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_uvc_t *uvc = sm->user_data;

	switch (evt)
	{
	case VSFSM_EVT_INIT:
	case VSFSM_EVT_URB_COMPLETE:
	case VSFSM_EVT_DELAY_DONE:
		err = uvc->init_pt.thread(&uvc->init_pt, evt);
		if (err < 0)
		{
			// TODO
		}
		else if (err == 0)
		{
			uvc->ctrl_urb->notifier_sm = &uvc->ctrl_sm;

			if (vsfusbh_uvc_report)
				vsfusbh_uvc_report(uvc, &uvc->cur_param, NULL);
		}
		break;
	default:
		break;
	}
	return NULL;
}

static const uint8_t negotiate_temp[26] =
{
	0x5c, 0x45, 0x02, 0x01, 0x40, 0x42, 0x0f, 0x00, 
	0x91, 0x13, 0xa7, 0x33, 0x00, 0x00, 0x7c, 0x2a, 
	0xb0, 0xaf, 0x00, 0x20, 0x1c, 0x00, 0x00, 0x0c, 
	0x00, 0x00
};

static vsf_err_t uvc_ctrl_thread(struct vsfsm_pt_t *pt, vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_uvc_t *uvc = pt->user_data;
	struct vsfhcd_urb_t *urb = uvc->ctrl_urb;

	vsfsm_pt_begin(pt);

	if (uvc->set_param.video_enable)
	{
		// negotiate
		uvc->video_iso_packet_len = 1024;
		uvc->video_iso_ep = 1;

		// commit param
		urb->transfer_buffer = uvc->ctrl_urb_buf;
		memcpy(urb->transfer_buffer, negotiate_temp, 26);
		urb->transfer_length = 26;
		urb->pipe = usb_sndctrlpipe(urb->hcddev, 0);
		err =  vsfusbh_control_msg(uvc->usbh, urb,
				USB_TYPE_CLASS |USB_RECIP_INTERFACE | USB_DIR_OUT, SET_CUR,
				0x0200, 0x0001);
		if (err != VSFERR_NONE)
			return err;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (urb->status != URB_OK)
			return VSFERR_FAIL;

		// set interfaces
		urb->transfer_buffer = NULL;
		urb->transfer_length = 0;
		err = vsfusbh_set_interface(uvc->usbh, urb, 1, 4);
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
		err = vsfusbh_set_interface(uvc->usbh, urb, 1, 0);
		if (err != VSFERR_NONE)
			return err;
		vsfsm_pt_wfe(pt, VSFSM_EVT_URB_COMPLETE);
		if (urb->status != URB_OK)
			return VSFERR_FAIL;
	}

	vsf_bufmgr_free(uvc->ctrl_urb_buf);
	uvc->ctrl_urb_buf = NULL;

	memcpy(&uvc->cur_param, &uvc->set_param,
			sizeof(struct vsfusbh_uvc_param_t));
	vsfusbh_uvc_report(uvc, &uvc->cur_param, NULL);

	vsfsm_pt_end(pt);

	return VSFERR_NONE;
}

static struct vsfsm_state_t *uvc_evt_handler_ctrl(struct vsfsm_t *sm,
		vsfsm_evt_t evt)
{
	vsf_err_t err;
	struct vsfusbh_uvc_t *uvc = (struct vsfusbh_uvc_t *)sm->user_data;

	switch (evt)
	{
	case VSFSM_EVT_INIT:
		break;
	case UAV_RESET_STREAM_PARAM:
		uvc->ctrl_pt.state = 0;
		if (uvc->ctrl_urb_buf == NULL)
		{
			uvc->ctrl_urb_buf = vsf_bufmgr_malloc(UVC_PROBE_CRTL_DATA_SIZE);
			if (uvc->ctrl_urb_buf == NULL)
				return NULL;
		}
	case VSFSM_EVT_URB_COMPLETE:
	case VSFSM_EVT_DELAY_DONE:
		err = uvc->ctrl_pt.thread(&uvc->ctrl_pt, evt);
		if (err < 0)
		{
			// TODO

			vsf_bufmgr_free(uvc->ctrl_urb_buf);
			uvc->ctrl_urb_buf = NULL;
		}
		else if (err == 0)
		{
			vsf_bufmgr_free(uvc->ctrl_urb_buf);
			uvc->ctrl_urb_buf = NULL;
		}
		break;
	default:
		break;
	}
	return NULL;
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
		if (uvc->video_urb_buf == NULL)
		{
			uvc->video_urb_buf = vsf_bufmgr_malloc(uvc->video_iso_packet_len);
			if (uvc->video_urb_buf == NULL)
				return NULL;
			urb->transfer_buffer = uvc->video_urb_buf;
			urb->transfer_length = uvc->video_iso_packet_len;
			urb->pipe = usb_rcvisocpipe(urb->hcddev, uvc->video_iso_ep);
			urb->transfer_flags |= USB_ISO_ASAP;
			urb->number_of_packets = 1;
			urb->iso_frame_desc[0].offset = 0;
			urb->iso_frame_desc[0].length = uvc->video_iso_packet_len;
			err = vsfusbh_submit_urb(uvc->usbh, urb);
			if (err != VSFERR_NONE)
				goto error;
		}
		break;
	case UAV_ISO_DISABLE:
		// TODO
		break;
	case VSFSM_EVT_URB_COMPLETE:
		if (urb->status == URB_OK)
		{
			uvc->video_payload.len = urb->actual_length;
			vsfusbh_uvc_report(uvc, &uvc->cur_param, &uvc->video_payload);
		}
		else
		{
			goto error;
		}
		err = vsfusbh_relink_urb(uvc->usbh, urb);
		if (err != VSFERR_NONE)
			goto error;
		break;
	default:
		break;
	}
	return NULL;

error:
	vsf_bufmgr_free(uvc->video_urb_buf);
	uvc->video_urb_buf = NULL;
	return NULL;
}

static void *vsfusbh_uvc_probe(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, struct vsfusbh_ifs_t *ifs, 
		const struct vsfusbh_device_id_t *id)
{
	struct vsfusbh_uvc_t *uvc;

	uvc = vsf_bufmgr_malloc(sizeof(struct vsfusbh_uvc_t));
	if (!uvc)
		return NULL;
	memset(uvc, 0, sizeof(struct vsfusbh_uvc_t));

	uvc->ctrl_urb = vsfusbh_alloc_urb(usbh);
	if (uvc->ctrl_urb == NULL)
	{
		vsf_bufmgr_free(uvc);
		return NULL;
	}
	uvc->video_urb = vsfusbh_alloc_urb(usbh);
	if (uvc->video_urb == NULL)
	{
		vsfusbh_free_urb(usbh, &uvc->ctrl_urb);
		vsf_bufmgr_free(uvc);
		return NULL;
	}
	
	uvc->usbh = usbh;
	uvc->dev = dev;
	uvc->ifs = ifs;

	uvc->video_payload.type = VSFUSBH_UVC_PAYLOAD_VIDEO;
	uvc->video_payload.buf = uvc->video_urb_buf;

	uvc->ctrl_urb->hcddev = &dev->hcddev;
	uvc->ctrl_urb->timeout = 200;
	uvc->ctrl_urb->notifier_sm = &uvc->init_sm;
	uvc->video_urb->hcddev = &dev->hcddev;
	uvc->video_urb->timeout = 200;
	uvc->video_urb->notifier_sm = &uvc->video_sm;

	uvc->init_sm.init_state.evt_handler = uvc_evt_handler_init;
	uvc->init_sm.user_data = uvc;
	uvc->init_pt.thread = uvc_init_thread;
	uvc->init_pt.user_data = uvc;
	uvc->init_pt.sm = &uvc->init_sm;
	uvc->init_pt.state = 0;	

	uvc->ctrl_sm.init_state.evt_handler = uvc_evt_handler_ctrl;
	uvc->ctrl_sm.user_data = uvc;
	uvc->ctrl_pt.thread = uvc_ctrl_thread;
	uvc->ctrl_pt.user_data = uvc;
	uvc->ctrl_pt.sm = &uvc->ctrl_sm;
	uvc->ctrl_pt.state = 0;
	uvc->video_sm.init_state.evt_handler = uvc_evt_handler_video;
	uvc->video_sm.user_data = uvc;
	
	vsfsm_init(&uvc->init_sm);
	vsfsm_init(&uvc->ctrl_sm);
	vsfsm_init(&uvc->video_sm);
	return uvc;
}

static void vsfusbh_uvc_disconnect(struct vsfusbh_t *usbh,
		struct vsfusbh_device_t *dev, void *priv)
{
	struct vsfusbh_uvc_t *uvc = priv;

	if (uvc->ctrl_urb)
		vsfusbh_free_urb(usbh, &uvc->ctrl_urb);
	if (uvc->video_urb)
		vsfusbh_free_urb(usbh, &uvc->video_urb);

	vsfsm_fini(&uvc->init_sm);
	vsfsm_fini(&uvc->ctrl_sm);
	vsfsm_fini(&uvc->video_sm);
	vsf_bufmgr_free(uvc);
}

vsf_err_t vsfusbh_uvc_set(void *dev_data, struct vsfusbh_uvc_param_t *param)
{
	struct vsfusbh_uvc_t *uvc = (struct vsfusbh_uvc_t *)dev_data;

	uvc->set_param = *param;
	vsfsm_post_evt_pending(&uvc->ctrl_sm, UAV_RESET_STREAM_PARAM);
	return VSFERR_NONE;
}

const struct vsfusbh_device_id_t vsfusbh_uvc_id_table[] =
{
	{
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT,
		.idVendor = 0x0c45,
		.idProduct = 0x6341,
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

