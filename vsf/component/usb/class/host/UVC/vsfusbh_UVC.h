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

#ifndef __VSFUSBH_UVC_H_INCLUDED__
#define __VSFUSBH_UVC_H_INCLUDED__

PACKED_HEAD struct PACKED_MID video_probe_commit_ctrl_t
{
	uint16_t bmHint;
	uint8_t bFormatIndex;
#define VSFUSBH_UVC_VIDEO_FORMAT_MJPEG	0x1
#define VSFUSBH_UVC_VIDEO_FORMAT_YUY2	0x2
#define VSFUSBH_UVC_VIDEO_FORMAT_RGB24	0x3
	uint8_t bFrameIndex;
	uint32_t dwFrameInterval;
	uint16_t wKeyFrameRate;
	uint16_t wPFrameRate;
	uint16_t wCompQuality;
	uint16_t wCompWindowSize;
	uint16_t wDelay;
	uint32_t dwMaxVideoFrameSize;
	uint32_t dwMaxPayloadTransferSize;
}; PACKED_TAIL

struct vsfusbh_uvc_param_t
{
	uint16_t vid;
	uint16_t pid;
	
	bool video_enable;
	bool audio_enable;
	
	uint8_t video_iso_ep;
	uint8_t audio_iso_ep;
	uint16_t video_iso_packet_len;
	uint16_t audio_iso_packet_len;
	
	uint8_t video_interface;
	uint8_t video_interface_altr_setting;
	uint8_t audio_interface;
	uint8_t audio_interface_altr_setting;

	struct video_probe_commit_ctrl_t video_ctrl;
};

#ifndef VSFCFG_EXCLUDE_USBH_UVC
vsf_err_t vsfusbh_uvc_set(void *dev_data, struct vsfusbh_uvc_param_t *param);


#define VSFUSBH_UVC_REPORT_TYPE_VIDEO		0x01
#define VSFUSBH_UVC_REPORT_TYPE_AUDIO		0x02
extern void (*vsfusbh_uvc_report)(void *dev, struct vsfusbh_uvc_param_t *param,
		uint8_t type, uint8_t *data, uint32_t size);
extern const struct vsfusbh_class_drv_t vsfusbh_uvc_drv;
#endif

#endif // __VSFUSBH_UVC_H_INCLUDED__
