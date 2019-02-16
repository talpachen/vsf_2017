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

#ifndef __VSFUSB_MSC_H_INCLUDED__
#define __VSFUSB_MSC_H_INCLUDED__

#define USBMSC_CBW_SIGNATURE			0x43425355
#define USBMSC_CSW_SIGNATURE			0x53425355

#define USBMSC_CBWFLAGS_DIR_OUT			0x00
#define USBMSC_CBWFLAGS_DIR_IN			0x80
#define USBMSC_CBWFLAGS_DIR_MASK		0x80

#define USBMSC_CBW_SIZE					31
struct USBMSC_CBW_t
{
	uint32_t dCBWSignature;
	uint32_t dCBWTag;
	uint32_t dCBWDataTransferLength;
	uint8_t bmCBWFlags;
	uint8_t bCBWLUN;
	uint8_t bCBWCBLength;
	uint8_t CBWCB[16];
}; PACKED

#define USBMSC_CSW_SIZE					13
struct USBMSC_CSW_t
{
	uint32_t dCSWSignature;
	uint32_t dCSWTag;
	uint32_t dCSWDataResidue;
	uint8_t dCSWStatus;
}; PACKED

enum usb_MSCBOT_req_t
{
	USB_MSCBOTREQ_GET_MAX_LUN	= 0xFE,
	USB_MSCBOTREQ_RESET			= 0xFF,
};

#define USBMSC_CSW_OK					0x00
#define USBMSC_CSW_FAIL					0x01
#define USBMSC_CSW_PHASE_ERROR			0x02

#endif	// __VSFUSB_MSC_H_INCLUDED__
