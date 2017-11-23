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

#ifndef __VSF_H_INCLUDED__
#define __VSF_H_INCLUDED__

#include "app_cfg.h"
#include "app_type.h"

#include "compiler.h"
#include "vsf_cfg.h"

#ifdef VSFCFG_MODULE
#include "appmod_cfg.h"
#endif
#include "vsfhal.h"

// framework
#include "framework/vsfsm/vsfsm.h"
#include "framework/vsftimer/vsftimer.h"

#ifdef VSFCFG_BUFFER
#include "component/fundation/buffer/buffer.h"
#endif
#ifdef VSFCFG_STREAM
#include "component/fundation/stream/stream.h"
#endif
#ifdef VSFCFG_MAL
#include "component/mal/vsfmal.h"
#ifdef VSFCFG_SCSI
#include "component/mal/vsfscsi.h"
#endif
#include "component/mal/drivers/embflash/embflash.h"
#endif
#ifdef VSFCFG_FILE
#include "component/file/vsfile.h"
#include "component/file/fs/malfs/vsf_malfs.h"
#include "component/file/fs/malfs/fat/vsffat.h"
#endif
#if defined(VSFCFG_MAL) && defined(VSFCFG_FILE)
#include "component/file/fs/fakefs/fakefat32/fakefat32.h"
#endif
#ifdef VSFCFG_DEBUG
#include "component/debug/debug.h"
#endif


#define VSF_API_VERSION						0x00000001

#ifdef VSFCFG_FUNC_SHELL
#include "component/shell/vsfshell.h"
#endif

#ifdef VSFCFG_FUNC_TCPIP
#include "component/tcpip/vsfip.h"
#include "component/tcpip/netif/eth/vsfip_eth.h"
#include "component/tcpip/proto/dhcp/vsfip_dhcpc.h"
#include "component/tcpip/proto/dhcp/vsfip_dhcpd.h"
#include "component/tcpip/proto/dns/vsfip_dnsc.h"
#include "component/tcpip/proto/http/vsfip_httpc.h"
#include "component/tcpip/proto/http/vsfip_httpd.h"
#include "component/tcpip/proto/telnet/vsfip_telnetd.h"
#endif

#ifdef VSFCFG_FUNC_USBD
#include "component/usb/core/vsfusbd.h"
#include "component/usb/class/device/CDC/vsfusbd_CDC.h"
#include "component/usb/class/device/CDC/vsfusbd_CDCACM.h"
#include "component/usb/class/device/VLLINK/vsfusbd_VLLINK.h"
#ifdef VSFCFG_FUNC_TCPIP
#include "component/usb/class/device/CDC/vsfusbd_RNDIS.h"
#endif
#include "component/usb/class/device/HID/vsfusbd_HID.h"
#ifdef VSFCFG_SCSI
#include "component/usb/class/device/MSC/vsfusbd_MSC_BOT.h"
#endif
#ifdef VSFCFG_FUNC_SDCD
#include "component/usb/core/dcd/sdcd/vsfsdcd.h"
#endif
#endif

#ifdef VSFCFG_FUNC_USBH
#include "component/usb/core/vsfusbh.h"
#include "component/usb/core/hcd/ohci/vsfohci.h"
#include "component/usb/core/dwc_otg/vsfdwcotg.h"
#include "component/usb/class/host/HUB/vsfusbh_HUB.h"
#include "component/usb/class/host/HID/vsfusbh_HID.h"
#ifdef VSFCFG_SCSI
#include "component/usb/class/host/MSC/vsfusbh_MSC.h"
#endif
#endif

#ifdef VSFCFG_FUNC_MFI
#include "component/mfi/vsfmfi.h"
#include "component/mfi/func/HID/vsfmfi_HID.h"
#include "component/mfi/func/EAP/vsfmfi_EAP.h"
#endif

#ifdef VSFCFG_FUNC_BCMWIFI
#include "component/tcpip/netif/eth/broadcom/bcm_wifi.h"
#include "component/tcpip/netif/eth/broadcom/bus/bcm_bus.h"
#endif

#ifdef VSFCFG_MODULE
#include "appmod.h"
#endif

#endif		// __VSF_H_INCLUDED__
