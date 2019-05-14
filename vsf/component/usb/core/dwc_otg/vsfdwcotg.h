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
#ifndef __VSFDWCOTG_H___
#define __VSFDWCOTG_H___

#include "vsf_type.h"
#include "vsf_cfg.h"

#include "dwcotg_regs.h"

#define VSFDWCOTG_HC_EP_AMOUNT_LIMIT		VSFUSBD_CFG_EPMAXNO

struct vsfdwc2_hcd_param_t
{
	// Global Parameter
	uint32_t index;

	uint8_t speed : 4;			// Search "enum usb_device_speed_t"
	uint8_t dma_en : 1;
	uint8_t ulpi_en : 1;
	uint8_t utmi_en : 1;
	uint8_t vbus_en : 1;

	uint8_t hc_ep_amount;		// Device: dual-dir ep; Host: ep num
	uint16_t fifo_depth;						// in terms of 32-bit

#ifdef VSFDWC2_OTG_ENABLE

#endif

#ifdef VSFDWC2_DEVICE_ENABLE
	uint16_t device_rx_fifo_depth;				// in terms of 32-bit
	uint16_t device_tx_fifo_depth[VSFDWCOTG_HC_EP_AMOUNT_LIMIT];	// in terms of 32-bit
#endif
#ifdef VSFDWC2_HOST_ENABLE
	uint16_t host_rx_fifo_depth;				// in terms of 32-bit
	uint16_t host_non_periodic_tx_fifo_depth;	// in terms of 32-bit
	uint16_t host_periodic_tx_fifo_depth;		// in terms of 32-bit
#endif
};

struct vsfdwcotg_device_rx_ep_pkt_t
{
	uint32_t *buf;
	uint16_t packet_size;
	uint16_t max_packet_size;
};

struct vsfdwc2_device_t
{
	struct vsfdwc2_hcd_param_t *hc_param;
	void (*cb)(enum vsfhal_usbd_evt_t evt, uint32_t value);
	struct vsfdwcotg_device_rx_ep_pkt_t *rx_ep_pkt;
	uint32_t *device_rx_buffer;
	uint32_t device_rx_buffer_size;
	uint32_t device_rx_buffer_pos;
	
	// Core Global registers starting at offset 000h
	struct dwcotg_core_global_regs_t *global_reg;
	// Host Global Registers starting at offset 400h.
	struct dwcotg_host_global_regs_t *host_global_regs;
	// Host Port 0 Control and Status Register at offset 440h.
	volatile uint32_t *hprt0;
	// Host Channel Specific Registers at offsets 500h-5FCh.
	struct dwcotg_hc_regs_t *hc_regs;
	// Device Global Registers starting at offset 800h
	struct dwcotg_dev_global_regs_t *dev_global_regs;
	// Device Logical IN Endpoint-Specific Registers 900h-AFCh
	struct dwcotg_dev_in_ep_regs_t *in_ep_regs;
	// Device Logical OUT Endpoint-Specific Registers B00h-CFCh
	struct dwcotg_dev_out_ep_regs_t *out_ep_regs;
	
	uint32_t *dfifo[VSFDWCOTG_HC_EP_AMOUNT_LIMIT];
};

#ifdef VSFDWC2_DEVICE_ENABLE
vsf_err_t vsfdwc2_usbd_init(struct vsfdwc2_device_t *otgd, int32_t int_priority);
vsf_err_t vsfdwc2_usbd_fini(struct vsfdwc2_device_t *otgd);
vsf_err_t vsfdwc2_usbd_poll(struct vsfdwc2_device_t *otgd);
vsf_err_t vsfdwc2_usbd_reset(struct vsfdwc2_device_t *otgd);
vsf_err_t vsfdwc2_usbd_connect(struct vsfdwc2_device_t *otgd);
vsf_err_t vsfdwc2_usbd_disconnect(struct vsfdwc2_device_t *otgd);
vsf_err_t vsfdwc2_usbd_set_address(struct vsfdwc2_device_t *otgd, uint8_t address);
uint8_t vsfdwc2_usbd_get_address(struct vsfdwc2_device_t *otgd);
vsf_err_t vsfdwc2_usbd_wakeup(struct vsfdwc2_device_t *otgd);
uint32_t vsfdwc2_usbd_get_frame_number(struct vsfdwc2_device_t *otgd);
vsf_err_t vsfdwc2_usbd_get_setup(struct vsfdwc2_device_t *otgd, uint8_t *buffer);
vsf_err_t vsfdwc2_usbd_prepare_buffer(struct vsfdwc2_device_t *otgd);
vsf_err_t vsfdwc2_usbd_ep_reset(struct vsfdwc2_device_t *otgd, uint8_t idx);
vsf_err_t vsfdwc2_usbd_ep_set_type(struct vsfdwc2_device_t *otgd, uint8_t idx, enum vsfhal_usbd_eptype_t type);
vsf_err_t vsfdwc2_usbd_ep_set_IN_epsize(struct vsfdwc2_device_t *otgd, uint8_t idx, uint16_t epsize);
uint16_t vsfdwc2_usbd_ep_get_IN_epsize(struct vsfdwc2_device_t *otgd, uint8_t idx);
vsf_err_t vsfdwc2_usbd_ep_set_IN_stall(struct vsfdwc2_device_t *otgd, uint8_t idx);
vsf_err_t vsfdwc2_usbd_ep_clear_IN_stall(struct vsfdwc2_device_t *otgd, uint8_t idx);
bool vsfdwc2_usbd_ep_is_IN_stall(struct vsfdwc2_device_t *otgd, uint8_t idx);
vsf_err_t vsfdwc2_usbd_ep_reset_IN_toggle(struct vsfdwc2_device_t *otgd, uint8_t idx);
vsf_err_t vsfdwc2_usbd_ep_set_IN_count(struct vsfdwc2_device_t *otgd, uint8_t idx, uint16_t size);
vsf_err_t vsfdwc2_usbd_ep_write_IN_buffer(struct vsfdwc2_device_t *otgd, uint8_t idx, uint8_t *buffer, uint16_t size);
vsf_err_t vsfdwc2_usbd_ep_set_OUT_epsize(struct vsfdwc2_device_t *otgd, uint8_t idx, uint16_t epsize);
uint16_t vsfdwc2_usbd_ep_get_OUT_epsize(struct vsfdwc2_device_t *otgd, uint8_t idx);
vsf_err_t vsfdwc2_usbd_ep_set_OUT_stall(struct vsfdwc2_device_t *otgd, uint8_t idx);
vsf_err_t vsfdwc2_usbd_ep_clear_OUT_stall(struct vsfdwc2_device_t *otgd, uint8_t idx);
bool vsfdwc2_usbd_ep_is_OUT_stall(struct vsfdwc2_device_t *otgd, uint8_t idx);
vsf_err_t vsfdwc2_usbd_ep_reset_OUT_toggle(struct vsfdwc2_device_t *otgd, uint8_t idx);
uint16_t vsfdwc2_usbd_ep_get_OUT_count(struct vsfdwc2_device_t *otgd, uint8_t idx);
vsf_err_t vsfdwc2_usbd_ep_read_OUT_buffer(struct vsfdwc2_device_t *otgd, uint8_t idx, uint8_t *buffer, uint16_t size);
vsf_err_t vsfdwc2_usbd_ep_enable_OUT(struct vsfdwc2_device_t *otgd, uint8_t idx);
#endif

#ifdef VSFDWC2_HOST_ENABLE
extern const struct vsfusbh_hcddrv_t vsfdwcotgh_drv;
#endif

#endif // __VSFDWCOTG_H___
